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
    n: Option<usize>,

    #[clap(short)]
    c: Option<usize>,
}

#[derive(Default, Clone, Copy)]
struct LoadResult {
    total_reqs: usize,
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
            total_reqs: 0,
            successful: 0,
            failed: 0,
            fatal: 0,
            total_ttfb: Duration::from_millis(0),
            mean_ttfb: Duration::from_millis(0),
            min_ttfb: Duration::from_millis(usize::MAX as u64),
            max_ttfb: Duration::from_millis(usize::MIN as u64),
            total_ttlb: Duration::from_millis(0),
            mean_ttlb: Duration::from_millis(0),
            min_ttlb: Duration::from_millis(usize::MAX as u64),
            max_ttlb: Duration::from_millis(usize::MIN as u64),
        }
    }
}

impl std::fmt::Display for LoadResult {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        writeln!(f, "{0: <12} {1: <10}", "Total Reqs: ", self.total_reqs)?;
        writeln!(f, "{0: <12} {1: <10}", "Successful: ", self.successful)?;
        writeln!(f, "{0: <12} {1: <10}", "Failed: ", self.failed)?;
        writeln!(f, "{0: <12} {1: <10}", "Fatal: ", self.fatal)?;
        writeln!(f, "{0: <12} {1: <10?}", "Total TTFB: ", self.total_ttfb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Mean  TTFB: ", self.mean_ttfb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Min   TTFB: ", self.min_ttfb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Max   TTFB: ", self.max_ttfb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Total TTlB: ", self.total_ttlb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Mean  TTlB: ", self.mean_ttlb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Min   TTlB: ", self.min_ttlb)?;
        writeln!(f, "{0: <12} {1: <10?}", "Max   TTlB: ", self.max_ttlb)
    }
}

impl Add for LoadResult {
    type Output = Self;

    fn add(mut self, rhs: Self) -> Self {
        self.total_reqs += rhs.total_reqs;
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
async fn send_requests(id: usize, num_request: usize, url: String) -> LoadResult {
    let mut result = LoadResult::new();
    let mut ttfbs: Vec<Duration> = Vec::new();
    let mut ttlbs: Vec<Duration> = Vec::new();
    result.total_reqs = num_request;
    for _ in 0..num_request {
        let start = Instant::now();
        let resp = reqwest::get(&url).await;
        match resp.as_ref() {
            Ok(resp) => {
                if resp.status().is_success() {
                    result.successful += 1;
                } else {
                    result.failed += 1;
                    eprintln!(
                        "{id}: Unsuccessful response code: {}",
                        resp.status().as_str()
                    );
                    continue;
                }
            }
            Err(err) => {
                result.fatal += 1;
                eprintln!("Response error: {:?}", err);
                continue;
            }
        }
        ttfbs.push(start.elapsed());
        let _ = reqwest::get(&url).await;
        ttlbs.push(start.elapsed());
        
    }
    result.total_ttfb = ttfbs
        .iter()
        .fold(Duration::from_millis(0), |acc, x| acc + *x);
    result.max_ttfb = *ttfbs.iter().max().expect("No max value calculated");
    result.min_ttfb = *ttfbs.iter().min().expect("No min value calculated");
    result.mean_ttfb = result.total_ttfb.div_f32(num_request as f32);
    result.total_ttlb = ttlbs
        .iter()
        .fold(Duration::from_millis(0), |acc, x| acc + *x);
    result.max_ttlb = *ttlbs.iter().max().expect("No max value calculated");
    result.min_ttlb = *ttlbs.iter().min().expect("No min value calculated");
    result.mean_ttlb = result.total_ttlb.div_f32(num_request as f32);
    return result;
}

#[tokio::main]
async fn main() { 
    let args = Args::parse();
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

    let mut outputs = Vec::with_capacity(num_threads);
    while let Some(res) = set.join_next().await {
        outputs.push(res.unwrap());
    }

    let final_res = outputs.iter().fold(LoadResult::new(), |acc, x| acc + *x);
    println!("{}", final_res);

}
