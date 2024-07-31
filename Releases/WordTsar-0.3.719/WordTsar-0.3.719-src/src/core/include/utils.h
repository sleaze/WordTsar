#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string strformat(const char *fmt, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)))
#endif
    ;

#endif // UTILS_H
