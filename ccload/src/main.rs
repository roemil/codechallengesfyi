use clap::Parser;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant};
use rand::Rng;

#[derive(Parser, Debug)]
#[clap(
    author = "URL under test",
    version,
    about = "A simple HTTP Load tester"
)]
struct Args {
    #[clap(short, long)]
    url: String,

    #[clap(short)]
    n: Option<u16>,

    #[clap(short)]
    c: Option<u16>,
}

#[derive(Debug)]
struct LoadResult {
    successful: usize,
    failed: usize,
    fatal: usize,
    total_resp_time: Duration,
    mean_resp_time: Duration,
    min_resp_time: Duration,
    max_resp_time: Duration,
}

impl LoadResult {
    fn new() -> LoadResult {
        LoadResult {
            successful: 0,
            failed: 0,
            fatal: 0,
            total_resp_time: Duration::from_millis(0),
            mean_resp_time: Duration::from_millis(0),
            min_resp_time: Duration::from_millis(0),
            max_resp_time: Duration::from_millis(0),
        }
    }
}

fn send_requests(id: u16, num_request: u16, url: String, result: Arc<Mutex<LoadResult>>) {
    let mut resp_time: Vec<Duration> = Vec::new();
    for i in 0..num_request {
        let resp: Result<reqwest::blocking::Response, reqwest::Error>;
        let new_url: String;
        let mut rng = rand::thread_rng();
        let shoud_be_successful = rng.gen_range(0..num_request)  as f32 / num_request as f32;
        if shoud_be_successful > 0.1 {
            new_url = url.clone();
        } else {
            new_url = url.clone() + "/some_non_existent_path";
        }
        let start = Instant::now();
        resp = reqwest::blocking::get(new_url);
        let duration = start.elapsed();
        resp_time.push(duration);
        match resp.as_ref() {
            Ok(resp) => {
                let mut mresult = result.lock().expect("Expected to lock");
                if resp.status().is_success() {
                    mresult.successful += 1;
                    println!("{id}: Successful response code: {}", resp.status().as_str());
                } else {
                    mresult.failed += 1;
                    println!(
                        "{id}: Unsuccessful response code: {}",
                        resp.status().as_str()
                    );
                }
            }
            Err(err) => {
                let mut mresult = result.lock().expect("Expected to lock");
                mresult.fatal += 1;
                eprintln!("Response error: {:?}", err);
            }
        }
    }
    let mut mresult = result.lock().unwrap();
    mresult.total_resp_time += resp_time
        .iter()
        .fold(Duration::from_millis(0), |acc, x| acc + *x);
    let max_resp_time = resp_time.iter().max().unwrap();
    if mresult.max_resp_time < *max_resp_time {
        mresult.max_resp_time = *max_resp_time;
    }
    let min_resp_time = resp_time.iter().min().unwrap();
    if mresult.min_resp_time < *min_resp_time {
        mresult.min_resp_time = *min_resp_time;
    }
}

fn main() {
    let args = Args::parse();
    println!("{:?}", args);
    let url = args.url;

    let num_request = match args.n {
        Some(n) => n,
        None => 1,
    };
    let num_threads = match args.c {
        Some(num_threads) => num_threads,
        None => 1,
    };
    assert!(num_request >= num_threads);
    let num_req_per_thread = num_request / num_threads;
    let mut children = vec![];
    let results = Arc::new(Mutex::new(LoadResult::new()));
    for id in 0..num_threads {
        let url_copied = url.clone();
        let result_copy = results.clone();
        children.push(thread::spawn(move || {
            send_requests(id, num_req_per_thread, url_copied, result_copy)
        }));
    }

    for child in children {
        // Wait for the thread to finish. Returns a result.
        let _ = child.join();
    }
    let mut mresult = results.lock().unwrap();
    mresult.mean_resp_time = mresult.total_resp_time.div_f32(num_request as f32);
    println!("Result: {:?}", mresult);
}
