## Load Balancer

This project aims to teach me how a load balancer works by simply forwarding the request to a backend server.<br>
The backend server in this example is a simple echo server.<br>

## TODO
1. Make it unit testable by mocking all sockets 2. Refactor the Loadbalancer
    / Echoserver server logic to a common TcpServer library 3. Improve integration tests(no sleeps should be needed to make it stable) 4. More integration tests to find edge cases.
