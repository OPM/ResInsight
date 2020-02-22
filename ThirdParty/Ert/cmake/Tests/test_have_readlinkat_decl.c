#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>

#define NCHAR 63

int main(int argc, char *argv[])
{
    char linkname[NCHAR + 1] = { 0 };
    ssize_t r;

    r = (argc < 2) ? -1
        : readlinkat(1, argv[1], linkname, NCHAR);

    return (r > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
