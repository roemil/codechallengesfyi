use std::convert::Infallible;
use std::fs;
use std::net::SocketAddr;
use std::sync::Arc;

use http_body_util::Full;
use hyper::body::Bytes;
use hyper::server::conn::http1;
use hyper::{Method, Request, Response, StatusCode};
use hyper_util::rt::TokioIo;
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

async fn forward_proxy(
    remote: SocketAddr,
    req: Request<hyper::body::Incoming>,
    rules: Arc<Vec<String>>,
) -> Result<Response<Full<Bytes>>, Infallible> {
    let headers = req.headers();
    if !headers.contains_key("proxy-connection") {
        return Ok(Response::new(Full::new(Bytes::from(
            StatusCode::BAD_REQUEST.as_str(),
        ))));
    }
    if *req.method() != Method::GET {
        return Ok(Response::new(Full::new(Bytes::from(
            StatusCode::BAD_REQUEST.as_str(),
        ))));
    }

    if rules.contains(&req.uri().to_string()) {
        return Ok(Response::new(Full::new(Bytes::from(
            StatusCode::FORBIDDEN.as_str(),
        ))));
    }

    let c = reqwest::Client::new();
    let r = c
        .get(req.uri().to_string())
        .header("User-Agent", headers.get("User-Agent").unwrap())
        .header("X-Forwarded-For", remote.to_string());
    // TODO: Make sure it is keep-alive'd
    let req = r.build().expect("Failed to build request");

    let b = match c.execute(req).await {
        Ok(r) => r.bytes().await,
        Err(e) => {
            eprintln!("Failed to forward request. Error: {}", e);
            return Ok(Response::new(Full::new(Bytes::from(
                StatusCode::BAD_GATEWAY.as_str(),
            ))));
        }
    };

    let response = Response::new(Full::new(b.unwrap()));
    println!("Response: {:?}", response);
    Ok(response)
}

fn parse_blacklist(path: &str) -> Vec<String> {
    fs::read_to_string(path)
        .expect("Should have been able to read the file")
        .lines()
        .into_iter()
        .map(|line| line.to_string())
        .collect()
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let addr = SocketAddr::from(([127, 0, 0, 1], 8089));

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
                .serve_connection(io, svc)
                .await
            {
                eprintln!("Error serving connection: {:?}", err);
            }
        });
    }
}
