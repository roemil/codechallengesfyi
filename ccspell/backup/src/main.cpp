#include "BloomFilter.h"

#include <cmath>
#include <iostream>
#include <string_view>
#include <string>
#include <fstream>
#include <cassert>
#include <arpa/inet.h>

constexpr int n = 1000000;
constexpr double epsilon = 0.02;

constexpr int m = (- n * std::log(epsilon) / std::pow(std::log(2),2));

int main(int argc, char** argv) {


    bool buildDict = false;
    for(int i = 0; i < argc; ++i) {
        std::string_view str{argv[i]};
        buildDict = str.find("-build") != std::string_view::npos;
    }

    if(buildDict){
        bf::BloomFilter<m> bf{};
        std::cout << "Building dict...\n";
        std::ifstream infile("../src/dict.txt");
        assert(infile.is_open() && "file not open");
        std::string line;
        while (std::getline(infile, line))
        {
            bf.buildBitArray(line);
        }
        std::ofstream fs("words.bf", std::ios::out | std::ios::binary | std::ios::app);
        bf.serialize(fs);
        std::cout << "Done.\n";
        return 0;
    }

    std::ifstream binfile("words.bf", std::ios::binary);
    assert(binfile.is_open() && "file not open");

    bf::BloomFilterLookUp bf{};
    bf.unserialize(binfile);
    for(int i = 1; i < argc; ++i) {
        std::string_view str{argv[i]};
        if (bf.lookup(str)){
            std::cout << str << " is correct!\n";
        } else {
            std::cout << str << " is NOT correct!\n";
        }
    }
        binfile.close();

}
