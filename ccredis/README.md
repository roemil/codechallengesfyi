## Simple Redis server
Work in progress<br>

This server support concurrent connection from redis-cli.
Supports the following commands:
* PING
* SET
* GET 

### Benchmark
* SET: 78125.00 requests per second, p50=0.359 msec                   
* GET: 79302.14 requests per second, p50=0.367 msec

### TOD
1. Use std::expected as error handling.
2. More commands.