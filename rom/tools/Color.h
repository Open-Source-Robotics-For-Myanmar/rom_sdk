#ifndef ROM_TOOLS_COLOR_H_
#define ROM_TOOLS_COLOR_H_

namespace rom::tools
{

// 16 named ANSI colors (8 standard "Normal" + 8 "Bright") plus 8 "Dark"
// variants - one of each hue, so every color has 3 distinguishable shades:
// Dark < Normal < Bright. Normal (ANSI 30-37) and Bright (ANSI 90-97) follow
// the terminal's own color theme; Dark is a fixed 24-bit RGB value chosen to
// be reliably darker than Normal regardless of theme. For anything beyond
// these 24 colors, use color256()/bg256() or rgb()/bg_rgb() (see Style.h).
enum class Color
{
    Default,
    // Normal: ANSI 30-37.
    Black,   Red,   Green,   Yellow,   Blue,   Magenta,   Cyan,   White,
    // Bright: ANSI 90-97.
    BrightBlack, BrightRed, BrightGreen, BrightYellow, BrightBlue, BrightMagenta, BrightCyan, BrightWhite,
    // Dark: fixed RGB, always darker than Normal.
    DarkBlack, DarkRed, DarkGreen, DarkYellow, DarkBlue, DarkMagenta, DarkCyan, DarkWhite,
};

}  // namespace rom::tools

#endif  // ROM_TOOLS_COLOR_H_
