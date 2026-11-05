#include "rom/tools/Line.h"

#include "rom/tools/Style.h"

namespace rom::tools
{

Line::~Line()
{
    if (active_) {
        stream() << reset << "\n";
    }
}

OutStream& Line::stream()
{
    static OutStream instance;
    return instance;
}

}  // namespace rom::tools
