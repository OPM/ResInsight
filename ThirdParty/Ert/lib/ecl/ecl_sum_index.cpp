/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_sum_index.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_sum_index.hpp>

/*
   This file contains all the internalized information from parsing a
   SMSPEC file. In most cases the ecl_sum object will contain a
   ecl_sum_index_type instance, and the end-user will not have direct
   interaction with the ecl_sum_index_type - but for instance when
   working with an ensemble of identical summary results one can use a
   shared ecl_sum_index instance.
*/

struct ecl_sum_index_type {
  hash_type        * well_var_index;             /* Indexes for all well variables. */
  hash_type        * well_completion_var_index;  /* Indexes for completion indexes .*/
  hash_type        * group_var_index;            /* Indexes for group variables.    */
  hash_type        * field_var_index;            /* Indexes for field variables.    */
  hash_type        * region_var_index;           /* The stored index is an offset.  */
  hash_type        * misc_var_index;             /* Indexes for misceallous variables - typically date. */
  hash_type        * block_var_index;            /* Indexes for block variables. */

  hash_type        * unit_hash;                  /* Units for the various measurements. */
};
