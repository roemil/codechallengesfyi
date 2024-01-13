# HTTP Load Tester
Low weight, single threaded load tester.

## Usage
cargo run -- -u URL:PORT -n NUM_REQUESTS -c NUM_CONCURRENT_REQUESTS<br>

* NUM_REQUEST is the total amount of requests to send.
* NUM_CONCURRENT_REQUESTS is the number of requests to send concurrently.

### Example:
```
cargo run -- -u http://127.0.0.1:7878 -n 100 -c 5
```
This command will execute 100 requests, divided into 5 asyncronous groups. Each group will therefore do 20 requests<br>
which will be summed up. The printed result is the aggregated result for all groups.