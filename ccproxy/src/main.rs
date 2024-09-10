use std::convert::Infallible;
use std::fs;
use std::net::SocketAddr;
use std::sync::Arc;

use hyper::server::conn::http1;
use hyper::{Method, Request, Response, StatusCode};
use hyper_util::rt::TokioIo;
use log::{error, info};
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

fn create_response<T>(body: T, http_code: StatusCode) -> Response<T> {
    let response = Response::new(body);
    let (mut parts, body) = response.into_parts();

    parts.status = http_code;
    Response::from_parts(parts, body)
}

async fn forward_proxy(
    local_socket: SocketAddr,
    req: Request<hyper::body::Incoming>,
    rules: Arc<Vec<String>>,
) -> Result<Response<String>, Infallible> {
    println!("Req {:?}", req);
    println!("Host {:?}", req.uri().host());
    println!("Port {:?}", req.uri().port());
    println!("Authority {:?}", req.uri().authority());
    println!("Remote {}", local_socket);
    let headers = req.headers();
    if !headers.contains_key("proxy-connection") {
        let resp = create_response("".to_string(), StatusCode::BAD_REQUEST);
        error!(
            "Error: Missing proxy-connection. Client: {}, Request URL: {}, StatusCode: {}",
            local_socket.to_string(),
            &req.uri(),
            resp.status()
        );
        return Ok(resp);
    }
    if *req.method() != Method::GET && *req.method() != Method::CONNECT {
        let resp = create_response("".to_string(), StatusCode::BAD_REQUEST);
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
        return response;
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
        return Ok(Response::<String>::default());
    }

    let c = reqwest::Client::new();
    let r = c
        .get(&req.uri().to_string())
        .header("User-Agent", headers.get("User-Agent").unwrap())
        .header("X-Forwarded-For", local_socket.to_string());
    // TODO: Make sure it is keep-alive'd
    let forward_req = r.build().expect("Failed to build request");

    let b = match c.execute(forward_req).await {
        Ok(r) => r.text().await,
        Err(e) => {
            eprintln!("Failed to forward request. Error: {}", e);
            let resp = create_response("".to_string(), StatusCode::BAD_GATEWAY);
            error!(
                "Failed to forward request. Error: {}, Client: {}, StatusCode: {}",
                e,
                local_socket.to_string(),
                resp.status()
            );
            return Ok(resp);
        }
    };

    // TODO: Add proper headers
    let response = Response::new(b.unwrap());
    info!(
        "Client: {}, Request URL: {}, StatusCode: {}",
        local_socket.to_string(),
        req.uri(),
        response.status()
    );
    Ok(response)
}

fn is_on_blacklist(rules: Arc<Vec<String>>, req: &Request<hyper::body::Incoming>, local_socket: SocketAddr) -> Option<Result<Response<String>, Infallible>> {
    if rules.contains(&req.uri().host().expect("Host must exist").to_string()) {
        let resp = create_response("".to_string(), StatusCode::FORBIDDEN);
        error!(
            "Error: Forbidden URI. Client: {}, Request URL: {}, StatusCode: {}",
            local_socket.to_string(),
            &req.uri(),
            resp.status()
        );
        return Some(Ok(resp));
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
