#pragma once

#include "HashFunction.h"
#include <cstdint>
#include <string_view>
#include <string>
#include <bitset>
#include <vector>
#include <arpa/inet.h>
#include <fstream>
#include <cassert>

namespace bf
{
template<typename std::size_t BitSetSize>
struct BloomFilter
{
    std::bitset<BitSetSize> bitarray_;

    int m_{BitSetSize};
    uint16_t version = 1;
    uint16_t numHashes = 4;

    void buildBitArray(const std::string_view str) {
        auto hash = fnv1(str);
        bitarray_[hash % m_] = 1;
        for(int i = 1; i < numHashes; ++i){
            hash = fnv1(hash);
            bitarray_[hash % m_] = 1;
        }
    }
    bool lookup(const std::string_view str) const {
        bool lookUpRes = true;
        auto hash = fnv1(str);
        lookUpRes &= bitarray_[hash % m_];
        
        for(int i = 1; i < numHashes; ++i){
            hash = fnv1(str);
            lookUpRes &= bitarray_[hash % m_];
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

        writeHeaderToFile(fs, htons(version), htons(numHashes), m_);

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
        for(int i = 0; i < m_; i += 8){
            bytes.push_back(getByte(i));
        }

        fs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        fs.close();
    }


};

struct BloomFilterLookUp
{
    std::vector<bool> bitarray_;
    int numBits_{};

    uint16_t version{};
    uint16_t numHashes{};
    BloomFilterLookUp() = default;

    bool lookup(const std::string_view str) const {
        bool lookUpRes = true;
        auto hash = fnv1(str);
        lookUpRes &= bitarray_[hash % numBits_];
        for(int i = 1; i < numHashes; ++i){
            hash = fnv1(str);
            lookUpRes &= bitarray_[hash % numBits_];
        }
        return lookUpRes;
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

        version = (ccheader[4] << 8);
        version |= ccheader[5];
        assert(version == 1);

        numHashes = (ccheader[6] << 8);
        numHashes |= ccheader[7];

        int firstByte = 0;
        firstByte = (ccheader[8] << 8);
        firstByte |= ccheader[9];

        int secondByte = 0;
        secondByte = (ccheader[10] << 8);
        secondByte |= ccheader[11];
        numBits_ = (firstByte << 16) | secondByte;
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
};
}