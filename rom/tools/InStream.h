#ifndef ROM_TOOLS_INSTREAM_H_
#define ROM_TOOLS_INSTREAM_H_

#include <charconv>
#include <mutex>
#include <string>
#include <type_traits>

namespace rom::tools
{

// A fast, thread-safe, syscall-backed stand-in for std::cin.
// Reads stdin through raw ::read() calls and parses numbers with
// std::from_chars (locale-free).
class InStream
{
public:
    template <typename T>
    InStream& operator>>(T& value)
    {
        std::lock_guard<std::mutex> lock(in_mutex());
        read_value(value);
        return *this;
    }

    // Reads up to (and consuming) the next '\n'. Returns false at EOF.
    bool getline(std::string& line);

private:
    static std::mutex& in_mutex();
    static void read_token(std::string& token);

    static void read_value(std::string& value);
    static void read_value(char& value);
    static void read_value(bool& value);

    template <typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>, int> = 0>
    static void read_value(T& value)
    {
        std::string token;
        read_token(token);
        std::from_chars(token.data(), token.data() + token.size(), value);
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    static void read_value(T& value)
    {
        std::string token;
        read_token(token);
        std::from_chars(token.data(), token.data() + token.size(), value);
    }
};

// rom::tools::cin >> value;
inline InStream cin;

}  // namespace rom::tools

#endif  // ROM_TOOLS_INSTREAM_H_
