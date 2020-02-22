/*
  When building with MSVC this file is renamed to stdbool.h and copied into the
  list of headers. The situation is as follows:

  - The ERT code makes use of stdbool in many places.  The msvc C
    compiler does not have a stdbool header, i.e. the #include <stdbool.h>
    statements fail when compiling.

  - When included in a C++ project the compiler already has a bool
    defined; it is therefore important not to redefine this symbol if
    we are compiling C++.

*/

#ifndef __cplusplus

typedef char bool;
#define true  1
#define false 0

#endif
