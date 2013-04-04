/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lsb.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __LSB_H__
#define __LSB_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <lsf/lsbatch.h>

#include <ert/util/stringlist.h>

  typedef struct lsb_struct lsb_type;


  lsb_type * lsb_alloc();
  void       lsb_free( lsb_type * lsb);
  bool       lsb_ready( const lsb_type * lsb);

  int                 lsb_initialize( const lsb_type * lsb);
  int                 lsb_submitjob( const lsb_type * lsb ,  struct submit * , struct submitReply *);
  int                 lsb_killjob( const lsb_type * lsb , int lsf_jobnr);
  int                 lsb_openjob( const lsb_type * lsb , int lsf_jobnr);
  struct jobInfoEnt * lsb_readjob( const lsb_type * lsb );
  int                 lsb_closejob( const lsb_type * lsb );
  char *              lsb_sys_msg( const lsb_type * lsb );
  stringlist_type   * lsb_get_error_list( const lsb_type * lsb );



#ifdef __cplusplus
}
#endif

#endif
