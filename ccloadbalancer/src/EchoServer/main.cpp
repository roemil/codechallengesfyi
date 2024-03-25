#include "EchoServer/EchoServer.h"
#include <cstdio>
#include <cstdlib>
#include <sys/poll.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        perror("Must provide port number");
        exit(1);
    }
    EchoServer server {};
    server.start(argv[1]);
}