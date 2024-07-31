#include <cstdio>
#include <cstdarg>

#include "src/core/include/utils.h"

#if __cplusplus < 201703L
#include <memory>
#endif

std::string strformat(const char *fmt, ...)
{
    char buf[256];

    va_list args;
    va_start(args, fmt);
    const auto r = std::vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

    if (r < 0)
        // conversion failed
        return {};

    const size_t len = r;
    if (len < sizeof buf)
        // we fit in the buffer
        return { buf, len };

#if __cplusplus >= 201703L
    // C++17: Create a string and write to its underlying array
    std::string s(len, '\0');
    va_start(args, fmt);
    std::vsnprintf(s.data(), len+1, fmt, args);
    va_end(args);

    return s;
#else
    // C++11 or C++14: We need to allocate scratch memory
    auto vbuf = std::unique_ptr<char[]>(new char[len+1]);
    va_start(args, fmt);
    std::vsnprintf(vbuf.get(), len+1, fmt, args);
    va_end(args);

    return { vbuf.get(), len };
#endif
}

