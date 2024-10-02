#pragma once

#include <raylib.h>
#include <cstdint>

constexpr Color MY_RED          { 166, 46, 90, 255 };
constexpr Color MY_PURPLE       { 89, 27, 79, 255 };
constexpr Color MY_DARK_BLUE    { 40, 17, 64, 255 };
constexpr Color MY_BEIGE        { 242, 211, 172, 255 };
constexpr Color MY_BLACK        { 50, 50, 50, 255 };

/// @brief 
/// @param color base color to be darken
/// @param darkness 0 to 1 value
/// @return Darken color
inline Color ColorDarken(Color color, float darkness)
{
    float clampedDarkness = Clamp(darkness, 0, 1);

    uint8_t red   = roundf(Lerp(color.r, 0, clampedDarkness));
    uint8_t green = roundf(Lerp(color.g, 0, clampedDarkness));
    uint8_t blue  = roundf(Lerp(color.b, 0, clampedDarkness));

    return { red, green, blue, 255 };
}