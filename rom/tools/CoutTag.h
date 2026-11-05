#ifndef ROM_TOOLS_COUTTAG_H_
#define ROM_TOOLS_COUTTAG_H_

#include "rom/tools/Color.h"
#include "rom/tools/Line.h"
#include "rom/tools/Style.h"

namespace rom::tools
{

// rom::tools::cout: one statement = one timestamped, color-reset, newline-terminated
// line. There is no rom::tools::endl; never write rom::tools::reset or "\n" yourself:
//   rom::tools::cout << "score: " << score;
//   rom::tools::cout << rom::tools::Color::Red << "uh oh";
//   rom::tools::cout << rom::tools::bold(rom::tools::Color::Green) << "ok";
//   rom::tools::cout << rom::tools::rgb(255, 105, 180) << "pink";
// A literal "\n" mid-statement still starts a new, separately timestamped line.
struct CoutTag
{
    template <typename T>
    Line operator<<(const T& value) const
    {
        Line line;
        line << value;
        return line;
    }
};

inline CoutTag cout;

}  // namespace rom::tools

#endif  // ROM_TOOLS_COUTTAG_H_
