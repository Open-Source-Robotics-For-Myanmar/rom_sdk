#include "rom/tools/InStream.h"

#include <cctype>
#include <cerrno>
#include <cstddef>

#include <unistd.h>

namespace rom::tools
{

namespace
{

struct Buffer
{
    char data[8192];
    std::size_t pos = 0;
    std::size_t len = 0;
};

Buffer& buf()
{
    thread_local Buffer b;
    return b;
}

bool refill()
{
    Buffer& b = buf();
    ssize_t n;
    do {
        n = ::read(STDIN_FILENO, b.data, sizeof(b.data));
    } while (n < 0 && errno == EINTR);
    if (n <= 0) return false;
    b.pos = 0;
    b.len = static_cast<std::size_t>(n);
    return true;
}

bool peek_char(char& c)
{
    Buffer& b = buf();
    if (b.pos >= b.len && !refill()) return false;
    c = b.data[b.pos];
    return true;
}

bool next_char(char& c)
{
    if (!peek_char(c)) return false;
    ++buf().pos;
    return true;
}

void skip_whitespace()
{
    char c;
    while (peek_char(c) && std::isspace(static_cast<unsigned char>(c))) {
        ++buf().pos;
    }
}

}  // namespace

bool InStream::getline(std::string& line)
{
    std::lock_guard<std::mutex> lock(in_mutex());
    line.clear();
    char c;
    bool got_any = false;
    while (next_char(c)) {
        got_any = true;
        if (c == '\n') break;
        line.push_back(c);
    }
    return got_any;
}

std::mutex& InStream::in_mutex()
{
    static std::mutex m;
    return m;
}

void InStream::read_token(std::string& token)
{
    token.clear();
    skip_whitespace();
    char c;
    while (peek_char(c) && !std::isspace(static_cast<unsigned char>(c))) {
        token.push_back(c);
        ++buf().pos;
    }
}

void InStream::read_value(std::string& value) { read_token(value); }

void InStream::read_value(char& value)
{
    skip_whitespace();
    char c;
    if (next_char(c)) value = c;
}

void InStream::read_value(bool& value)
{
    std::string token;
    read_token(token);
    value = (token == "1" || token == "true");
}

}  // namespace rom::tools
