## Simple forward proxy using hyper
Works both with and without TLS

## Build and run
cargo run

## Example

In another window do: curl --proxy "http://localhost:8089" "http://httpbin.org/ip" or with TLS just replace the second http to https.

If you want to blacklist specific pages you add the hostname to blacklist.txt.
