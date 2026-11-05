#include "rom/tools/InStream.h"

#include <string>

#include <unistd.h>

#include <gtest/gtest.h>

namespace rom::tools
{
namespace
{

TEST(InStreamTest, ReadsTokensAndLines)
{
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);
    const std::string input = "42 3.5 hello true x\nline two\n";
    ASSERT_EQ(::write(fds[1], input.data(), input.size()),
              static_cast<ssize_t>(input.size()));
    ::close(fds[1]);

    const int saved = ::dup(STDIN_FILENO);
    ::dup2(fds[0], STDIN_FILENO);
    ::close(fds[0]);

    int i = 0;
    double d = 0.0;
    std::string s;
    bool b = false;
    char c = 0;
    rom::tools::cin >> i >> d >> s >> b >> c;

    std::string rest;
    const bool got_rest = rom::tools::cin.getline(rest);
    std::string second;
    const bool got_second = rom::tools::cin.getline(second);
    std::string eof_line;
    const bool got_eof = rom::tools::cin.getline(eof_line);

    ::dup2(saved, STDIN_FILENO);
    ::close(saved);

    EXPECT_EQ(i, 42);
    EXPECT_NEAR(d, 3.5, 1e-12);
    EXPECT_EQ(s, "hello");
    EXPECT_TRUE(b);
    EXPECT_EQ(c, 'x');
    EXPECT_TRUE(got_rest);
    EXPECT_EQ(rest, "");
    EXPECT_TRUE(got_second);
    EXPECT_EQ(second, "line two");
    EXPECT_FALSE(got_eof);
}

}  // namespace
}  // namespace rom::tools
