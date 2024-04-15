#include "BloomFilter.h"

#include <cmath>

#include <cstdint>
#include <gtest/gtest.h>

constexpr int n = 1000000;
constexpr double epsilon = 0.02;

constexpr int m = (- n * std::log(epsilon) / std::pow(std::log(2),2));

TEST(BloomFilterTest, basic){
    const bf::BloomFilter bf{};
    EXPECT_EQ(bf.version_ , 0);
}

TEST(BloomFilterTest, LookUp){
    bf::BloomFilter bf{m, 1, 4};
    std::cout << "Building dict...\n";
    std::ifstream infile("../src/dict.txt");
    assert(infile.is_open() && "file not open");
    std::string line;
    while (std::getline(infile, line))
    {
        bf.buildBitArray(line);
    }

    EXPECT_TRUE(bf.lookup("hello"));
    EXPECT_TRUE(bf.lookup("what"));
    EXPECT_TRUE(bf.lookup("now"));
    EXPECT_TRUE(bf.lookup("clock"));
    EXPECT_FALSE(bf.lookup("cloc"));

}

void verifyBitArray(const std::vector<bool>& expected, const std::vector<bool>& actual, int numBits){
    // We can only verify up to numBits since actual might be padded to complete the last byte.
    for(int i = 0; i < numBits; ++i){
        EXPECT_EQ(expected[i], actual[i]);
    }
}

TEST(BloomFilterTest, SerializeUnserialize){
    bf::BloomFilter bf{m, 1, 4};
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
    std::cout << "Done...\n";

    std::ifstream binfile("words.bf", std::ios::binary);
    assert(binfile.is_open() && "file not open");
    bf::BloomFilter bfUnserialize{};
    std::cout << "Building bloom filter from file...\n";
    bfUnserialize.unserialize(binfile);
    std::cout << "Done...\n";

    EXPECT_EQ(bfUnserialize.numBits_, m);
    EXPECT_EQ(bfUnserialize.version_, 1);
    EXPECT_EQ(bfUnserialize.numHashes_, 4);
    verifyBitArray(bf.bitarray_, bfUnserialize.bitarray_, m);

    EXPECT_TRUE(bfUnserialize.lookup("hello"));
    EXPECT_TRUE(bfUnserialize.lookup("what"));
    EXPECT_TRUE(bfUnserialize.lookup("now"));
    EXPECT_TRUE(bfUnserialize.lookup("clock"));
    EXPECT_FALSE(bfUnserialize.lookup("cloc"));

}

TEST(BloomFilterTest, SerializeUnserialize2){
    bf::BloomFilter bf{m, 1, 4};
    std::cout << "Building dict...\n";
    std::ifstream infile("../src/dict.txt");
    assert(infile.is_open() && "file not open");

    std::string line;
    while (std::getline(infile, line))
    {
        bf.buildBitArray(line);
    }

    std::ofstream fs("words.bf", std::ios::out | std::ios::binary | std::ios::app);
//    bf.serialize(fs);
    std::cout << "Done...\n";

    std::ifstream binfile("words.bf", std::ios::binary);
    assert(binfile.is_open() && "file not open");
    bf::BloomFilter bfUnserialize{};
    std::cout << "Building bloom filter from file...\n";
    bfUnserialize.unserialize2(binfile);
    std::cout << "Done...\n";

    EXPECT_EQ(bfUnserialize.numBits_, m);
    EXPECT_EQ(bfUnserialize.version_, 1);
    EXPECT_EQ(bfUnserialize.numHashes_, 4);
    verifyBitArray(bf.bitarray_, bfUnserialize.bitarray_, m);

    EXPECT_TRUE(bfUnserialize.lookup("hello"));
    EXPECT_TRUE(bfUnserialize.lookup("what"));
    EXPECT_TRUE(bfUnserialize.lookup("now"));
    EXPECT_TRUE(bfUnserialize.lookup("clock"));
    EXPECT_FALSE(bfUnserialize.lookup("cloc"));

}

TEST(BloomFilterTest, writer){
    std::ofstream fs("writer.bf", std::ios::out | std::ios::binary | std::ios::app);
    std::vector<uint8_t> dataToWrite{};
    for(int i = 0; i < 64; ++i){
        dataToWrite.push_back(i%2);
    }
    fs.write(reinterpret_cast<const char*>(dataToWrite.data()), dataToWrite.size());
    fs.close();

    std::ifstream binfile("writer.bf", std::ios::in |std::ios::binary);
    assert(binfile.is_open() && "file not open");
    std::ostringstream out;
    out << binfile.rdbuf();
    std::string str = out.str();
    std::vector<uint8_t> dataRead{};
    std::copy(str.begin(), str.end(), std::back_inserter(dataRead));
    
    EXPECT_EQ(dataRead.size(), dataToWrite.size());
    for(int i = 0; i < 64; ++i){
        if(dataToWrite[i] != dataRead[i]){
            std::cout << i << "\n";
            FAIL();
        }
    }

    EXPECT_EQ(dataToWrite, dataRead);

}