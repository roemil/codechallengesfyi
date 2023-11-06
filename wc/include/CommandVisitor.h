#pragma once

#include "CommandVisitor.h"

#include <string>

struct FileSizeCounter;
struct LineCounter;
struct WordCounter;
struct CharCounter;

struct CommandVisitor
{
    explicit CommandVisitor(const std::string& filename) : filename(filename) {}
    std::string filename{};
    int fileSize{};
    void visit(FileSizeCounter* fileSizeCounter) const;
    int lineCount{};
    void visit(LineCounter* lineCounter) const;
    int wordCount{};
    void visit(WordCounter* wordCounter) const;
    int charCount{};
    void visit(CharCounter* wordCounter) const;
};