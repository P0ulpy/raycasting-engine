#pragma once

#include <raylib.h>
#include <cstdint>

constexpr Color MY_RED          { 166, 46, 90, 255 };
constexpr Color MY_PURPLE       { 89, 27, 79, 255 };
constexpr Color MY_DARK_BLUE    { 40, 17, 64, 255 };
constexpr Color MY_BEIGE        { 242, 211, 172, 255 };
constexpr Color MY_BLACK        { 50, 50, 50, 255 };

constexpr Color ColorAlpha255(Color color, uint8_t alpha)
{
    return { color.r, color.g, color.b, alpha };
}