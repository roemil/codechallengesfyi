#include "RespEncoder.h"
#include "Resp.h"

#include <string_view>

void RespEncoder::appendChars(const std::string_view str)
{
    for (const auto c : str) {
        buffer.push_back(c);
    }
}

void RespEncoder::RespEncoder::appendCRLF()
{
    buffer.push_back('\r');
    buffer.push_back('\n');
}

void RespEncoder::appendSimpleString(const std::string_view str)
{
    buffer.push_back(static_cast<char>(Prefix::SIMPLE_STRING));
    for (const auto& c : str) {
        if (c == '\r' || c == '\n') {
            throw std::invalid_argument("String must not contain CR or LF!");
        }
        buffer.push_back(static_cast<char>(c));
    }
    appendCRLF();
}

void RespEncoder::appendBulkstring(const std::string_view str)
{
    buffer.push_back(static_cast<char>(Prefix::BULK_STRING));
    buffer.push_back(char(str.length()) + '0');
    appendCRLF();
    // TODO: Should probably do something like buf.writeu8() or writeu32() to handle wide chars
    appendChars(str);
    appendCRLF();
}

void RespEncoder::appendNull()
{
    buffer.push_back(static_cast<char>(Prefix::BULK_STRING));
    buffer.push_back(-1);
    appendCRLF();
}

void RespEncoder::appendError(const std::string_view str)
{
    buffer.push_back(static_cast<char>(Prefix::ERROR));
    for (const auto& c : str) {
        buffer.push_back(static_cast<char>(c));
    }
    appendCRLF();
}

void RespEncoder::appendInt(const std::string_view n)
{
    buffer.push_back(static_cast<char>(Prefix::INTEGER));
    appendChars(n);
    appendCRLF();
}

void RespEncoder::beginArray(const unsigned numElements)
{
    buffer.push_back(static_cast<char>(Prefix::ARRAY));
    buffer.push_back(char(numElements) + '0');
    appendCRLF();
}

void RespEncoder::beginMap(const unsigned numElements)
{
    buffer.push_back(static_cast<char>(Prefix::MAP));
    buffer.push_back(char(numElements) + '0');
    appendCRLF();
}

void RespEncoder::appendKV(const std::string_view key, const std::string_view val)
{
    appendBulkstring(key);
    appendBulkstring(val);
}
void RespEncoder::appendKV(const std::string_view key, const int val)
{
    appendBulkstring(key);
    appendInt(std::to_string(val));
}

const std::vector<char>& RespEncoder::getBuffer() const { return buffer; }