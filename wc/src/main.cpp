#include "../include/CommandVisitor.h"
#include "../include/Counters.h"

#include <_ctype.h>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <variant>
#include <vector>
#include <memory>


void printResult(const int data, const std::string& filename)
{
    std::cout << "  " << data << " " << filename << std::endl;
}

template<typename ... Ts>
struct Overload : Ts ... { 
    using Ts::operator() ...;
};
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

int main(int argc, char* argv[])
{
    std::vector<std::string> commands{};
    std::string filename{};
    if(argc == 2)
    {
        commands.push_back("-c");
        commands.push_back("-l");
        commands.push_back("-w");
        commands.push_back("-m");
        filename = argv[1];
    }
    else if(argc >= 3)
    {
        for(int i = 1; i < argc-1; ++i)
        {
            commands.push_back(argv[i]);
        }
        filename = argv[argc-1];
    }
    else if(argc == 2 || argc == 1)
    {
        std::cout << "wrong number of input arguments...\n";
        std::cout << "expect command and filename\n";
        exit(1);
    }

    /*
    Two different versions of the visitor pattern are implemented.
    1. The traditional version
    2. The modern version (>c++ 17) using std::variant
    */

    CommandVisitor commandVisitor{filename};
    std::vector<std::shared_ptr<ICounter>> counters{};
    std::vector<std::variant<std::shared_ptr<FileSizeCounter>,
    std::shared_ptr<LineCounter>,
    std::shared_ptr<WordCounter>,
    std::shared_ptr<CharCounter>
    >> counterVariants{};

    for(const auto& command : commands)
    {
        if(command == "-c")
        {
            auto filesizeCounter = std::make_shared<FileSizeCounter>();
            counters.push_back(filesizeCounter);
            counterVariants.push_back(filesizeCounter);
        }
        if(command == "-l")
        {
            auto lineCounter = std::make_shared<LineCounter>();
            counters.push_back(lineCounter);
            counterVariants.push_back(lineCounter);
        }
        if(command == "-w")
        {
            auto wordCounter = std::make_shared<WordCounter>();
            counters.push_back(wordCounter);
            counterVariants.push_back(wordCounter);
        }
        if(command == "-m")
        {
            auto charCounter = std::make_shared<CharCounter>();
            counters.push_back(charCounter);
            counterVariants.push_back(charCounter);
        }
    }

    for(auto& counter : counters)
    {
        counter->accept(commandVisitor);
    }

    auto printVisitor = Overload 
    {
        [](std::shared_ptr<FileSizeCounter> counter){ counter->print(); },
        [](std::shared_ptr<LineCounter> counter){ counter->print(); },
        [](std::shared_ptr<WordCounter> counter){ counter->print(); },
        [](std::shared_ptr<CharCounter> counter){ counter->print(); },
    };

    for(auto& variant : counterVariants)
    {
        std::visit(printVisitor, variant);
    }

    std::cout << " " << filename << std::endl;

}