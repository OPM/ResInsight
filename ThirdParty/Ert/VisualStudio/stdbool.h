/*
  The ert code is based on C99, and in particular makes extensive use
  of the C99 feature stdbool.h. When including the ert headers in a
  VisualStudio C++ project this creates problems because the
  VisualStudio C compiler is not C99 conforming, and the compiler will
  complain it can not find the stdbool.h header.

  The symbols defined in the stdbool header are actually correctly(?)
  defined by the VisualStudio compiler already, so this header file
  does not define any bool related symbols!
  
  To actually use this file you should copy it manually into the ert
  include directory as used by VisualStudio. 
*/

#ifndef ERT_STDBOOL_H
#define ERT_STDBOOL_H

#ifndef __cplusplus
typedef int bool;
#define true  1
#define false 0
#endif


#endif
