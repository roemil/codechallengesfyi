#include "LoadBalancer.h"

int main()
{
    LoadBalancer server {};
    server.addBackend(8081);
    server.addBackend(8082);
    server.start("8080");
}
