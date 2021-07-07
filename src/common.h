#ifndef COMMON_H
#define COMMON_H

#define COUT(str) printf(#str" = %s\n", str);

#define DIE(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__), exit(1)

static const char APP_NAME[] = "pageboy";

#endif
