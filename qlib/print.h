#ifndef PRINT_H
#define PRINT_H

enum
{
    PRINT_DEFAULT,
    PRINT_ERROR,
    PRINT_WARNING,
};

void print(const char *msg, ...);
void logprint(const char *where, const char *msg, ...);

#endif // PRINT_H
