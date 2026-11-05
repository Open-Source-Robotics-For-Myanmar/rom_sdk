#include "rom/tools/Style.h"

#include <cstdio>

namespace rom::tools
{

namespace
{

constexpr int color_slot(Color c)
{
    switch (c) {
        case Color::Black:   case Color::BrightBlack:   case Color::DarkBlack:   return 0;
        case Color::Red:     case Color::BrightRed:     case Color::DarkRed:     return 1;
        case Color::Green:   case Color::BrightGreen:   case Color::DarkGreen:   return 2;
        case Color::Yellow:  case Color::BrightYellow:  case Color::DarkYellow:  return 3;
        case Color::Blue:    case Color::BrightBlue:    case Color::DarkBlue:    return 4;
        case Color::Magenta: case Color::BrightMagenta: case Color::DarkMagenta: return 5;
        case Color::Cyan:    case Color::BrightCyan:    case Color::DarkCyan:    return 6;
        case Color::White:   case Color::BrightWhite:   case Color::DarkWhite:   return 7;
        default:             return -1;
    }
}

enum class Shade { Normal, Bright, Dark };

constexpr Shade shade_of(Color c)
{
    switch (c) {
        case Color::BrightBlack:   case Color::BrightRed:
        case Color::BrightGreen:   case Color::BrightYellow:
        case Color::BrightBlue:    case Color::BrightMagenta:
        case Color::BrightCyan:    case Color::BrightWhite:
            return Shade::Bright;
        case Color::DarkBlack:   case Color::DarkRed:
        case Color::DarkGreen:   case Color::DarkYellow:
        case Color::DarkBlue:    case Color::DarkMagenta:
        case Color::DarkCyan:    case Color::DarkWhite:
            return Shade::Dark;
        default:
            return Shade::Normal;
    }
}

struct Rgb { std::uint8_t r; std::uint8_t g; std::uint8_t b; };

// Fixed 24-bit RGB for each "Dark" Color. Values match well-known "dark" CSS
// color names where one exists; olive is used for yellow. DarkBlack avoids
// pure black, which would be invisible on a black terminal background.
constexpr Rgb dark_rgb(Color c)
{
    switch (c) {
        case Color::DarkBlack:   return Rgb{20, 20, 20};
        case Color::DarkRed:     return Rgb{139, 0, 0};
        case Color::DarkGreen:   return Rgb{0, 100, 0};
        case Color::DarkYellow:  return Rgb{128, 128, 0};
        case Color::DarkBlue:    return Rgb{0, 0, 139};
        case Color::DarkMagenta: return Rgb{139, 0, 139};
        case Color::DarkCyan:    return Rgb{0, 139, 139};
        case Color::DarkWhite:   return Rgb{105, 105, 105};
        default:                 return Rgb{0, 0, 0};
    }
}

// background=false -> 30-37 / 90-97; background=true -> 40-47 / 100-107.
// Only meaningful for Normal/Bright colors; ANSI has no "dark" SGR code.
constexpr int ansi_code(Color c, bool background)
{
    if (c == Color::Default) return background ? 49 : 39;
    const bool bright = shade_of(c) == Shade::Bright;
    const int base = background ? (bright ? 100 : 40) : (bright ? 90 : 30);
    return base + color_slot(c);
}

template <typename... Args>
Style make_style(const char* fmt, Args... args)
{
    Style s;
    const int n = std::snprintf(s.data, sizeof(s.data), fmt, args...);
    s.len = n > 0 ? static_cast<std::uint8_t>(n) : 0;
    return s;
}

}  // namespace

Style color(Color c, bool is_bold)
{
    if (shade_of(c) == Shade::Dark) {
        const Rgb v = dark_rgb(c);
        return is_bold ? make_style("\033[1;38;2;%d;%d;%dm", v.r, v.g, v.b)
                       : make_style("\033[38;2;%d;%d;%dm", v.r, v.g, v.b);
    }
    return is_bold ? make_style("\033[1;%dm", ansi_code(c, false))
                   : make_style("\033[%dm", ansi_code(c, false));
}

Style bold(Color c) { return color(c, true); }

Style bg(Color c)
{
    if (shade_of(c) == Shade::Dark) {
        const Rgb v = dark_rgb(c);
        return make_style("\033[48;2;%d;%d;%dm", v.r, v.g, v.b);
    }
    return make_style("\033[%dm", ansi_code(c, true));
}

Style color256(std::uint8_t index) { return make_style("\033[38;5;%dm", index); }
Style bg256(std::uint8_t index) { return make_style("\033[48;5;%dm", index); }

Style rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    return make_style("\033[38;2;%d;%d;%dm", r, g, b);
}

Style bg_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    return make_style("\033[48;2;%d;%d;%dm", r, g, b);
}

const Style dark_black = color(Color::DarkBlack);

}  // namespace rom::tools
