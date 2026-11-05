#include "rom/tools/Style.h"

#include <string>

#include <gtest/gtest.h>

namespace rom::tools
{
namespace
{

std::string Str(const Style& s) { return std::string(s.data, s.len); }

TEST(StyleTest, NormalAndBrightForeground)
{
    EXPECT_EQ(Str(color(Color::Default)), "\033[39m");
    EXPECT_EQ(Str(color(Color::Red)), "\033[31m");
    EXPECT_EQ(Str(color(Color::BrightBlue)), "\033[94m");
}

TEST(StyleTest, Bold)
{
    EXPECT_EQ(Str(bold(Color::Green)), "\033[1;32m");
    EXPECT_EQ(Str(color(Color::Green, true)), "\033[1;32m");
}

TEST(StyleTest, DarkUsesFixedRgb)
{
    EXPECT_EQ(Str(color(Color::DarkRed)), "\033[38;2;139;0;0m");
    EXPECT_EQ(Str(bold(Color::DarkGreen)), "\033[1;38;2;0;100;0m");
    EXPECT_EQ(Str(bg(Color::DarkBlue)), "\033[48;2;0;0;139m");
    EXPECT_EQ(Str(dark_black), Str(color(Color::DarkBlack)));
}

TEST(StyleTest, Background)
{
    EXPECT_EQ(Str(bg(Color::Default)), "\033[49m");
    EXPECT_EQ(Str(bg(Color::Red)), "\033[41m");
    EXPECT_EQ(Str(bg(Color::BrightRed)), "\033[101m");
}

TEST(StyleTest, PaletteAndTrueColor)
{
    EXPECT_EQ(Str(color256(202)), "\033[38;5;202m");
    EXPECT_EQ(Str(bg256(17)), "\033[48;5;17m");
    EXPECT_EQ(Str(rgb(255, 105, 180)), "\033[38;2;255;105;180m");
    EXPECT_EQ(Str(bg_rgb(1, 2, 3)), "\033[48;2;1;2;3m");
}

TEST(StyleTest, Reset)
{
    EXPECT_EQ(Str(reset), "\033[0m");
}

}  // namespace
}  // namespace rom::tools
