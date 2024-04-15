#pragma once

#include <cstdint>
#include <string_view>

constexpr uint64_t FNV_offset_basis = 14695981039346656037U;
constexpr uint64_t FNV_prime = 1099511628211;

[[nodiscard]] constexpr uint64_t fnv1(std::string_view str) {
    uint64_t hash = FNV_offset_basis;

    for (const auto c : str){
        hash *= FNV_prime;
        hash ^= c;
    }

    return hash;
}

[[nodiscard]] constexpr uint64_t fnv1(uint64_t data) {
    uint64_t hash = FNV_offset_basis;

    for (int i = 0; i < 8; ++i){
        hash *= FNV_prime;
        const uint8_t byte = (data >> i) & 0xff;
        hash ^= byte;
    }

    return hash;
}