use clap::Parser;
use tokio::task::JoinSet;
use std::cmp;
use std::ops::Add;
use std::time::{Duration, Instant};

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

#[derive(Debug, Default, Clone, Copy)]
struct LoadResult {
    successful: usize,
    failed: usize,
    fatal: usize,
    total_ttfb: Duration,
    mean_ttfb: Duration,
    min_ttfb: Duration,
    max_ttfb: Duration,
    total_ttlb: Duration,
    mean_ttlb: Duration,
    min_ttlb: Duration,
    max_ttlb: Duration,
}

impl LoadResult {
    fn new() -> LoadResult {
        LoadResult {
            successful: 0,
            failed: 0,
            fatal: 0,
            total_ttfb: Duration::from_millis(0),
            mean_ttfb: Duration::from_millis(0),
            min_ttfb: Duration::from_millis(0),
            max_ttfb: Duration::from_millis(0),
            total_ttlb: Duration::from_millis(0),
            mean_ttlb: Duration::from_millis(0),
            min_ttlb: Duration::from_millis(0),
            max_ttlb: Duration::from_millis(0),
        }
    }
    // TODO: add pretty print
}

impl Add for LoadResult {
    type Output = Self;

    fn add(mut self, rhs: Self) -> Self {
        // TODO: Fix calculations.
        self.successful += rhs.successful;
        self.failed += rhs.failed;
        self.fatal += rhs.fatal;
        self.total_ttfb += rhs.total_ttfb;
        self.mean_ttfb = (self.mean_ttfb + rhs.mean_ttfb)/2;
        self.min_ttfb = cmp::min(self.min_ttfb, rhs.min_ttfb);
        self.max_ttfb = cmp::max(self.max_ttfb, rhs.max_ttfb);

        self.total_ttlb += rhs.total_ttlb;
        self.mean_ttlb = (self.mean_ttlb + rhs.mean_ttlb)/2;
        self.min_ttlb = cmp::min(self.min_ttlb, rhs.min_ttlb);
        self.max_ttlb = cmp::max(self.max_ttlb, rhs.max_ttlb);
        self
    }
}
async fn send_requests(id: u16, num_request: u16, url: String) -> LoadResult {
    let mut mresult = LoadResult::new();
    let mut ttfbs: Vec<Duration> = Vec::new();
    let mut ttlbs: Vec<Duration> = Vec::new();
    for _ in 0..num_request {
        let start = Instant::now();
        let resp = reqwest::get(&url).await;
        ttfbs.push(start.elapsed());
        match resp.as_ref() {
            Ok(resp) => {
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
                mresult.fatal += 1;
                eprintln!("Response error: {:?}", err);
            }
        }
        let _ = reqwest::get(&url).await;
        ttlbs.push(start.elapsed());
        
    }
    mresult.total_ttfb = ttfbs
        .iter()
        .fold(Duration::from_millis(0), |acc, x| acc + *x);
    mresult.max_ttfb = *ttfbs.iter().max().unwrap();
    mresult.min_ttfb = *ttfbs.iter().min().unwrap();
    mresult.total_ttfb = ttlbs
        .iter()
        .fold(Duration::from_millis(0), |acc, x| acc + *x);
    mresult.max_ttlb = *ttlbs.iter().max().unwrap();
    mresult.min_ttlb = *ttlbs.iter().min().unwrap();
    return mresult;
}

#[tokio::main]
async fn main() { 
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
    let num_req_per_thread = num_request.clone() / num_threads.clone();
    let mut set = JoinSet::new();
    for id in 0..num_threads {
        let url_clone = url.clone();
        set.spawn(async move {
            let res = send_requests(id, num_req_per_thread, url_clone).await;
            res
        });
    }

    let mut outputs = Vec::with_capacity(num_threads as usize);
    while let Some(res) = set.join_next().await {
        outputs.push(res.unwrap());
    }

    let final_res = outputs.iter().fold(LoadResult::new(), |acc, x| acc + *x);
    println!("{:?}", final_res);

}
