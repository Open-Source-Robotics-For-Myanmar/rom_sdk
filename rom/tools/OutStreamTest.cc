#include "rom/tools/OutStream.h"

#include <cctype>
#include <string>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>

#include "rom/tools/CoutTag.h"

namespace rom::tools
{
namespace
{

// Redirects STDOUT_FILENO into a pipe for the lifetime of the object.
class StdoutCapture
{
public:
    StdoutCapture()
    {
        EXPECT_EQ(::pipe(fds_), 0);
        saved_ = ::dup(STDOUT_FILENO);
        ::dup2(fds_[1], STDOUT_FILENO);
    }

    ~StdoutCapture() { Restore(); }

    std::string Finish()
    {
        Restore();
        std::string out;
        char tmp[4096];
        ssize_t n;
        while ((n = ::read(fds_[0], tmp, sizeof(tmp))) > 0) {
            out.append(tmp, static_cast<std::size_t>(n));
        }
        ::close(fds_[0]);
        return out;
    }

private:
    void Restore()
    {
        if (saved_ < 0) return;
        ::dup2(saved_, STDOUT_FILENO);
        ::close(saved_);
        ::close(fds_[1]);
        saved_ = -1;
    }

    int fds_[2]{-1, -1};
    int saved_ = -1;
};

// Splits output into lines and strips the "[seconds.nanoseconds] " prefix,
// failing the test if a line has no valid timestamp.
std::vector<std::string> StripTimestamps(const std::string& out)
{
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start < out.size()) {
        std::size_t end = out.find('\n', start);
        if (end == std::string::npos) end = out.size();
        const std::string line = out.substr(start, end - start);
        start = end + 1;

        const std::size_t close = line.find("] ");
        EXPECT_EQ(line.rfind('[', 0), 0u) << line;
        EXPECT_NE(close, std::string::npos) << line;
        if (close == std::string::npos) continue;
        const std::string ts = line.substr(1, close - 1);
        const std::size_t dot = ts.find('.');
        EXPECT_NE(dot, std::string::npos) << ts;
        EXPECT_EQ(ts.size() - dot - 1, 9u) << ts;
        for (char c : ts) {
            EXPECT_TRUE(std::isdigit(static_cast<unsigned char>(c)) || c == '.') << ts;
        }
        lines.push_back(line.substr(close + 2));
    }
    return lines;
}

TEST(OutStreamTest, CoutWritesOneResetLinePerStatement)
{
    StdoutCapture capture;
    rom::tools::cout << "score: " << 42;
    const auto lines = StripTimestamps(capture.Finish());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "score: 42\033[0m");
}

TEST(OutStreamTest, FormatsBasicTypes)
{
    StdoutCapture capture;
    rom::tools::cout << true << ' ' << false << ' ' << -7 << ' ' << 1.5 << ' '
              << std::string("s") << ' ' << std::string_view("v");
    const auto lines = StripTimestamps(capture.Finish());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "true false -7 1.5 s v\033[0m");
}

TEST(OutStreamTest, ColorIsWrittenAndReset)
{
    StdoutCapture capture;
    rom::tools::cout << rom::tools::Color::Red << "uh oh";
    const auto lines = StripTimestamps(capture.Finish());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "\033[31muh oh\033[0m");
}

TEST(OutStreamTest, EmbeddedNewlineStartsNewTimestampedLine)
{
    StdoutCapture capture;
    rom::tools::cout << "a" << "\nb";
    const auto lines = StripTimestamps(capture.Finish());
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "a");
    EXPECT_EQ(lines[1], "b\033[0m");
}

}  // namespace
}  // namespace rom::tools
