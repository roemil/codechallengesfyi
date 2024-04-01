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
    ~LoadBalancerThread()
    {
        // lbthread_.join();
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
    ~EchoServerThread()
    {
        // echoServer_.join();
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
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    constexpr auto msg = "hello from client";
    client.socket_.send(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto res = client.socket_.recv();
    EXPECT_EQ(std::string { res.data() }, msg);
}


// Issue when lb is started first
TEST_F(LoadBalancerTest, NoBackendsAvailable)
{
    LoadBalancerThread lb {};
    waitForServer(8080);

    TestClient client { 8080 };
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    constexpr auto msg = "hello from client";
    client.socket_.send(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto res = client.socket_.recvWithError();
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), 0);
}
