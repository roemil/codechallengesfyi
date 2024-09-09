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

fn create_response<T>(body: T, http_code : StatusCode) -> Response<T> {
    let response = Response::new(body);
        let (mut parts, body) = response.into_parts();

        parts.status = http_code;
        Response::from_parts(parts, body)
}

async fn forward_proxy(
    remote: SocketAddr,
    req: Request<hyper::body::Incoming>,
    rules: Arc<Vec<String>>,
) -> Result<Response<String>, Infallible> {
    let headers = req.headers();
    if !headers.contains_key("proxy-connection") {
        let resp = create_response("".to_string(), StatusCode::BAD_REQUEST);
        error!(
            "Error: Missing proxy-connection. Client: {}, Request URL: {}, StatusCode: {}",
            remote.to_string(),
            &req.uri(),
            resp.status()
        );
        return Ok(resp);
    }
    if *req.method() != Method::GET {
        let resp = create_response("".to_string(), StatusCode::BAD_REQUEST);
        error!(
            "Error: Not a Get request. Client: {}, Request URL: {}, Method: {} , StatusCode: {}",
            remote.to_string(),
            &req.uri(),
            req.method(),
            resp.status()
        );
        return Ok(resp);
    }

    if rules.contains(&req.uri().to_string()) {
        let resp = create_response("".to_string(), StatusCode::FORBIDDEN);
        error!(
            "Error: Forbidden URI. Client: {}, Request URL: {}, StatusCode: {}",
            remote.to_string(),
            &req.uri(),
            resp.status()
        );
        return Ok(resp);
    }

    let c = reqwest::Client::new();
    let r = c
        .get(&req.uri().to_string())
        .header("User-Agent", headers.get("User-Agent").unwrap())
        .header("X-Forwarded-For", remote.to_string());
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
                remote.to_string(),
                resp.status()
            );
            return Ok(resp);
        }
    };

    // TODO: Add proper headers
    let response = Response::new(b.unwrap());
    info!(
        "Client: {}, Request URL: {}, StatusCode: {}",
        remote.to_string(),
        req.uri(),
        response.status()
    );
    Ok(response)
}

fn parse_blacklist(path: &str) -> Vec<String> {
    fs::read_to_string(path)
        .expect("File must exist!")
        .lines()
        .into_iter()
        .map(|line| line.to_string())
        .collect()
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
            if let Err(err) = http1::Builder::new().serve_connection(io, svc).await {
                eprintln!("Error serving connection: {:?}", err);
            }
        });
    }
}
