#pragma once

#include <string>

struct CommandVisitor;

struct ICounter
{
    virtual void accept(const CommandVisitor&) = 0;
    virtual void count(const std::string&) = 0;
    virtual void print() const = 0;
};

struct FileSizeCounter : ICounter
{
    int filesize{};
    void accept(const CommandVisitor& visitor);
    void count(const std::string& filename);
    void print() const;
};
struct LineCounter : ICounter
{
    int lineCount{};
    void accept(const CommandVisitor& visitor);
    void count(const std::string& filename);
    void print() const;
};
struct WordCounter : ICounter
{
    int wordCount{};
    void accept(const CommandVisitor& visitor);
    void count(const std::string& filename);
    void print() const;
};
struct CharCounter : ICounter
{
    int charCount{};
    void accept(const CommandVisitor& visitor);
    void count(const std::string& filename);
    void print() const;
};
