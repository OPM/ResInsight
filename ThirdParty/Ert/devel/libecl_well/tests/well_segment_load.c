/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_segment_load.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/util/test_util.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_rsthead.h>
#include <ert/ecl/ecl_kw_magic.h>

#include <ert/ecl_well/well_segment.h>
#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_segment_collection.h>


int main(int argc , char ** argv) {
  const char * Xfile = argv[1];
  ecl_file_type * rst_file = ecl_file_open( Xfile , 0 );
  ecl_rsthead_type * rst_head = ecl_rsthead_alloc( rst_file );
  const ecl_kw_type * iwel_kw = ecl_file_iget_named_kw( rst_file , IWEL_KW , 0 );
  const ecl_kw_type * iseg_kw = ecl_file_iget_named_kw( rst_file , ISEG_KW , 0 );
  const ecl_kw_type * rseg_kw = ecl_file_iget_named_kw( rst_file , RSEG_KW , 0 );
  
  test_install_SIGNALS();
  test_assert_not_NULL( rst_file );
  test_assert_not_NULL( rst_head );
  
  {
    int well_nr;
    
    for (well_nr = 0; well_nr < rst_head->nwells; well_nr++) {
      int iwel_offset = rst_head->niwelz * well_nr;
      well_segment_collection_type * segments = well_segment_collection_alloc();
      int seg_well_nr = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_SEGMENTED_WELL_NR_ITEM) - 1; // -1: Ordinary well.
      if (seg_well_nr >= 0) {
        int segment_id;
        int segment_count = 0;
        
        for (segment_id = 0; segment_id < rst_head->nsegmx; segment_id++) {
          well_segment_type * segment = well_segment_alloc_from_kw( iseg_kw , rseg_kw , rst_head , seg_well_nr , segment_id );
          
          test_assert_true( well_segment_is_instance( segment ));
          
          if (well_segment_active( segment )) {
            well_segment_collection_add( segments , segment );
            test_assert_int_equal( well_segment_collection_get_size( segments ) , segment_count + 1);
            test_assert_ptr_equal( well_segment_collection_iget( segments , segment_count) , segment );
            segment_count++;
          } else
            well_segment_free( segment );
        }
      }  

      {
        well_segment_collection_type * segments2 = well_segment_collection_alloc();
        test_assert_int_equal( well_segment_collection_load_from_kw( segments2 , well_nr , iwel_kw , iseg_kw , rseg_kw , rst_head ) , 
                               well_segment_collection_get_size( segments));

        well_segment_collection_link( segments );
        well_segment_collection_link( segments2 );
        well_segment_collection_free( segments );
        well_segment_collection_free( segments2 );
      }
    }
  }
  ecl_file_close( rst_file );
  ecl_rsthead_free( rst_head );
  exit(0);
}
