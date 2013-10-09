/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'util_chdir.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifdef HAVE_CHDIR
#include <unistd.h>
#endif

/*
#ifdef HAVE_WINDOWS_CHDIR
#include <direct.h>
#endif
*/

int util_chdir(const char * path) {
#ifdef HAVE_CHDIR
  return chdir( path );
#else
#ifdef HAVE_WINDOWS_CHDIR
  return _chdir( path );
#endif
#endif
}
