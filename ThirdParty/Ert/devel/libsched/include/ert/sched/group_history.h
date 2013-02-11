/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'group_history.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GROUP_HISTORY_H__
#define __GROUP_HISTORY_H__

#ifdef __cplusplus 
extern "C" {
#endif
#include <ert/util/time_t_vector.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

typedef struct group_history_struct group_history_type;


bool                 group_history_group_exists( const group_history_type * group_history , int report_step);
group_history_type * group_history_alloc( const char * group_name , const time_t_vector_type * time , int report_step );
void                 group_history_free( group_history_type * group_history ); 
void                 group_history_free__( void * arg );
void                 group_history_add_child(group_history_type * group_history , void * child_history , const char * child_name , int report_step );
void                 group_history_init_child_names( group_history_type * group_history , int report_step , stringlist_type * child_names );
const         char * group_history_get_name( const group_history_type * group_history );
void                 group_history_fprintf( const group_history_type * group_history , int report_step , bool recursive , FILE * stream );


double               group_history_iget_GOPRH( const void * __group_history , int report_step );
double               group_history_iget_GGPRH( const void * __group_history , int report_step );
double               group_history_iget_GWPRH( const void * __group_history , int report_step );
double               group_history_iget_GWCTH( const void * __group_history , int report_step );
double               group_history_iget_GGORH( const void * __group_history , int report_step );

double               group_history_iget_GWPTH( const void * __group_history , int report_step );
double               group_history_iget_GOPTH( const void * __group_history , int report_step );
double               group_history_iget_GGPTH( const void * __group_history , int report_step );

double               group_history_iget( const void * index , int report_step );

UTIL_IS_INSTANCE_HEADER( group_history );



#ifdef __cplusplus 
}
#endif
#endif
