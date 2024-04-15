
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string_view>

#include "IrcClient.h"
#include "Tui.h"
#include "thread"

int main()
{
    IrcClient client{"CCClient", "Coding Challenges Client"};
    Tui tui{&client};
    client.tui_ = &tui;
    
    std::thread t{[&client](){
        client.connect("irc.freenode.net");
    }};
    tui.start();
    t.join();
}