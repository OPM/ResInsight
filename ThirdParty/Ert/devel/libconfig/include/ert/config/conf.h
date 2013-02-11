/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'conf.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONF_H__
#define __CONF_H__

/* libconfig: lightweight configuration parser
 *
 *
 *
 * Introduction
 *
 * This library provides a lightweight configuration parser for the
 * enkf application. The goal of the library is to provide the
 * developer with a tool for rapid specification of configuration
 * files, automatic checking of user provided configuration files
 * and typed access to configuration items.
 * 
 * 
 * 
 * A Simple Example
 *
 * Let us consider a simple example of user provided configuration
 * file that can be used with the parser:
 *
 *
 * res_sim FrontSim2007
 * {
 *   executable = /bin/frontsim2007; 
 *   version    = 2007;
 *   
 *   run_host bgo179lin
 *   {
 *     hostname = bgo179lin.nho.hydro.com;
 *     num_jobs = 4;
 *   };
 * };
 *
 *
 * Note that the newlines are not neccessary. In the example above,
 * the user has provided an instance of the class "res_sim" with name
 * FrontSim2007. Further, the user has set the items executable and version.
 * He has also provided a instance of the sub class "run_host" with name
 * bgo179lin and allocated 4 jobs to this machine.
 * 
 * 
 * 
 * Structure
 *
 * The system is built around four basic objects:
 *
 *  - Class definitions.
 *  - Item specifications.
 *  - Instances of classes.
 *  - Instances of item specifications, i.e. items.
 *
 * The relationship between the objects is as follows :
 *
 *  - Class:
 *    . Can have contain both classes and item specifications.
 *    . Can not contain items or class instances.
 *
 *  - Item specifications:
 *    . Can not contain any of the other objects.
 *
 *  - Instances of classes:
 *    . Can contain class instances and items.
 *
 *  - Items:
 *    . Can not contain any of the other objects.
 *
 *
 *
 * General Use
 *
 * The parser is designed to be used in the following way:
 *
 *  - The developer creates the classes and item specifications needed.
 *  - Using the library and the classes, user provided configuration
 *    files are read and validated.
 *  - If the validation fails, the developer can choose to exit.
 *  - Using the library, the devloper has typed access to all
 *    information provided by the user.
 *  
 */

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/set.h>
#include <ert/util/stringlist.h>

#include <ert/config/conf_data.h>



typedef struct conf_class_struct           conf_class_type;
typedef struct conf_instance_struct        conf_instance_type;
typedef struct conf_item_spec_struct       conf_item_spec_type;
typedef struct conf_item_struct            conf_item_type;
typedef struct conf_item_mutex_struct      conf_item_mutex_type;


/** D E F A U L T   A L L O C / F R E E    F U N C T I O N S */



conf_class_type * conf_class_alloc_empty(
  const char * class_name,
  bool         require_instance,
  bool         singleton,
  const char * help);

void conf_class_free(
  conf_class_type * conf_class);

void conf_class_free__(
  void * conf_class);



conf_instance_type * conf_instance_alloc_default(
  const conf_class_type * conf_class,
  const char            * name);

conf_instance_type * conf_instance_copyc(
  const conf_instance_type * conf_instance);

void conf_instance_free(
  conf_instance_type * conf_instance);

void conf_instance_free__(
  void * conf_instance);



conf_item_spec_type * conf_item_spec_alloc(
  char    * name,
  bool      required_set,
  dt_enum   dt,
  const char * help);

void conf_item_spec_free(
  conf_item_spec_type * conf_item_spec);

void conf_item_spec_free__(
  void * conf_item_spec);



conf_item_type * conf_item_alloc(
  const conf_item_spec_type * conf_item_spec,
  const char                * value);

conf_item_type * conf_item_copyc(
  const conf_item_type * conf_item);

void conf_item_free(
  conf_item_type * conf_item);

void conf_item_free__(
  void * conf_item);



void conf_item_mutex_free(
  conf_item_mutex_type * conf_item_mutex);

void conf_item_mutex_free__(
  void * conf_item_mutex);



/** M A N I P U L A T O R S ,   I N S E R T I O N */ 



void conf_class_insert_owned_sub_class(
  conf_class_type * conf_class,
  conf_class_type * sub_conf_class);

void conf_class_insert_owned_item_spec(
  conf_class_type     * conf_class,
  conf_item_spec_type * item_spec);

void conf_instance_insert_owned_sub_instance(
  conf_instance_type * conf_instance,
  conf_instance_type * sub_conf_instance);

void conf_instance_insert_owned_item(
  conf_instance_type * conf_instance,
  conf_item_type     * conf_item);

void conf_instance_insert_item(
  conf_instance_type * conf_instance,
  const char         * item_name,
  const char         * value);

void conf_instance_overload(
  conf_instance_type       * conf_instance_target,
  const conf_instance_type * conf_instance_source);

conf_item_mutex_type * conf_class_new_item_mutex(
  conf_class_type  * conf_class,
  bool               require_one,
  bool inverse);

void conf_item_mutex_add_item_spec(
  conf_item_mutex_type       * conf_item_mutex,
  const conf_item_spec_type  * conf_item_spec);



/** M A N I P U L A T O R S ,   C L A S S   A N D   I T E M   S P E C I F I C A T I O N */ 



void conf_class_set_help(
  conf_class_type * conf_class,
  const char      * help);



void conf_item_spec_add_restriction(
  conf_item_spec_type * conf_item_spec,
  const char          * restriction);

void conf_item_spec_set_default_value(
  conf_item_spec_type * conf_item_spec,
  const char          * default_value);

void conf_item_spec_set_help(
  conf_item_spec_type * conf_item_spec,
  const char          * help);



/** A C C E S S O R S */



bool conf_class_has_item_spec(
  const conf_class_type * conf_class,
  const char            * item_name);

bool conf_class_has_sub_class(
  const conf_class_type * conf_class,
  const char            * sub_class_name);

const conf_item_spec_type * conf_class_get_item_spec_ref(
  const conf_class_type * conf_class,
  const char            * item_name);

const conf_class_type * conf_class_get_sub_class_ref(
  const conf_class_type * conf_class,
  const char            * sub_class_name);



const char * conf_instance_get_name_ref(
  const conf_instance_type * conf_instance);

bool conf_instance_is_of_class(
  const conf_instance_type * conf_instance,
  const char               * class_name);

bool conf_instance_has_item(
  const conf_instance_type * conf_instance,
  const char               * item_name);

bool conf_instance_has_sub_instance(
  const conf_instance_type * conf_instance,
  const char               * sub_instance_name);

const conf_instance_type * conf_instance_get_sub_instance_ref(
  const conf_instance_type * conf_instance,
  const char               * sub_instance_name);

stringlist_type * conf_instance_alloc_list_of_sub_instances_of_class(
  const conf_instance_type * conf_instance,
  const conf_class_type    * conf_class);

stringlist_type * conf_instance_alloc_list_of_sub_instances_of_class_by_name(
  const conf_instance_type * conf_instance,
  const char               * sub_class_name);

const conf_class_type * conf_instance_get_class_ref(
  const conf_instance_type * conf_instance);

const char * conf_instance_get_class_name_ref(
  const conf_instance_type * conf_instance);

const char * conf_instance_get_item_value_ref(
  const conf_instance_type * conf_instance,
  const char               * item_name);

/** If the dt supports it, these functions will parse the item
    value to the requested types.

    NOTE:
    If the dt does not support it, or the conf_instance
    does not have the item, the functions will abort your program.
*/
int conf_instance_get_item_value_int(
  const conf_instance_type * conf_instance,
  const char               * item_name);

double conf_instance_get_item_value_double(
  const conf_instance_type * conf_instance,
  const char               * item_name);

time_t conf_instance_get_item_value_time_t(
  const conf_instance_type * conf_instance,
  const char               * item_name);

/** V A L I D A T O R S */



bool conf_instance_validate(
  const conf_instance_type * conf_instance);



/** A L L O C   F R O M   F I L E */


conf_instance_type * conf_instance_alloc_from_file(
  const conf_class_type * conf_class,
  const char            * name,
  const char            * file_name);

#ifdef __cplusplus 
}
#endif
#endif  
