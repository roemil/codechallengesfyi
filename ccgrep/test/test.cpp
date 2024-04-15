#include "CcGrep.h"

#include <gtest/gtest.h>


TEST(ccgrep, basic)
{
    const std::string file = "../test/data/test_small.txt";
    EXPECT_EQ("helloworld\nlol\n", grep("", file));
}

TEST(ccgrep, oneLetterPattern)
{
    const std::string file = "../test/data/rockbands.txt";
    EXPECT_EQ("Judas Priest\nBon Jovi\nJunkyard\n", grep("J", file));
}

TEST(ccgrep, recursive)
{
    const std::string file = "../test/data";
    EXPECT_EQ(
    "../test/data/test-subdir/BFS1985.txt:Since Bruce Springsteen, Madonna, way before Nirvana\n"
    "../test/data/test-subdir/BFS1985.txt:On the radio was Springsteen, Madonna, way before Nirvana\n"
    "../test/data/test-subdir/BFS1985.txt:And bring back Springsteen, Madonna, way before Nirvana\n"
    "../test/data/test-subdir/BFS1985.txt:Bruce Springsteen, Madonna, way before Nirvana\n"
    "../test/data/rockbands.txt:Nirvana\n",
    grep_recursive("Nirvana", file));
}

// TEST(ccgrep, inverted)
// {
//     const std::string file = "../test/test-subdir.txt";
//     EXPECT_EQ("", grep("Nirvana", file)grep_inverted("Madonna", file));
// }