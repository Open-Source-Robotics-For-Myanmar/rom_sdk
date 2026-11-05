#ifndef ROM_TOOLS_LINE_H_
#define ROM_TOOLS_LINE_H_

#include "rom/tools/OutStream.h"

namespace rom::tools
{

// The temporary returned by a `rom::tools::cout << ...;` statement. Its destructor
// runs once, at the statement's closing ';', and appends rom::tools::reset and a
// trailing "\n", so every statement is a complete, reset, flushed line.
// The move constructor + `active_` flag keep this correct even if the
// compiler does not elide the temporary returned by CoutTag::operator<<.
class Line
{
public:
    Line() = default;
    Line(const Line&) = delete;
    Line& operator=(const Line&) = delete;
    Line& operator=(Line&&) = delete;
    Line(Line&& other) noexcept : active_(other.active_) { other.active_ = false; }
    ~Line();

    template <typename T>
    Line& operator<<(const T& value)
    {
        stream() << value;
        return *this;
    }

private:
    static OutStream& stream();

    bool active_ = true;
};

}  // namespace rom::tools

#endif  // ROM_TOOLS_LINE_H_
