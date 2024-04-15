#pragma once

#include "HashFunction.h"
#include <algorithm>
#include <cstdint>
#include <netinet/in.h>
#include <string_view>
#include <string>
#include <iterator>
#include <vector>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <cassert>

namespace bf
{
struct BloomFilter
{
    BloomFilter() = default;
    BloomFilter(int m, int version, int numHashes) : numBits_(m), version_(version), numHashes_(numHashes) {
        bitarray_ = std::vector<bool>(numBits_, false);
    }
    std::vector<bool> bitarray_; // Vector<bool> is optimzed for bit storage.

    int numBits_{};
    uint16_t version_ {};
    uint16_t numHashes_{};

    void buildBitArray(const std::string_view str) {
        assert(numBits_ > 0);
        assert(bitarray_.max_size() > 0);
        auto hash = fnv1(str);
        bitarray_[hash % numBits_] = 1;
        for(int i = 1; i < numHashes_; ++i){
            hash = fnv1(hash);
            bitarray_[hash % numBits_] = 1;
        }
    }
    bool lookup(const std::string_view str) const {
        assert(numBits_ > 0);
        bool lookUpRes = true;
        auto hash = fnv1(str);
        lookUpRes &= bitarray_[hash % numBits_];
        
        for(int i = 1; i < numHashes_; ++i){
            hash = fnv1(hash);
            lookUpRes &= bitarray_[hash % numBits_];
        }
        return lookUpRes;
    }

    void serialize(std::ofstream& fs) {
        auto writeHeaderToFile = [](std::ofstream& fs, uint16_t version, uint16_t numHashes, int numBits){
            fs.write("CCBF", 4);
            fs.write(reinterpret_cast<const char*>(&version), 2);
            fs.write(reinterpret_cast<const char*>(&numHashes), 2);

            auto firstBytes = htons(numBits >> 16);
            auto secondBytes = htons(numBits & 0xffff);
            
            fs.write(reinterpret_cast<const char*>(&firstBytes), 2);
            fs.write(reinterpret_cast<const char*>(&secondBytes), 2);
        };

        writeHeaderToFile(fs, htons(version_), htons(numHashes_), numBits_);

        std::vector<uint8_t> bytes{};
        auto getByte = [ba = bitarray_](int i){
            uint8_t byte{};
            for(int j = 0; j < 8; ++j){
                byte |= ba[i + j];
                if(j < 7){
                    byte = byte << 1;
                }
            }
            return byte;
        };
        for(int i = 0; i < numBits_; i += 8){
            bytes.push_back(getByte(i));
        }

        fs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        fs.close();
    }

    void unserialize(std::ifstream& binfile) {
        std::vector<uint8_t> ccheader{};
        for(int i = 0; i < 12; ++i)
        {
            char byte;
            binfile.read(&byte, sizeof(char));
            ccheader.push_back(static_cast<uint8_t>(byte));
        }

        const std::string prefix{static_cast<char>(ccheader[0]),static_cast<char>(ccheader[1]),static_cast<char>(ccheader[2]),static_cast<char>(ccheader[3])};
        assert(prefix == "CCBF" && "Prefix is incorrect");

        auto getNext2Bytes = [](const std::vector<uint8_t>& data, int startingInd){
            auto value = data[startingInd] << 8;
            value |= data[startingInd+1];
            return value;
        };

        version_ = getNext2Bytes(ccheader, 4);
        numHashes_ = getNext2Bytes(ccheader, 6);

        auto buildNumberOfBitsUsed = [getNext2Bytes](const std::vector<uint8_t> data, int startingInd){
            const int firstByte = getNext2Bytes(data, startingInd);
            const int secondByte = getNext2Bytes(data, startingInd+2);
            return (firstByte << 16) | secondByte;    
        };

        numBits_ = buildNumberOfBitsUsed(ccheader, 8);
        assert(numBits_ > 0);
        int numBytes = (numBits_ + 7)/8; // round up for partial bytes
        bitarray_.reserve(numBits_);

        char byte;
        std::vector<uint8_t> tmp{};
        tmp.reserve(numBytes);
        while(binfile.read(&byte, sizeof(byte)))
        {
            tmp.push_back(static_cast<uint8_t>(byte));
        }
        for(const auto byte : tmp){
            for(int i = 0; i < 8; ++i){
                bitarray_.push_back((byte >> (7-i)) & 0x1);
            }
        }
        assert(!tmp.empty());
        assert(!bitarray_.empty());
    }
    void unserialize2(std::ifstream& binfile) {
        std::ostringstream out;
        out << binfile.rdbuf();
        std::string str = out.str();
        std::string_view ccheader{str.begin(), str.begin()+12};
        const std::string prefix{static_cast<char>(ccheader[0]),static_cast<char>(ccheader[1]),static_cast<char>(ccheader[2]),static_cast<char>(ccheader[3])};
        assert(prefix == "CCBF" && "Prefix is incorrect");

        auto getNext2Bytes = [](const std::string_view& data, int startingInd){
            auto value = data[startingInd] << 8;
            value |= data[startingInd+1];
            return value;
        };

        version_ = getNext2Bytes(ccheader, 4);
        numHashes_ = getNext2Bytes(ccheader, 6);

        auto buildNumberOfBitsUsed = [getNext2Bytes](const std::string_view data, int startingInd){
            const int first2Bytes = getNext2Bytes(data, startingInd);
            const int second2Bytes = getNext2Bytes(data, startingInd+2);
            return (first2Bytes << 16) | second2Bytes;    
        };

        numBits_ = buildNumberOfBitsUsed(ccheader, 8);
        assert(numBits_ > 0);
        bitarray_.reserve(numBits_);
        std::string_view tmp{str.begin()+12, str.end()};

        for(const auto byte : tmp){
            for(int i = 0; i < 8; ++i){
                bitarray_.push_back((byte >> (7-i)) & 0x1);
            }
        }
        assert(!tmp.empty());
        assert(!bitarray_.empty());
    }
};
}