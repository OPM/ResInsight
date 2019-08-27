/*
   Copyright (C) 2016  Equinor ASA, Norway.

   The file 'ecl_version.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ECL_VERSION
#define ECL_VERSION

#include <stdbool.h>

#ifdef __cplusplus
extern"C" {
#endif

const char * ecl_version_get_git_commit();
const char * ecl_version_get_git_commit_short();
const char * ecl_version_get_build_time();
int    	     ecl_version_get_major_version();
int    	     ecl_version_get_minor_version();
const char * ecl_version_get_micro_version();
bool         ecl_version_is_ert_devel_version();

#ifdef __cplusplus
}
#endif

#endif
