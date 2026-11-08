#ifndef ROM_TOOLS_OUTSTREAM_H_
#define ROM_TOOLS_OUTSTREAM_H_

#include <charconv>
#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>
#include <type_traits>

#include "rom/tools/Color.h"
#include "rom/tools/Style.h"

namespace rom::tools
{

// A fast, thread-safe, syscall-backed stand-in for std::cout.
//
// talks to the terminal only through raw ::write() calls and formats numbers
// with std::to_chars (locale-free, no allocation). A thread-local buffer
// accumulates each line; the output mutex is taken only once per line to
// flush it as a single write(), so concurrent threads never interleave
// mid-line. Every line is prefixed with a steady_clock timestamp,
// "[seconds.nanoseconds] ". Any '\n' ends the current line and flushes it.
//
// Most code should use rom::tools::cout (see CoutTag.h) instead of this class.
class OutStream
{
public:
    OutStream& operator<<(bool value);
    OutStream& operator<<(char value);
    OutStream& operator<<(const char* value);
    OutStream& operator<<(const std::string& value);
    OutStream& operator<<(std::string_view value);
    OutStream& operator<<(const void* ptr);
    OutStream& operator<<(const Style& s);
    OutStream& operator<<(Color c);

    template <typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>, int> = 0>
    OutStream& operator<<(T value)
    {
        char tmp[32];
        auto res = std::to_chars(tmp, tmp + sizeof(tmp), value);
        return write_raw(tmp, static_cast<std::size_t>(res.ptr - tmp));
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    OutStream& operator<<(T value)
    {
        char tmp[64];
        auto res = std::to_chars(tmp, tmp + sizeof(tmp), value);
        return write_raw(tmp, static_cast<std::size_t>(res.ptr - tmp));
    }

    // Accepts any std::ostream manipulator (std::flush, std::endl, ...) purely
    // as a "flush now" signal. std::endl does NOT insert '\n' here; only
    // <iosfwd> is needed, so including this header never pulls in <ostream>.
    using Manip = std::ostream& (*)(std::ostream&);
    OutStream& operator<<(Manip);

private:
    OutStream& write_raw(const char* data, std::size_t len);
};

}  // namespace rom::tools

#endif  // ROM_TOOLS_OUTSTREAM_H_
