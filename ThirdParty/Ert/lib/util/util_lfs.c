/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'util_lfs.c' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

/*
  This file, together with ifdef and typedef in util.h together try to
  define transparent large file (> 2GB) support on windows and
  linux. To support large files the datatype used to hold a file
  offset must be 64 bit, in practical consequences this means:

    - functions ftell() and fseek() must use 64 bit offset types.
    - The size field in struct stat must be 64 bit.

  On linux (at least on 64 bit platform) this is the deafult,
  i.e. large files can be accessed out of the box. On windows the
  situaton is more complicated:

     - functions ftell() and fseek() expect 32 bit offset types.
     - The size field in struct stat is a 32 bit variable.

  Observe that the situation where 32 bit offset variables are used on
  windows apply even on a 64 bit platform. To provide large file
  support windows has the functions _ftelli64() and _fseeki64() and
  the struct _stat64. Here we provide small wrapper functions
  util_ftell(), util_fseek() and typedef struct stat_info.

  The final challenge is that the types 'long' and 'off_t == long'
  have different size on windows and linux:


     Windows(64 bit): sizeof(long)  = 4
                      sizeof(off_t) = 4

     Linux(64 bit):   sizeof(long)  = 8
                      sizeof(off_t) = 8


  To protect against this confusion we have typedefed a type
  'offset_type' in util.h, and all file operations should use that type.
*/

#include <ert/util/util.h>

offset_type util_ftell(FILE * stream) {
#ifdef ERT_WINDOWS_LFS
  return _ftelli64(stream);
#else
  #ifdef HAVE_FSEEKO
  return ftello(stream);
  #else
  return ftell(stream);
  #endif
#endif
}



int util_fseek(FILE * stream, offset_type offset, int whence) {
#ifdef ERT_WINDOWS_LFS
  return _fseeki64(stream , offset , whence);
#else
  #ifdef HAVE_FSEEKO
  return fseeko( stream , offset , whence );
  #else
  return fseek( stream , offset , whence );
  #endif
#endif
}



void util_rewind(FILE * stream) {
#ifdef ERT_WINDOWS_LFS
  _fseeki64(stream , 0L , SEEK_SET);
#else
  rewind( stream );
#endif
}





int util_stat(const char * filename , stat_type * stat_info) {
#ifdef ERT_WINDOWS_LFS
  return _stat64(filename , stat_info);
#else
  return stat(filename , stat_info);
#endif
}


int util_fstat(int fileno, stat_type * stat_info) {
#ifdef ERT_WINDOWS_LFS
  return _fstat64(fileno , stat_info);
#else
  return fstat(fileno , stat_info);
#endif
}
