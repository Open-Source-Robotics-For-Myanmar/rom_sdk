#ifndef ROM_TOOLS_STYLE_H_
#define ROM_TOOLS_STYLE_H_

#include <cstdint>

#include "rom/tools/Color.h"

namespace rom::tools
{

// A ready-to-write ANSI escape sequence, built once and then streamed into
// rom::tools::cout as plain bytes (no formatting work happens on the hot path).
struct Style
{
    char data[32]{};
    std::uint8_t len = 0;
};

// Foreground color, optionally bold. Dark colors use a fixed 24-bit RGB
// escape instead of an SGR code.
Style color(Color c, bool is_bold = false);
Style bold(Color c);

// Background color. Dark colors use a fixed 24-bit RGB escape.
Style bg(Color c);

// 256-color palette (0-255).
Style color256(std::uint8_t index);
Style bg256(std::uint8_t index);

// 24-bit true color (most modern terminals support this).
Style rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b);
Style bg_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b);

// Streaming `rom::tools::reset` clears color/bold back to the terminal default.
inline constexpr Style reset{{'\033', '[', '0', 'm'}, 4};

// Deprecated convenience alias, kept for backward compatibility - equivalent
// to rom::tools::color(rom::tools::Color::DarkBlack). Prefer streaming Color::DarkBlack.
extern const Style dark_black;

}  // namespace rom::tools

#endif  // ROM_TOOLS_STYLE_H_
