#include "../include/Counters.h"
#include "../include/CommandVisitor.h"

#include <fstream>
#include <string>
#include <iostream>

void FileSizeCounter::accept(const CommandVisitor& visitor)
{
    visitor.visit(this);
}
void FileSizeCounter::count(const std::string& filename)
{
    std::fstream file(filename, std::ios::in | std::ios::ate);
    filesize = file.tellg();
}
void FileSizeCounter::print() const
{
    std::cout << '\t' << filesize;
}

void LineCounter::accept(const CommandVisitor& visitor)
{
    visitor.visit(this);
}
void LineCounter::count(const std::string& filename)
{
    std::fstream file(filename, std::ios::in);
    lineCount = std::count(std::istreambuf_iterator<char>(file), 
                           std::istreambuf_iterator<char>(), '\n');
}
void LineCounter::print() const
{
    std::cout << '\t' << lineCount;
}

void WordCounter::accept(const CommandVisitor& visitor)
{
    visitor.visit(this);
}
void WordCounter::count(const std::string& filename)
{
    std::fstream file(filename, std::ios::in);
    std::istream_iterator<std::string> in{ file }, end;
    wordCount = std::distance(in, end);
}
void WordCounter::print() const
{
    std::cout << '\t' << wordCount;
}

void CharCounter::accept(const CommandVisitor& visitor)
{
    visitor.visit(this);
}
void CharCounter::count(const std::string& filename)
{
    std::fstream file(filename, std::ios::in);
    char ch{};
    while(file.get(ch))
    {
        ++charCount;
    }
}
void CharCounter::print() const
{
    std::cout << '\t' << charCount;
}