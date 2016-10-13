/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_rstfile.c' is part of ERT - Ensemble based Reservoir Tool.

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

/*****************************************************************/
/*                   S U M M A R Y   F I L E S                   */
/*****************************************************************/


bool ecl_file_select_smryblock( ecl_file_type * ecl_file , int report_step ) {
  return ecl_file_select_block( ecl_file , SEQHDR_KW , report_step );
}


ecl_file_type * ecl_file_open_smryblock( const char * filename , int report_step , int flags) {
  ecl_file_type * ecl_file = ecl_file_open( filename , flags );
  if (ecl_file) {
    if (!ecl_file_select_smryblock( ecl_file , report_step )) {
      ecl_file_close( ecl_file );
      ecl_file = NULL;
    }
  }
  return ecl_file;
}



