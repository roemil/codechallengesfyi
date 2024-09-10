use std::convert::Infallible;
use std::fs;
use std::net::SocketAddr;
use std::sync::Arc;
use bytes::Bytes;

use http_body_util::combinators::BoxBody;
use http_body_util::BodyExt;
use hyper::client::conn::http1::Builder;
use hyper::server::conn::http1;
use hyper::{Error, Method, Request, Response, StatusCode};
use hyper_util::rt::TokioIo;
use log::error;
use tokio::net::TcpListener;

#[derive(Debug, Clone)]
pub struct Proxy<S> {
    inner: S,
}
impl<S> Proxy<S> {
    pub fn new(inner: S) -> Self {
        Proxy { inner }
    }
}
type Req = Request<hyper::body::Incoming>;

impl<S> hyper::service::Service<Req> for Proxy<S>
where
    S: hyper::service::Service<Req>,
{
    type Response = S::Response;
    type Error = S::Error;
    type Future = S::Future;
    fn call(&self, req: Req) -> Self::Future {
        println!(
            "processing request from: {} {}",
            req.method(),
            req.uri().path()
        );
        self.inner.call(req)
    }
}

fn create_response(http_code: StatusCode) -> Response<BoxBody<Bytes, Error>> {
    let body = Bytes::from(http_code.as_str().to_string());
    let mut response = Response::new(BoxBody::<Bytes, Error>::new(http_body_util::Full::new(body).map_err(|e| match e {})));
    *response.status_mut() = http_code;
    response
}

async fn forward_proxy(
    local_socket: SocketAddr,
    req: Request<hyper::body::Incoming>,
    rules: Arc<Vec<String>>,
) -> Result<Response<BoxBody<Bytes, Error>>, Infallible> {
    println!("Req {:?}", req);
    println!("Host {:?}", req.uri().host());
    println!("Port {:?}", req.uri().port());
    println!("Authority {:?}", req.uri().authority());
    println!("Remote {}", local_socket);
    let headers = req.headers();
    if !headers.contains_key("proxy-connection") {
        let resp = create_response(StatusCode::BAD_REQUEST);
        error!(
            "Error: Missing proxy-connection. Client: {}, Request URL: {}, StatusCode: {}",
            local_socket.to_string(),
            &req.uri(),
            resp.status()
        );
        return Ok(resp);
    }
    if *req.method() != Method::GET && *req.method() != Method::CONNECT {
        let resp = create_response(StatusCode::BAD_REQUEST);
        error!(
            "Error: Not a Get or Connect request. Client: {}, Request URL: {}, Method: {} , StatusCode: {}",
            local_socket.to_string(),
            &req.uri(),
            req.method(),
            resp.status()
        );
        return Ok(resp);
    }

    if let Some(response) = is_on_blacklist(rules, &req, local_socket) {
        return Ok(response);
    }

    if *req.method() == Method::CONNECT {
        let addr = req.uri().authority().unwrap().to_string();
        tokio::task::spawn(async move {
            match hyper::upgrade::on(req).await {
                Ok(upgraded) => {
                    if let Err(e) = tunnel(upgraded, addr).await {
                        eprintln!("server io error: {}", e);
                    };
                }
                Err(e) => eprintln!("upgrade error: {}", e),
            }
        });
        // Must return a empty body to client then the connection will be upgraded.
        return Ok(Response::default());
    }

    Ok(forward_request(req).await?)
}

async fn forward_request(req: Request<hyper::body::Incoming>) -> Result<Response<BoxBody<Bytes, Error>>, Infallible> {
    let addr = req.uri().host().unwrap();
    let port = req.uri().port_u16().unwrap_or(80);
    let stream = tokio::net::TcpStream::connect((addr,port))
        .await
        .expect("Remote not available");
    let io = TokioIo::new(stream);
    let (mut sender, conn) = Builder::new()
        .preserve_header_case(true)
        .title_case_headers(true)
        .handshake(io)
        .await
        .unwrap();
    tokio::task::spawn(async move {
        if let Err(err) = conn.await {
            println!("Connection failed: {:?}", err);
        }
    });
    let resp = sender.send_request(req).await.unwrap();
    Ok(resp.map(|body| body.boxed()))
}

fn is_on_blacklist(
    rules: Arc<Vec<String>>,
    req: &Request<hyper::body::Incoming>,
    local_socket: SocketAddr,
) -> Option<Response<BoxBody<Bytes, Error>>> {
    if rules.contains(&req.uri().host().expect("Host must exist").to_string()) {
        let resp = create_response(StatusCode::FORBIDDEN);
        error!(
            "Error: Forbidden URI. Client: {}, Request URL: {}, StatusCode: {}",
            local_socket.to_string(),
            &req.uri(),
            resp.status()
        );
        return Some(resp);
    }
    None
}

fn parse_blacklist(path: &str) -> Vec<String> {
    fs::read_to_string(path)
        .expect("File must exist!")
        .lines()
        .into_iter()
        .map(|line| line.to_string())
        .collect()
}

async fn tunnel(upgraded: hyper::upgrade::Upgraded, addr: String) -> std::io::Result<()> {
    // Connect to remote server
    let mut server = tokio::net::TcpStream::connect(addr).await?;
    let mut upgraded = TokioIo::new(upgraded);

    // Proxying data
    let _ = tokio::io::copy_bidirectional(&mut upgraded, &mut server).await?;

    Ok(())
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let addr = SocketAddr::from(([127, 0, 0, 1], 8089));
    log4rs::init_file("log_config.yaml", Default::default()).unwrap();

    let listener = TcpListener::bind(addr).await?;

    let blacklist = Arc::new(parse_blacklist("src/blacklist.txt"));

    loop {
        let (stream, _) = listener.accept().await?;
        let io = TokioIo::new(stream);
        let blacklist_c = blacklist.clone();
        tokio::task::spawn(async move {
            let remote = io.inner().peer_addr().expect("Remote address must exist");
            let svc = hyper::service::service_fn(move |req| {
                forward_proxy(remote, req, blacklist_c.clone())
            });
            if let Err(err) = http1::Builder::new()
                .preserve_header_case(true)
                .title_case_headers(true)
                .serve_connection(io, svc)
                .with_upgrades()
                .await
            {
                eprintln!("Error serving connection: {:?}", err);
            }
        });
    }
}
