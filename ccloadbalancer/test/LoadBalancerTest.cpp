#include "LoadBalancer.h"
#include "EchoServer/EchoServer.h"
#include "TcpSocket.h"
#include <stdexcept>
#include <thread>

#include <gtest/gtest.h>

struct TestClient {
    TestClient(int port)
        : socket_(TcpSocket { port })
    {
    }
    TcpSocket socket_;
};

class LoadBalancerTest : public testing::Test {
protected:
};

struct LoadBalancerThread {
    LoadBalancerThread()
        : lbthread_ { []() {
            LoadBalancer lb {};
            lb.addBackend(8081);
            lb.addBackend(8082);
            lb.start("8080");
        } }
    {
        lbthread_.detach();
    }
    std::thread lbthread_;
};

struct EchoServerThread {
    EchoServerThread(const std::string_view port)
        : echoServer_ { [port]() {
            EchoServer echoserver {};
            echoserver.start(port);
        } }
    {
        echoServer_.detach();
    }
    std::thread echoServer_;
};

namespace {
void waitForServer(int port)
{
    bool isReady = false;
    while (!isReady) {
        try {
            TcpSocket test { port };
            isReady = true;
            std::cout << port << " is ready\n";
            break;
        } catch (std::invalid_argument&) {
            std::cout << port << " is not ready yet\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}
}

TEST_F(LoadBalancerTest, Basic)
{
    EchoServerThread backend { "8081" };
    waitForServer(8081);

    LoadBalancerThread lb {};
    waitForServer(8080);

    TestClient client { 8080 };
    constexpr auto msg = "hello from client";
    client.socket_.send(msg);
    auto res = client.socket_.recv();
    EXPECT_EQ(std::string { res.first.data() }, msg);
    std::cout << "\nDone\n";
}

TEST_F(LoadBalancerTest, NoBackendsAvailable)
{
    LoadBalancerThread lb {};
    waitForServer(8080);

    TestClient client { 8080 };
    constexpr auto msg = "hello from client";
    client.socket_.send(msg);
    auto res = client.socket_.recvWithError();
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), 0);
}

TEST_F(LoadBalancerTest, ResultNotAvailableAtFirst)
{
    EchoServerThread backend { "8081" };
    waitForServer(8081);

    LoadBalancerThread lb {};
    waitForServer(8080);

    TestClient client { 8080 };
    constexpr auto msg = "hello from client";
    client.socket_.send(msg);
    auto res = client.socket_.recvWithError();
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), 0);
}
