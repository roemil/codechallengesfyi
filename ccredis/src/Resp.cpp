#include "Resp.h"

#include <iostream>

bool operator==(const RedisRespRes& lhs, const RedisRespRes& rhs)
{
    return lhs.integer_ == rhs.integer_ && lhs.string_ == rhs.string_ && lhs.error_ == rhs.error_ && lhs.array_ == rhs.array_;
}

std::ostream& operator<<(std::ostream& os, const RedisRespRes& resp)
{
    os << "Int: " << resp.integer_.value_or(-1) << "\n";
    os << "String: " << resp.string_.value_or("") << "\n";
    os << "Error: " << resp.error_.value_or("") << "\n";
    if (resp.array_.has_value()) {
        for (const auto& elem : resp.array_.value()) {
            os << elem;
        }
    }
    return os;
}