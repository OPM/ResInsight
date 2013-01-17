/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'combine_grdecl.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <ecl_kw.h>
#include <ecl_util.h>
#include <stdio.h>
#include <stdbool.h>
#include <util.h>
#include <ecl_box.h>


int main(int argc, char ** argv) {

  //int nx,ny,nz;
  //char filename[128];
  //ecl_type_enum ecl_type = ecl_float_type;
  //bool endian_flip = true;
  //
  //ecl_kw_type  * main_kw;
  //ecl_box_type * ecl_box;
  //
  //printf("Main filename => "); 
  //scanf("%s" , filename); 
  //printf("Total size of grid (nx ny nz) => ");
  //scanf("%d %d %d" , &nx , &ny , &nz);
  //
  //{
  //  FILE * stream = util_fopen(filename , "r");
  //  main_kw = ecl_kw_fscanf_alloc_grdecl_data(stream , nx*ny*nz , ecl_type , endian_flip);
  //  fclose(stream);
  //}
  //ecl_box = ecl_box_alloc(nx,ny,nz,  0, 0 , 0 , 0,0,0);
  //
  //{
  //  int scan_count;
  //  int x1,x2,y1,y2,z1,z2;
  //  do {
  //    printf("box: file x1 x2 y1 y2 z1 z2 (^d to exit): ");
  //    scan_count = scanf("%s %d %d %d %d %d %d" , filename , &x1 , &x2 , &y1 , &y2 , &z1 , &z2);
  //    if (scan_count == 7) {
  //      ecl_box_set_size(ecl_box, x1,x2 , y1 , y2 , z1 , z2);
  //      {
  //        FILE * stream = util_fopen(filename , "r");
  //        ecl_kw_type * sub_kw = ecl_kw_fscanf_alloc_grdecl_data(stream , ecl_box_get_box_size(ecl_box) , ecl_type , endian_flip);
  //        ecl_kw_merge(main_kw , sub_kw , ecl_box);
  //        ecl_kw_free(sub_kw);
  //        fclose(stream);
  //      }
  //    } else if (scan_count == 2) {
  //      int format = x1;
  //
  //      if (format == 1) {
  //        FILE * stream = util_fopen(filename , "w");
  //        ecl_kw_fprintf_grdecl(main_kw , stream);
  //        printf("New kw saved to: %s \n",filename);
  //        fclose(stream);
  //      } else {
  //        fortio_type * fortio = fortio_open(filename , "w" , endian_flip);
  //        ecl_kw_set_fmt_file(main_kw , false);
  //        ecl_kw_fwrite(main_kw , fortio);
  //        printf("New kw saved to: %s \n",filename);
  //        fortio_close(fortio);
  //      }
  //      
  //    }
  //  } while (scan_count != EOF);
  //}
  //ecl_box_free(ecl_box);
  //ecl_kw_free(main_kw);

  exit(1);
}
