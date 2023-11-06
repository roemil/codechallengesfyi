#include "../include/CommandVisitor.h"

#include "../include/Counters.h"

void CommandVisitor::visit(FileSizeCounter* fileSizeCounter) const
{
    fileSizeCounter->count(filename);
}

void CommandVisitor::visit(LineCounter* lineCounter) const
{
    lineCounter->count(filename);
}
void CommandVisitor::visit(WordCounter* wordCounter) const
{
    wordCounter->count(filename);
}

void CommandVisitor::visit(CharCounter* charCounter) const
{
    charCounter->count(filename);
}