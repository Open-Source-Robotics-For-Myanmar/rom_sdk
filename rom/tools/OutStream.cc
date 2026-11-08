#include "rom/tools/OutStream.h"

#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>

#include <unistd.h>

namespace rom::tools
{

namespace
{

struct Buffer
{
    char data[8192];
    std::size_t len = 0;
    bool at_line_start = true;
};

Buffer& buf()
{
    thread_local Buffer b;
    return b;
}

std::mutex& out_mutex()
{
    static std::mutex m;
    return m;
}

void direct_write(const char* data, std::size_t len)
{
    std::lock_guard<std::mutex> lock(out_mutex());
    std::size_t written = 0;
    while (written < len) {
        const ssize_t n = ::write(STDOUT_FILENO, data + written, len - written);
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (n == 0) break;
        written += static_cast<std::size_t>(n);
    }
}

void flush_now(Buffer& b)
{
    if (b.len == 0) return;
    direct_write(b.data, b.len);
    b.len = 0;
}

void raw_append(Buffer& b, const char* data, std::size_t len)
{
    if (len > sizeof(b.data)) {
        flush_now(b);
        direct_write(data, len);
        return;
    }
    if (b.len + len > sizeof(b.data)) {
        flush_now(b);
    }
    std::memcpy(b.data + b.len, data, len);
    b.len += len;
}

// Monotonic steady_clock timestamp, e.g. "[1234.567890123] ".
std::size_t format_timestamp(char* out, std::size_t cap)
{
    const int64_t total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                 std::chrono::steady_clock::now().time_since_epoch())
                                 .count();
    const int64_t sec = total_ns / 1'000'000'000;
    const int64_t nsec = total_ns % 1'000'000'000;
    const int n = std::snprintf(out, cap, "[%" PRId64 ".%09" PRId64 "] ", sec, nsec);
    return n > 0 ? static_cast<std::size_t>(n) : 0;
}

void append_with_timestamp(Buffer& b, const char* data, std::size_t len)
{
    if (b.at_line_start) {
        char ts[32];
        const std::size_t ts_len = format_timestamp(ts, sizeof(ts));
        raw_append(b, ts, ts_len);
        b.at_line_start = false;
    }
    raw_append(b, data, len);
}

}  // namespace

OutStream& OutStream::operator<<(bool value)
{
    return value ? write_raw("true", 4) : write_raw("false", 5);
}

OutStream& OutStream::operator<<(char value) { return write_raw(&value, 1); }

OutStream& OutStream::operator<<(const char* value)
{
    return write_raw(value, std::strlen(value));
}

OutStream& OutStream::operator<<(const std::string& value)
{
    return write_raw(value.data(), value.size());
}

OutStream& OutStream::operator<<(std::string_view value)
{
    return write_raw(value.data(), value.size());
}

OutStream& OutStream::operator<<(const void* ptr)
{
    char tmp[20] = "0x";
    auto res = std::to_chars(tmp + 2, tmp + sizeof(tmp),
                             reinterpret_cast<std::uintptr_t>(ptr), 16);
    return write_raw(tmp, static_cast<std::size_t>(res.ptr - tmp));
}

OutStream& OutStream::operator<<(const Style& s) { return write_raw(s.data, s.len); }

OutStream& OutStream::operator<<(Color c) { return (*this) << color(c); }

OutStream& OutStream::operator<<(Manip)
{
    flush_now(buf());
    return *this;
}

OutStream& OutStream::write_raw(const char* data, std::size_t len)
{
    Buffer& b = buf();
    // Any '\n' byte ends the current line: flush immediately and start the
    // next line with a fresh timestamp.
    std::size_t start = 0;
    for (std::size_t i = 0; i < len; ++i) {
        if (data[i] != '\n') continue;
        append_with_timestamp(b, data + start, i - start + 1);
        flush_now(b);
        b.at_line_start = true;
        start = i + 1;
    }
    if (start < len) {
        append_with_timestamp(b, data + start, len - start);
    }
    return *this;
}

}  // namespace rom::tools
