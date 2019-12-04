#ifndef ERROR_H
#define ERROR_H

#define TINY_EOF -1
#define TINY_UNEXPECTED_EOF -2
#define TINY_UNEXPECTED_TOKEN -3
#define TINY_INVALID_STRING -4
#define TINY_INVALID_STRING_X_NO_FOLLOWING_HEX_DIGITS -5
#define TINY_INVALID_PARSER -6

void error(const char *format, ...);

#endif // ERROR_H