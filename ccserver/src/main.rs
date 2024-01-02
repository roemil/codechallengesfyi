use std::fmt;
use std::{
    io::{prelude::*, BufReader},
    net::{TcpListener, TcpStream},
};
use std::fs;
use std::{thread, time};

enum StatusCode {
    Ok = 200,
    NotFound = 404,
    BadRequest = 400,
}

struct Response {
    status_code : StatusCode,
    data : Option<String>
}

impl fmt::Display for StatusCode {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            StatusCode::Ok => write!(f, "{} {}", (StatusCode::Ok as i32).to_string(), "OK"),
            StatusCode::NotFound => write!(f, "404 Not Found"),
            StatusCode::BadRequest => write!(f, "400 Bad Request"),
        }
    }
}

impl fmt::Display for Response {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        writeln!(f, "{}", self.status_code)?;
        write!(f, "\n")?;
        match &self.data {
            Some(data) => write!(f, "{data}"),
            None => write!(f, ""),
        }
    }
}

fn handle_get_request(request: &str) -> Option<Response> {
    println!("Got request for path: {request}");
    let request = request.trim();
    // TODO: Dont allow relative paths
    let path = String::from("src") + match request {
        "/" => "/index.html",
        path => path,
    };
    println!("{path}");
    let html_data = fs::read_to_string(path);
    match html_data {
        Ok(data) => {
            return Some(Response {status_code: StatusCode::Ok, data: Some(data) });
        }
        _ => {
            eprintln!("Failed to open file");
            return None
        },
    }
}

fn handle_client(mut stream: &TcpStream, index : usize) {
    let buf_reader = BufReader::new(stream);
    let http_request: Vec<_> = buf_reader
        .lines()
        .map(|result| result.unwrap())
        .take_while(|line| !line.is_empty())
        .collect();
    println!("Request: {:#?}", http_request[0]);

    let request = &http_request[0]
        .strip_suffix("HTTP/1.1")
        .expect("Version not present");

    let response = match request.find("GET") {
        Some(_) => request
            .strip_prefix("GET")
            .and_then(handle_get_request)
            .unwrap_or_else(|| Response{status_code: StatusCode::NotFound, data: None}),
        None => {
            eprintln!("Unsupported request");
            Response{status_code: StatusCode::BadRequest, data: None}
        }
    };

    println!("Thread ID: {index}");
    // Uncomment to slow down server
    // let ten_millis = time::Duration::from_secs(20);
    // thread::sleep(ten_millis);

    writeln!(stream, "HTTP/1.1 {}", response).unwrap();
}

fn main() {
    let listener = TcpListener::bind("127.0.0.1:7878").unwrap();

    // accept connections and spawn a new thread to handle it.
    for (index, stream) in listener.incoming().enumerate() {
        thread::spawn(move || handle_client(&stream.unwrap(), index));
    }
}
