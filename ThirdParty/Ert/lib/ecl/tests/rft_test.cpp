/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'rft_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ecl_rft_file.h>
#include <ecl_rft_node.h>
#include <stringlist.h>
#include <util.h>


int main (int argc , char ** argv) {
  if (argc != 2)
    util_exit("I want one RFT file - try again \n");
  {
    const char * filename = argv[1];
    ecl_rft_file_type * rft_file = ecl_rft_file_alloc( filename );
    stringlist_type   * wells    = ecl_rft_file_alloc_well_list( rft_file );

    printf("<ECLIPSE RFT FILE>\n");
    {
      int iw;
      for (iw = 0; iw < stringlist_get_size( wells ); iw++) {
	const char * well = stringlist_iget(wells , iw);
	printf("    <WELL>\n");
	{
	  int it;
	  for (it = 0; it < ecl_rft_file_get_well_occurences( rft_file , well ); it++) {
	    const ecl_rft_node_type * node = ecl_rft_file_iget_well_rft( rft_file , well , it);
	    time_t date  = ecl_rft_node_get_date( node );
	    {
	      int mday, year,month;
	      util_set_date_values( date , &mday , &month , &year);
	      printf("        <RFT>\n");
	      printf("            <DATE>%02d/%02d/%4d</DATE> \n",mday,month,year);
	      {
		int num_cells = ecl_rft_node_get_size( node );
		int icell;
		for (icell = 0; icell < num_cells; icell++) {
		  int i,j,k;
		  ecl_rft_node_iget_ijk( node , icell , &i , &j , &k);
		  printf("            <cell>\n");
		  printf("                <PRESSURE> %g </PRESSURE> \n", ecl_rft_node_iget_pressure( node, icell));
		  printf("                <DPETH>    %g </DEPTH>     \n" , ecl_rft_node_iget_depth( node , icell));
		  printf("                <ijk> %3d,%3d,%3d </ijk>  \n",i,j,k);
		  printf("            </cell>\n");
		}
	      }
	      printf("        </RFT>\n");
	    }
	  }
	}
	printf("    </WELL>\n");
      }
    }
    printf("</ECLIPSE RFT FILE>\n");
    ecl_rft_file_free( rft_file );
  }
}
