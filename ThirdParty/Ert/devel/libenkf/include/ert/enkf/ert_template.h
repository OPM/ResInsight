/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ert_template.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ERT_TEMPLATE_H__
#define __ERT_TEMPLATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/subst_list.h>
#include <ert/util/stringlist.h>

#include <ert/config/config.h>

typedef struct ert_template_struct  ert_template_type;
typedef struct ert_templates_struct ert_templates_type;


stringlist_type   * ert_templates_alloc_list( ert_templates_type * ert_templates);
ert_template_type * ert_template_alloc( const char * template_file , const char * target_file, subst_list_type * parent_subst) ;
void                ert_template_free( ert_template_type * ert_tamplete );
void                ert_template_instantiate( ert_template_type * ert_template , const char * path , const subst_list_type * arg_list ); 
void                ert_template_add_arg( ert_template_type * ert_template , const char * key , const char * value );
void                ert_template_free__(void * arg);

void                ert_templates_clear( ert_templates_type * ert_templates );
ert_template_type * ert_templates_get_template( ert_templates_type * ert_templates , const char * key);

ert_templates_type * ert_templates_alloc(subst_list_type * parent_subst);
void                 ert_templates_free( ert_templates_type * ert_templates );
ert_template_type  * ert_templates_add_template( ert_templates_type * ert_templates , const char * key , const char * template_file , const char * target_file , const char * arg_string);
void                 ert_templates_instansiate( ert_templates_type * ert_templates , const char * path , const subst_list_type * arg_list);
void                 ert_templates_del_template( ert_templates_type * ert_templates , const char * key);

const char         * ert_template_get_template_file( const ert_template_type * ert_template);
const char         * ert_template_get_target_file( const ert_template_type * ert_template);
const char         * ert_template_get_args_as_string( const ert_template_type * ert_template );
void                 ert_templates_fprintf_config( const ert_templates_type * ert_templates , FILE * stream );
void                 ert_templates_init( ert_templates_type * templates , const config_type * config );

#ifdef __cplusplus
}
#endif
#endif
