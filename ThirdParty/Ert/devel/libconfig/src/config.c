/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <stdio.h>

#include <util.h>
#include <parser.h>
#include <hash.h>
#include <stringlist.h>
#include <set.h>
#include <subst_list.h>
#include <vector.h>

#include <config.h>

#define CLEAR_STRING "__RESET__"






/**
Structure to parse configuration files of this type:

KEYWORD1  ARG2   ARG2  ARG3
KEYWORD2  ARG1-2
....
KEYWORDN  ARG1  ARG2

A keyword can occure many times.

*/



/**

                                                              
                           =============================                                
                           | config_type object        |
                           |                           |                                
                           | Contains 'all' the        |                                
                           | configuration information.|                                
                           |                           |                                
                           =============================                                
                               |                   |
                               |                   \________________________                                             
                               |                                            \          
                              KEY1                                         KEY2   
                               |                                             |  
                              \|/                                           \|/ 
                   =========================                      =========================          
                   | config_item object    |                      | config_item object    |                     
                   |                       |                      |                       |                     
                   | Indexed by a keyword  |                      | Indexed by a keyword  |                     
                   | which is the first    |                      | which is the first    |                     
                   | string in the         |                      | string in the         |                     
                   | config file.          |                      | config file.          |                     
                   |                       |                      |                       |                                                                           
                   =========================                      =========================                     
                       |             |                                        |
                       |             |                                        |
                      \|/           \|/                                      \|/  
============================  ============================   ============================
| config_item_node object  |  | config_item_node object  |   | config_item_node object  |
|                          |  |                          |   |                          |
| Only containing the      |  | Only containing the      |   | Only containing the      |
| stringlist object        |  | stringlist object        |   | stringlist object        |
| directly parsed from the |  | directly parsed from the |   | directly parsed from the |
| file.                    |  | file.                    |   | file.                    |
|--------------------------|  |--------------------------|   |--------------------------|
| ARG1 ARG2 ARG3           |  | VERBOSE                  |   | DEBUG                    |
============================  ============================   ============================


The example illustrated above would correspond to the following config
file (invariant under line-permutations):

KEY1   ARG1 ARG2 ARG3
KEY1   VERBOSE
KEY2   DEBUG


Example config file(2):

OUTFILE   filename
INPUT     filename
OPTIONS   store
OPTIONS   verbose
OPTIONS   optimize cache=1

In this case the whole config object will contain three items,
corresponding to the keywords OUTFILE, INPUT and OPTIONS. The two
first will again only contain one node each, whereas the OPTIONS item
will contain three nodes, corresponding to the three times the keyword
"OPTIONS" appear in the config file. Observe that *IF* the OPTIONS
item had been added with append_arg == false, only the last occurence,
corresponding to 'optimize cache=1' would be present.

*/

typedef struct validate_struct validate_type;

/** 
   This is a 'support-struct' holding various pieces of information
   needed during the validation process. Observe the following about
   validation:

    1. It is atomic, in the sense that if you try to an item like this:

         KW  ARG1 ARG2 ARG3

       where ARG1 and ARG2 are valid, whereas there is something wrong
       with ARG3, NOTHING IS SET.

    2. Validation is a two-step process, the first step is run when an
       item is parsed. This includes checking:

        o The number of argument.  
        o That the arguments have the right type.
        o That the values match the selection set.
      
       The second validation step is done when the pasing is complete,
       in this pass we check dependencies - i.e. required_chldren and
       required_children_on_value.


   Observe that nothing has-to be set in this struct. There are some dependencies:

    1. Only _one_ of common_selection_set and indexed_selection_set
       can be set.

    2. If setting indexed_selection_set or type_map, you MUST set
       argc_max first.
*/
    
struct validate_struct {
  int                  argc_min;                /* The minimum number of arguments: -1 means no lower limit. */
  int                  argc_max;                /* The maximum number of arguments: -1 means no upper limit. */ 
  set_type          *  common_selection_set;    /* A selection set which will apply uniformly to all the arguments. */ 
  set_type          ** indexed_selection_set;   /* A selection set which will apply for specifi (indexed) arguments. */
  config_item_types *  type_map;                /* A list of types for the items - can be NULL. Set along with argc_minmax(); */
  int                  type_map_size;
  stringlist_type   *  required_children;       /* A list of item's which must also be set (if this item is set). (can be NULL) */
  hash_type         *  required_children_value; /* A list of item's which must also be set - depending on the value of this item. (can be NULL) (This one is complex). */
};


struct config_struct {
  hash_type            * items;                     /* A hash of config_items - the actual content. */
  stringlist_type      * parse_errors;              /* A stringlist containg the errors found when parsing.*/
  set_type             * parsed_files;              /* A set of config files whcih have been parsed - to protect against circular includes. */
  hash_type            * messages;                  /* Can print a (warning) message when a keyword is encountered. */
  subst_list_type      * define_list;
  hash_type            * auto_items;                /* This is a hash of vectors of stringlist instances of items which have been auto-add'ed while parsing. */
  char                 * config_file;               /* The last parsed file - NULL if no file is parsed-. */
  char                 * abs_path;
};



#define CONFIG_ITEM_ID 6751
struct config_item_struct {
  int                         __id;                      /* Used for run-time checking */
  char                        * kw;                      /* The kw which identifies this item· */
  
  int                           alloc_size;              /* The number of nodes which have been allocated. */  
  int                           node_size;               /* The number of active nodes.*/
  config_item_node_type      ** nodes;                   /* A vector of config_item_node_type instances. */

  bool                          append_arg;              /* Should the values be appended if a keyword appears several times in the config file. */
  bool                          currently_set;           /* Has a value been assigned to this keyword. */
  bool                          required_set;            
  stringlist_type             * required_children;       /* A list of item's which must also be set (if this item is set). (can be NULL) */
  hash_type                   * required_children_value; /* A list of item's which must also be set - depending on the value of this item. (can be NULL) */
  validate_type               * validate;                /* Information need during validation. */ 
  bool                          expand_envvar;           /* Should environment variables like $HOME be expanded?*/ 
};
 

struct config_item_node_struct {
  stringlist_type             * stringlist;              /* The values which have been set. */
  char                        * config_cwd;              /* The currently active cwd (relative or absolute) of the parser when this item is set. */
};


/*****************************************************************/

static void config_add_and_free_error(config_type * config , char * error_message);

/*****************************************************************/
static validate_type * validate_alloc() {
  validate_type * validate = util_malloc(sizeof * validate );
  validate->argc_min = -1;
  validate->argc_max = -1;
  validate->common_selection_set    = NULL;
  validate->indexed_selection_set   = NULL;
  validate->type_map                = NULL;
  validate->type_map_size           = 0;
  validate->required_children       = NULL;
  validate->required_children_value = NULL;

  return validate;
}


static void validate_free(validate_type * validate) {
  if (validate->common_selection_set != NULL) set_free(validate->common_selection_set);
  if (validate->indexed_selection_set != NULL) {
    for (int i = 0; i < validate->argc_max; i++)
      if (validate->indexed_selection_set[i] != NULL)
        set_free(validate->indexed_selection_set[i]);
    free(validate->indexed_selection_set);
  }
  if (validate->type_map != NULL) free(validate->type_map);
  if (validate->required_children != NULL) stringlist_free(validate->required_children);
  if (validate->required_children_value != NULL) hash_free(validate->required_children_value);
  free(validate);
}

static void validate_set_argc_minmax(validate_type * validate , int argc_min , int argc_max, int type_map_size , const config_item_types * type_map) {
  if (validate->argc_min != -1)
    util_abort("%s: sorry - current implementation does not allow repeated calls to: %s \n",__func__ , __func__);
  
  if (argc_min == -1)
    argc_min = 0;

  validate->argc_min = argc_min;
  validate->argc_max = argc_max;
  
  {
    int internal_type_size = 0;  /* Should end up in the range [argc_min,argc_max] */

    if (argc_max > 0) 
      internal_type_size = argc_max;
    else
      internal_type_size = argc_min;

    if (internal_type_size > 0) {
      validate->indexed_selection_set = util_calloc( internal_type_size , sizeof * validate->indexed_selection_set );
      for (int iarg=0; iarg < internal_type_size; iarg++)
        validate->indexed_selection_set[iarg] = NULL;
    }
  }
  
  if (type_map != NULL) {
    validate->type_map_size = type_map_size;
    validate->type_map = util_alloc_copy(type_map , type_map_size * sizeof * type_map );
  }

}



static void validate_set_common_selection_set(validate_type * validate , int argc , const char ** argv) {
  if (validate->common_selection_set != NULL)
    set_free(validate->common_selection_set);
  validate->common_selection_set = set_alloc( argc , argv );
}


static void validate_set_indexed_selection_set(validate_type * validate , int index , int argc , const char ** argv) {
  
  if (validate->indexed_selection_set == NULL)
    util_abort("%s: must call xxx_set_argc_minmax() first - aborting \n",__func__);
  
  if (index >= validate->argc_min)
    util_abort("%s: When not not setting argc_max selection set can only be applied to indices up to argc_min\n",__func__);
  
  if (validate->indexed_selection_set[index] != NULL)
    set_free(validate->indexed_selection_set[index]);
  
  validate->indexed_selection_set[index] = set_alloc(argc , argv);
}




static char * __alloc_relocated__(const char * config_cwd , const char * value) {
  char * file;
  
  if (util_is_abs_path(value))
    file = util_alloc_string_copy( value );
  else
    file = util_alloc_filename(config_cwd , value , NULL);

  return file;
}





/*****************************************************************/
static void config_item_node_fprintf(config_item_node_type * node , int node_nr , FILE * stream) {
  fprintf(stream , "   %02d: ",node_nr);
  stringlist_fprintf(node->stringlist , " " , stream);
  fprintf(stream,"     config_CWD: %s \n",node->config_cwd);
}


static config_item_node_type * config_item_node_alloc() {
  config_item_node_type * node = util_malloc(sizeof * node );
  node->stringlist = stringlist_alloc_new();
  node->config_cwd = NULL;
  return node;
}


static void config_item_node_set(config_item_node_type * node , const stringlist_type * token_list) {
  int argc = stringlist_get_size( token_list ) - 1;
  for (int iarg=0; iarg < argc; iarg++) 
    stringlist_append_copy( node->stringlist , stringlist_iget( token_list , iarg + 1));
}



static char * config_item_node_alloc_joined_string(const config_item_node_type * node, const char * sep) {
  return stringlist_alloc_joined_string(node->stringlist , sep);
}


static void config_item_node_free(config_item_node_type * node) {
  stringlist_free(node->stringlist);
  util_safe_free(node->config_cwd);
  free(node);
}






static void config_item_realloc_nodes(config_item_type * item , int new_size) {
  const int old_size = item->alloc_size;
  item->nodes      = util_realloc(item->nodes , sizeof * item->nodes * new_size );
  item->alloc_size = new_size;
  {
    int i;
    for (i=old_size; i < new_size; i++)
      item->nodes[i] = NULL;
  }
}



static void config_item_node_clear(config_item_node_type * node) {
  stringlist_clear( node->stringlist );
  util_safe_free(node->config_cwd);
  node->config_cwd = NULL;
}


static config_item_node_type * config_item_iget_node(const config_item_type * item , int index) {
  if (index < 0 || index >= item->node_size)
    util_abort("%s: error - asked for node nr:%d available: [0,%d> \n",__func__ , index , item->node_size);
  return item->nodes[index];
}


/** 
    Adds a new node as side-effect ... 
*/
static config_item_node_type * config_item_get_new_node(config_item_type * item) {
  if (item->node_size == item->alloc_size)
    config_item_realloc_nodes(item , item->alloc_size * 2);
  {
    config_item_node_type * new_node = config_item_node_alloc();
    item->nodes[item->node_size] = new_node;
    item->node_size++;
    return new_node;
  }
}


static config_item_node_type * config_item_get_first_node(config_item_type * item) {

  if (item->node_size == 0)
    config_item_get_new_node(item); 
  
  return config_item_iget_node(item , 0);
}


/*
  This function will fail item has not been allocated 
  append_arg == false.
*/

const char * config_item_iget(const config_item_type * item , int occurence , int index) {
  config_item_node_type * node = config_item_iget_node(item , occurence);  
  return stringlist_iget( node->stringlist , index );
}


void config_item_assure_type(const config_item_type * item , int index , config_item_types item_type) {
  if (index < item->validate->type_map_size) {
    bool OK = false;
    
    if (item->validate->type_map != NULL)
      if (item->validate->type_map[index] == item_type)
        OK = true;
    
    if (!OK)
      util_abort("%s: failed - wrong installed type \n" , __func__);
  }
}



bool config_item_iget_as_bool(const config_item_type * item, int occurence , int index) {
  bool value;
  util_sscanf_bool( config_item_iget(item , occurence ,index) , &value );
  return value;
}


static int config_item_iget_as_int(const config_item_type * item, int occurence , int index) {
  int value;
  config_item_assure_type(item , index , CONFIG_INT);
  util_sscanf_int( config_item_iget(item , occurence , index) , &value );
  return value;
}


double config_item_iget_as_double(const config_item_type * item, int occurence , int index) {
  double value;
  config_item_assure_type(item , index , CONFIG_FLOAT);
  util_sscanf_double( config_item_iget(item , occurence , index) , &value );
  return value;
}


static stringlist_type * config_item_iget_stringlist_ref(const config_item_type * item, int occurence) {
  config_item_node_type * node = config_item_iget_node(item , occurence);  
  return node->stringlist;
}


static char * config_item_ialloc_joined_string(const config_item_type * item , const char * sep , int occurence) {
  config_item_node_type * node = config_item_iget_node(item , occurence);  
  return config_item_node_alloc_joined_string(node , sep);
}

/**
   This function counts the number of times a config item has been
   set. Referring again to the example at the top:

     config_item_get_occurences( "KEY1" )

   will return 2. However, if the item has been added with append_arg
   set to false, this function can only return zero or one.
*/


static int config_item_get_occurences(const config_item_type * item) {
  return item->node_size;
}


static char * config_item_alloc_joined_string(const config_item_type * item , const char * sep) {
  const int occurences = config_item_get_occurences( item );
  char * joined_string = NULL;
  
  for (int i =0; i < occurences ; i++) {
    joined_string = util_strcat_realloc( joined_string , config_item_ialloc_joined_string(item , sep , i));
    if (i < (occurences - 1))
      joined_string = util_strcat_realloc( joined_string , sep );
  }
  
  return joined_string;
}


static const stringlist_type * config_item_get_stringlist_ref(const config_item_type * item) {
  if (item->append_arg) 
    util_abort("%s: this function can only be used on items added with append_arg == FALSE\n" , __func__);
  return config_item_iget_stringlist_ref(item , 0);
}


/**
   If copy == false - the stringlist will break down when/if the
   config object is freed - your call.
*/
   

static stringlist_type * config_item_alloc_complete_stringlist(const config_item_type * item, bool copy) {
  int inode;
  stringlist_type * stringlist = stringlist_alloc_new();
  for (inode = 0; inode < item->node_size; inode++) {

    if (copy)
      stringlist_append_stringlist_copy( stringlist , item->nodes[inode]->stringlist );
    else
      stringlist_append_stringlist_ref( stringlist , item->nodes[inode]->stringlist );  
    
  }

  return stringlist;
}



/**
   If copy == false - the stringlist will break down when/if the
   config object is freed - your call.
*/

static stringlist_type * config_item_alloc_stringlist(const config_item_type * item, bool copy) {
  if (item->append_arg) {
    util_abort("%s: item:%s must be initialized with append_arg == false for this call. \n",__func__);
    return NULL;
  } else {
    stringlist_type * stringlist = stringlist_alloc_new();

    if (copy)
      stringlist_append_stringlist_copy( stringlist , item->nodes[0]->stringlist );
    else
      stringlist_append_stringlist_ref( stringlist , item->nodes[0]->stringlist );  
    
    return stringlist;
  }
}



/**
   If copy == false - the hash will break down when/if the
   config object is freed - your call.
*/

static hash_type * config_item_alloc_hash(const config_item_type * item , bool copy) {
  hash_type * hash = hash_alloc();
  int inode;
  for (inode = 0; inode < item->node_size; inode++) {
    const config_item_node_type * node = item->nodes[inode];

    if (copy) {
      hash_insert_hash_owned_ref(hash , 
                                 stringlist_iget(node->stringlist , 0) , 
                                 util_alloc_string_copy(stringlist_iget(node->stringlist , 1)) , 
                                 free);
    } else
      hash_insert_ref(hash , stringlist_iget(node->stringlist , 0) , stringlist_iget(node->stringlist , 1));
    
  }
  return hash;
}


config_item_type * config_item_alloc(const char * kw , bool required , bool append_arg) {
  config_item_type * item = util_malloc(sizeof * item );

  item->__id       = CONFIG_ITEM_ID;
  item->kw         = util_alloc_string_copy(kw);
  item->alloc_size = 0;
  item->node_size  = 0;
  item->nodes      = NULL;
  config_item_realloc_nodes(item , 1);

  item->currently_set           = false;
  item->append_arg              = append_arg;
  item->required_set            = required;
  item->required_children       = NULL;
  item->required_children_value = NULL;
  item->expand_envvar           = true;  /* Default is to expand $VAR expressions; can be turned off with
                                            config_item_set_envvar_expansion( item , false ); */
  item->validate                = validate_alloc();
  return item;
}



/* 
   Observe that this function is a bit funny - because it will
   actually free the incoming message.
*/

static void config_add_and_free_error(config_type * config , char * error_message) {
  if (error_message != NULL) {
    int error_nr = stringlist_get_size(config->parse_errors) + 1;
    stringlist_append_owned_ref(config->parse_errors , util_alloc_sprintf("  %02d: %s" , error_nr , error_message));
    free(error_message);
  }
}

/**
   Used to reset an item is the special string 'CLEAR_STRING'
   is found as the only argument:

   OPTION V1
   OPTION V2 V3 V4
   OPTION __RESET__ 
   OPTION V6

   In this case OPTION will get the value 'V6'. The example given
   above is a bit contrived; this option is designed for situations
   where several config files are parsed serially; and the user can
   not/will not update the first.
*/

static void config_item_clear( config_item_type * item ) {
  int i;
  for (i = 0; i < item->node_size; i++)
    config_item_node_free( item->nodes[i] );
  util_safe_free(item->nodes);
  item->nodes = NULL;
  item->node_size     = 0;
  item->currently_set = false;
  config_item_realloc_nodes(item , 1);
}



/**
   This function validates the setting of an item. If the item is OK NULL is returned,
   otherwise an error message is returned.
*/

static bool config_item_validate_set(config_type * config , const config_item_type * item , stringlist_type * token_list , const char * config_file, const char * config_cwd) {
  bool OK = true;
  int argc = stringlist_get_size( token_list ) - 1;
  if (item->validate->argc_min >= 0) {
    if (argc < item->validate->argc_min) {
      OK = false;
      if (config_file != NULL)
        config_add_and_free_error(config , util_alloc_sprintf("Error when parsing config_file:\"%s\" Keyword:%s must have at least %d arguments.",config_file , item->kw , item->validate->argc_min));
      else
        config_add_and_free_error(config , util_alloc_sprintf("Error:: Keyword:%s must have at least %d arguments.",item->kw , item->validate->argc_min));
    }
  }

  if (item->validate->argc_max >= 0) {
    if (argc > item->validate->argc_max) {
      OK = false;
      if (config_file != NULL)
        config_add_and_free_error(config , util_alloc_sprintf("Error when parsing config_file:\"%s\" Keyword:%s must have maximum %d arguments.",config_file , item->kw , item->validate->argc_max));
      else
        config_add_and_free_error(config , util_alloc_sprintf("Error:: Keyword:%s must have maximum %d arguments.",item->kw , item->validate->argc_max));
    }
  }

  /* 
     OK - now we have verified that the number of arguments is correct. Then
     we start actually looking at the values. 
  */

  if (OK) { 
    /* Validating selection set - first common, then indexed */
    if (item->validate->common_selection_set) {
      for (int iarg = 0; iarg < argc; iarg++) {
        if (!set_has_key(item->validate->common_selection_set , stringlist_iget( token_list , iarg + 1))) {
          config_add_and_free_error(config , util_alloc_sprintf("%s: is not a valid value for: %s.",stringlist_iget( token_list , iarg + 1) , item->kw));
          OK = false;
        }
      }
    } else if (item->validate->indexed_selection_set != NULL) {
      for (int iarg = 0; iarg < argc; iarg++) {
        if ((item->validate->argc_max > 0) || (iarg < item->validate->argc_min)) {  /* Without this test we might go out of range on the indexed selection set. */
          if (item->validate->indexed_selection_set[iarg] != NULL) {
            if (!set_has_key(item->validate->indexed_selection_set[iarg] , stringlist_iget( token_list , iarg + 1))) {
              config_add_and_free_error(config , util_alloc_sprintf("%s: is not a valid value for item %d of \'%s\'.",stringlist_iget( token_list , iarg + 1) , iarg + 1 , item->kw));
              OK = false;
            }
          }
        }
      }
    }

    /*
      Observe that the following code might rewrite the content of
      argv for arguments referring to path locations.
    */


    /* Validate the TYPE of the various argumnents */
    if (item->validate->type_map != NULL) {
      for (int iarg = 0; iarg < argc; iarg++) {
        if (iarg < item->validate->type_map_size) {
          const char * value = stringlist_iget(token_list , iarg + 1);
          switch (item->validate->type_map[iarg]) {
          case(CONFIG_STRING): /* This never fails ... */
            break;
          case(CONFIG_EXECUTABLE):
            {
              /*
                1. If the supplied value is an abolute path - do nothing.
                2. If the supplied is _not_ an absolute path:
                
                a. Try if the relocated exists - then use that.
                b. Else - try if the util_alloc_PATH_executable() exists.
              */
              if (!util_is_abs_path( value )) {
                char * relocated  = __alloc_relocated__(config_cwd , value);
                char * path_exe   = util_alloc_PATH_executable( value );
                
                if (util_file_exists(relocated)) {
                  if (util_is_executable(relocated))
                    stringlist_iset_copy( token_list , iarg , relocated);
                } else if (path_exe != NULL)
                  stringlist_iset_copy( token_list , iarg , path_exe);
                else
                  config_add_and_free_error(config , util_alloc_sprintf("Could not locate executable:%s ", value));

                free(relocated);
                util_safe_free(path_exe);
              } else {
                if (!util_is_executable( value ))
                  config_add_and_free_error(config , util_alloc_sprintf("Could not locate executable:%s ", value));
              }
            }
            break;
          case(CONFIG_INT):
            if (!util_sscanf_int( value , NULL ))
              config_add_and_free_error(config , util_alloc_sprintf("Failed to parse:%s as an integer.",value));
            break;
          case(CONFIG_FLOAT):
            if (!util_sscanf_double( value , NULL ))
              config_add_and_free_error(config , util_alloc_sprintf("Failed to parse:%s as a floating point number.", value));
            break;
          case(CONFIG_EXISTING_FILE):
            {
              char * file = __alloc_relocated__(config_cwd , value);
              if (!util_file_exists(file)) 
                config_add_and_free_error(config , util_alloc_sprintf("Can not find file %s in %s ",value , config_cwd));
              else
                stringlist_iset_copy( token_list , iarg + 1 , file);  
              free( file );
            }
            break;
          case(CONFIG_FILE):
            {
              char * file = __alloc_relocated__(config_cwd , value);
              stringlist_iset_copy( token_list , iarg + 1 , file);  
              free( file );
            }
            break;
          case(CONFIG_EXISTING_DIR):
            {
              char * dir = __alloc_relocated__(config_cwd , value);
              if (!util_is_directory(value))
                config_add_and_free_error(config , util_alloc_sprintf("Can not find directory: %s. ",value));
              else
                stringlist_iset_copy( token_list , iarg + 1 , dir);  
              free( dir );
            }
            break;
          case(CONFIG_BOOLEAN):
            if (!util_sscanf_bool( value , NULL ))
              config_add_and_free_error(config , util_alloc_sprintf("Failed to parse:%s as a boolean.", value));
            break;
          case(CONFIG_BYTESIZE):
            if (!util_sscanf_bytesize( value , NULL))
              config_add_and_free_error(config , util_alloc_sprintf("Failed to parse:\"%s\" as number of bytes." , value));
            break;
          default:
            util_abort("%s: config_item_type:%d not recognized \n",__func__ , item->validate->type_map[iarg]);
          }
        }
      }
    }
  }
  return OK;
}



/*
  The last argument (config_file) is only used for printing
  informative error messages, and can be NULL. The config_cwd is
  essential if we are looking up a filename, otherwise it can be NULL.

  Returns a string with an error description, or NULL if the supplied
  arguments were OK. The string is allocated here, but is assumed that
  calling scope will free it.
*/

static void config_item_set_arg__(config_type * config , config_item_type * item , stringlist_type * token_list , const char * config_file , const char * config_cwd) {
  int argc = stringlist_get_size( token_list ) - 1;

  if (argc == 1 && (strcmp(stringlist_iget(token_list , 1) , CLEAR_STRING) == 0)) {
    config_item_clear(item);
  } else {
    config_item_node_type * node;
    
    if (item->append_arg)
      node = config_item_get_new_node(item);
    else {
      node = config_item_get_first_node(item);
      config_item_node_clear(node);
    }


    /* Filtering based on DEFINE statements */
    if (subst_list_get_size( config->define_list ) > 0) {
      int iarg;
      for (iarg = 0; iarg < argc; iarg++) {
        char * filtered_copy = subst_list_alloc_filtered_string( config->define_list , stringlist_iget(token_list , iarg + 1));
        stringlist_iset_owned_ref( token_list , iarg + 1 , filtered_copy);
      }
    }

    
    /* Filtering based on environment variables */
    if (item->expand_envvar) {
      int iarg;
      for (iarg = 0; iarg < argc; iarg++) {
        int    env_offset = 0;
        char * env_var;
        do {
          env_var = util_isscanf_alloc_envvar(  stringlist_iget(token_list , iarg + 1) , env_offset );
          if (env_var != NULL) {
            const char * env_value = getenv( &env_var[1] );
            if (env_value != NULL) {
              char * new_value = util_string_replace_alloc( stringlist_iget( token_list , iarg + 1 ) , env_var , env_value );
              stringlist_iset_owned_ref( token_list , iarg + 1 , new_value );
            } else {
              env_offset += 1;
              fprintf(stderr,"** Warning: environment variable: %s is not defined \n", env_var);
            }
          }
        } while (env_var != NULL);
      }
    }
    
    if (config_item_validate_set(config , item , token_list , config_file, config_cwd)) {
      config_item_node_set(node , token_list);
      item->currently_set = true;
      
      if (config_cwd != NULL)
        node->config_cwd = util_alloc_string_copy( config_cwd );
      else
        node->config_cwd = util_alloc_cwd(  );  /* For use from external scope. */
    }

  }
}
 






void config_item_validate(config_type * config , const config_item_type * item) {
  
  if (item->currently_set) {

    if (item->required_children != NULL) {
      int i;
      for (i = 0; i < stringlist_get_size(item->required_children); i++) {
        if (!config_has_set_item(config , stringlist_iget(item->required_children , i))) {
          char * error_message = util_alloc_sprintf("When:%s is set - you also must set:%s.",item->kw , stringlist_iget(item->required_children , i));
          config_add_and_free_error(config , error_message);
        }
      }
    }

    if (item->required_children_value != NULL) {
      int inode;
      for (inode = 0; inode < config_item_get_occurences(item); inode++) {
        config_item_node_type * node   = config_item_iget_node(item , inode);
        stringlist_type       * values = node->stringlist;
        int is;

        for (is = 0; is < stringlist_get_size(values); is++) {
          const char * value = stringlist_iget(values , is);
          if (hash_has_key(item->required_children_value , value)) {
            stringlist_type * children = hash_get(item->required_children_value , value);
            int ic;
            for (ic = 0; ic < stringlist_get_size( children ); ic++) {
              const char * req_child = stringlist_iget( children , ic );
              if (!config_has_set_item(config , req_child )) {
                char * error_message = util_alloc_sprintf("When:%s is set to:%s - you also must set:%s.",item->kw , value , req_child );
                config_add_and_free_error(config , error_message);
              }
            }
          }
        }
      }
    }
  } else if (item->required_set) {  /* The item is not set ... */
    char * error_message = util_alloc_sprintf("Item:%s must be set.",item->kw);
    config_add_and_free_error(config , error_message);
  }
}


void config_item_fprintf(const config_item_type * item , FILE * stream) {
  fprintf(stream, "%s \n",item->kw);
  for (int i=0; i < item->node_size; i++) 
    config_item_node_fprintf(item->nodes[i] , i , stream);

  if (item->required_children_value != NULL) {
    char ** values = hash_alloc_keylist( item->required_children_value );
    
    for (int i=0; i < hash_get_size(item->required_children_value ); i++) {
      fprintf(stream , "  %-10s: ",values[i]);
      stringlist_fprintf(hash_get( item->required_children_value , values[i]) , " " , stream);
      fprintf(stream , "\n");
    }
    
    util_free_stringlist( values , hash_get_size( item->required_children_value ));
  }
}


void config_item_free( config_item_type * item) {
  free(item->kw);
  {
    int i;
    for (i = 0; i < item->node_size; i++)
      config_item_node_free( item->nodes[i] );
    free(item->nodes);
  }
  if (item->required_children       != NULL) stringlist_free(item->required_children);
  if (item->required_children_value != NULL) hash_free(item->required_children_value); 
  validate_free(item->validate);
  free(item);
}





void config_item_free__ (void * void_item) {
  config_item_type * item = (config_item_type *) void_item;
  if (item->__id != CONFIG_ITEM_ID) 
    util_abort("%s: internal error - cast failed \n",__func__);
  
  config_item_free( item );
}


bool config_item_is_set(const config_item_type * item) {
  return item->currently_set;
}

void config_item_set_common_selection_set(config_item_type * item , int argc , const char ** argv) {
  validate_set_common_selection_set(item->validate , argc , argv);
}

void config_item_set_indexed_selection_set(config_item_type * item , int index , int argc , const char ** argv) {
  validate_set_indexed_selection_set(item->validate , index , argc , argv);
}


void config_item_set_required_children(config_item_type * item , stringlist_type * stringlist) {
  item->required_children = stringlist_alloc_deep_copy(stringlist);
}

void config_item_add_required_children(config_item_type * item , const char * child_key) {
  if (item->required_children == NULL)
    item->required_children = stringlist_alloc_new();
  
  stringlist_append_copy( item->required_children , child_key );
}



/**
   This works in the following way: 

     if item == value {
        All children in child_list must also be set.
     }

     
*/        

void config_item_set_required_children_on_value(config_item_type * item , const char * value , stringlist_type * child_list) {
  if (item->required_children_value == NULL)
    item->required_children_value = hash_alloc();
  hash_insert_hash_owned_ref( item->required_children_value , value , stringlist_alloc_deep_copy(child_list) , stringlist_free__);
}



/**
   This function is used to set the minimum and maximum number of
   arguments for an item. In addition you can pass in a pointer to an
   array of config_item_types values which will be used for validation
   of the input. This vector must be argc_max elements long; it can be
   NULL.
*/


void config_item_set_argc_minmax(config_item_type * item , int argc_min , int argc_max, int type_map_size , const config_item_types * type_map) {
  validate_set_argc_minmax(item->validate , argc_min , argc_max , type_map_size , type_map);
}
  


void config_item_set_envvar_expansion( config_item_type * item , bool expand_envvar ) {
  item->expand_envvar = expand_envvar;
}


#undef __TYPE__



/*****************************************************************/



config_type * config_alloc() {
  config_type *config     = util_malloc(sizeof * config );
  config->items           = hash_alloc();
  config->parse_errors    = stringlist_alloc_new();
  config->parsed_files    = set_alloc_empty();
  config->messages        = hash_alloc();
  config->define_list     = subst_list_alloc( NULL );
  config->config_file     = NULL;
  config->abs_path        = NULL;
  config->auto_items      = hash_alloc( );
  return config;
}


void config_free(config_type * config) {
  util_safe_free( config->config_file );
  util_safe_free( config->abs_path );
  hash_free(config->items);
  hash_free(config->messages);
  stringlist_free(config->parse_errors);
  set_free(config->parsed_files);
  subst_list_free( config->define_list );
  hash_free( config->auto_items );
  free(config);
}



static void config_insert_item__(config_type * config , const char * kw , const config_item_type * item , bool ref) {
  if (ref)
    hash_insert_ref(config->items , kw , item);
  else
    hash_insert_hash_owned_ref(config->items , kw , item , config_item_free__);
}


/**
   This function allocates a simple item with all values
   defaulted. The item is added to the config object, and a pointer is
   returned to the calling scope. If you want to change the properties
   of the item you can do that with config_item_set_xxxx() functions
   from the calling scope.
*/


config_item_type * config_add_item(config_type * config , 
                                   const char  * kw, 
                                   bool  required  , 
                                   bool  append_arg) {
  
  config_item_type * item = config_item_alloc( kw , required , append_arg);
  config_insert_item__(config , kw , item , false);
  return item;
}



/**
  This is a minor wrapper for adding an item with the properties. 

    1. It has argc_minmax = {1,1}
    2. It has append == false.
    
   The value can than be extracted with config_get_value() and
   config_get_value_as_xxxx functions. 
*/

config_item_type * config_add_key_value( config_type * config , const char * key , bool required , config_item_types item_type) {
  config_item_type * item = config_add_item( config , key , required , false );
  config_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types  [1]) { item_type });
  return item;
}



bool config_has_item(const config_type * config , const char * kw) {
  return hash_has_key(config->items , kw);
}

config_item_type * config_get_item(const config_type * config , const char * kw) {
  return hash_get(config->items , kw);
}

bool config_item_set(const config_type * config , const char * kw) {
  return config_item_is_set(hash_get(config->items , kw));
}





void config_add_define( config_type * config , const char * key , const char * value ) {
  subst_list_append_copy( config->define_list , key , value , NULL );
}




char ** config_alloc_active_list(const config_type * config, int * _active_size) {
  char ** complete_key_list = hash_alloc_keylist(config->items);
  char ** active_key_list = NULL;
  int complete_size = hash_get_size(config->items);
  int active_size   = 0;
  int i;

  for( i = 0; i < complete_size; i++) {
    if  (config_item_is_set(config_get_item(config , complete_key_list[i]) )) {
      active_key_list = util_stringlist_append_copy(active_key_list , active_size , complete_key_list[i]);
      active_size++;
    }
  }
  *_active_size = active_size;
  util_free_stringlist(complete_key_list , complete_size);
  return active_key_list;
}




static void config_validate(config_type * config, const char * filename) {
  int size = hash_get_size(config->items);
  char ** key_list = hash_alloc_keylist(config->items);
  int ikey;
  for (ikey = 0; ikey < size; ikey++) {
    const config_item_type * item = config_get_item(config , key_list[ikey]);
    config_item_validate(config , item);
  }
  util_free_stringlist(key_list , size);

  if (stringlist_get_size(config->parse_errors) > 0) {
    fprintf(stderr,"Parsing errors in configuration file \'%s\':\n" , filename);
    stringlist_fprintf(config->parse_errors , "\n", stderr);
    util_exit("");
  }
  
}


/**
   If the parser is set up to include unknown items (a bit dangerous
   because misspellings++ will not be catched) the items which have
   been auto-added can be retrieved afterwards with the function
   config_pop_auto_items() which will return a hash table; the calling
   scope takes ownership of this table (removing it from the config
   object), and must free it with hash_free(). 

   The hash table is organized as follows:

   
     auto_items = { "KEY1"      : Vector( Stringlist("Val1","Val2","Val3") , Stringlist("ValX","ValY") ) , 
                   "Transport" : Vector( Stringlist("Car" , "Boat" , "Plane")),
                   "Sex"       : Vector( Stringlist("Male" , "Female" , "????") , Stringlist("yes")) }

  i.e. each element is a vector stringlist instances.                 

*/

hash_type * config_pop_auto_items( config_type * config ) {
  hash_type * auto_items = config->auto_items;              /* If the same config instance is used to parse again - the auto
                                                               items will from now on have lost their special auto status. */
  config->auto_items = hash_alloc();
  return auto_items;
}


static void config_append_auto_item( config_type * config , const char * key , const stringlist_type * token_list) {
  vector_type * v     = hash_get( config->auto_items , key );
  stringlist_type * s = stringlist_alloc_deep_copy_with_offset( token_list , 1);
  
  vector_append_owned_ref( v , s , stringlist_free__);
}





/**
   This function parses the config file 'filename', and updated the
   internal state of the config object as parsing proceeds. If
   comment_string != NULL everything following 'comment_string' on a
   line is discarded.

   include_kw is a string identifier for an include functionality, if
   an include is encountered, the included file is parsed immediately
   (through a recursive call to config_parse__). if include_kw == NULL,
   include files are not supported.

   Observe that use of include, relative paths and all that shit is
   quite tricky. The following is currently implemented:

    1. The front_end function will split the path to the config file
       in a path_name component and a file component.

    2. Recursive calls to config_parse__() will keep control of the
       parsers notion of cwd (note that the real OS'wise cwd never
       changes), and every item is tagged with the config_cwd
       currently active.

    3. When an item has been entered with type CONFIG_FILE /
       CONFIG_DIRECTORY / CONFIG_EXECUTABLE - the item is updated to
       reflect to be relative (iff it is relative in the first place)
       to the path of the root config file.

   These are not strict rules - it is possible to get other things to
   work as well, but the problem is that it very quickly becomes
   dependant on 'arbitrariness' in the parsing configuration.

   auto_add: whether unrecognized keywords should be added to the the
             config object.  

   validate: whether we should validate when complete, that should
             typically only be done at the last parsing.

      
   define_kw: This a string which can serve as a "#define" for the
   parsing. The define_kw keyword should have two arguments - a key
   and a value. If the define_kw is present all __subsequent__
   occurences of 'key' are replaced with 'value'.  alloc_new_key
   is an optinal function (can be NULL) which is used to alloc a new
   key, i.e. add leading and trailing 'magic' characters.


   Example:  
   --------

   char * add_angular_brackets(const char * key) {
       char * new_key = util_alloc_sprintf("<%s>" , key);
   }



   config_parse(... , "myDEF" , add_angular_brackets , ...) 


   Config file:
   -------------
   myDEF   Name         BJARNE
   myDEF   sexual-pref  Dogs
   ...
   ... 
   PERSON  <Name> 28 <sexual-pref>      
   ...
   ------------

   After parsing we will have an entry: "NAME" , "Bjarne" , "28" , "Dogs".

   The key-value pairs internalized during the config parsing are NOT
   returned to the calling scope in any way.
*/


static void config_parse__(config_type * config , 
                           const char * config_cwd , 
                           const char * _config_file, 
                           const char * comment_string , 
                           const char * include_kw ,
                           const char * define_kw , 
                           bool warn_unrecognized,
                           bool auto_add , 
                           bool validate) {
  char * config_file  = util_alloc_filename(config_cwd , _config_file , NULL);
  char * abs_filename = util_alloc_realpath(config_file);
  parser_type * parser = parser_alloc(" \t" , "\"", NULL , NULL , "--" , "\n");

  if (!set_add_key(config->parsed_files , abs_filename)) 
    util_exit("%s: file:%s already parsed - circular include ? \n",__func__ , config_file);
  else {
    FILE * stream = util_fopen(config_file , "r");
    bool   at_eof = false;
  
    while (!at_eof) {
      int    active_tokens;
      stringlist_type * token_list;
      char  *line_buffer;

      
      line_buffer  = util_fscanf_alloc_line(stream , &at_eof);
      if (line_buffer != NULL) {
        token_list = parser_tokenize_buffer(parser , line_buffer , true);
        active_tokens = stringlist_get_size( token_list );

        /*
        util_split_string(line_buffer , " \t" , &tokens , &token_list);
        active_tokens = tokens;
        for (i = 0; i < tokens; i++) {
          char * comment_ptr = NULL;
          if(comment_string != NULL)
            comment_ptr = strstr(token_list[i] , comment_string);
          
          if (comment_ptr != NULL) {
            if (comment_ptr == token_list[i])
              active_tokens = i;
            else
              active_tokens = i + 1;
            break;
          }
          }
        */

        if (active_tokens > 0) {
          const char * kw = stringlist_iget( token_list , 0 );

          /*Treating the include keyword. */
          if (include_kw != NULL && (strcmp(include_kw , kw) == 0)) {
            if (active_tokens != 2) 
              util_abort("%s: keyword:%s must have exactly one argument. \n",__func__ ,include_kw);
            {
              char *include_path  = NULL;
              char *extension     = NULL;
              char *include_file  = NULL;

              {
                char * tmp_path;
                char * tmp_file;
                util_alloc_file_components(stringlist_iget(token_list , 1) , &tmp_path , &tmp_file , &extension);

                /* Allocating a new path with current config_cwd and the (relative) path to the new config_file */
                if (tmp_path == NULL)
                  include_path = util_alloc_string_copy( config_cwd );
                else {
                  if (!util_is_abs_path(tmp_path)) 
                    include_path = util_alloc_filename(config_cwd , tmp_path , NULL);
                  else
                    include_path = util_alloc_string_copy(tmp_path);
                }
                
                include_file = util_alloc_filename(NULL , tmp_file , extension);
                free(tmp_file);
                free(tmp_path);
              }

              config_parse__(config , include_path , include_file , comment_string , include_kw , define_kw , warn_unrecognized, auto_add , false); /* Recursive call */
              util_safe_free(include_file);
              util_safe_free(include_path);
            }
          } else if ((define_kw != NULL) && (strcmp(define_kw , kw) == 0)) {
            if (active_tokens < 3) 
              util_abort("%s: keyword:%s must have exactly one (or more) arguments. \n",__func__ , define_kw);
            {
              char * key   = util_alloc_string_copy( stringlist_iget(token_list ,1) );
              char * value = stringlist_alloc_joined_substring( token_list , 2 , active_tokens , " ");
              
              {
                char * filtered_value = subst_list_alloc_filtered_string( config->define_list , value);
                config_add_define( config , key , filtered_value );
                free( filtered_value );
              }
              free(key);
              free(value);
            }
          } else {
            bool auto_item = false;
            if (hash_has_key(config->messages , kw))
              printf("%s \n",(char *) hash_get(config->messages , kw));
            
            if (!config_has_item(config , kw)) {
              if (warn_unrecognized)
                fprintf(stderr,"** Warning keyword:%s not recognized when parsing:%s --- \n" , kw , config_file);

              if (auto_add) {
                config_add_item(config , kw , true , false);                        /* Auto created items get append_arg == true, and required == true (which is trivially satisfied). */
                hash_insert_hash_owned_ref( config->auto_items , kw , vector_alloc_new() , vector_free__ );
              }
            }
            
            if (config_has_item(config , kw)) {
              config_item_type * item = config_get_item(config , kw);
              config_item_set_arg__(config , item , token_list , config_file , config_cwd);
            } 
            
            if (auto_item)
              config_append_auto_item( config , kw , token_list);
          }
        }
        stringlist_free(token_list);
        free(line_buffer);
      }
    }
    if (validate) config_validate(config , config_file);
    fclose(stream);
  }
  parser_free( parser );
  free(abs_filename);
  free(config_file);
}



static void config_set_config_file( config_type * config , const char * config_file ) {
  config->config_file = util_realloc_string_copy( config->config_file , config_file );
  
  util_safe_free(config->abs_path);
  config->abs_path = util_alloc_abs_path( config_file );
}


const char * config_get_config_file( const config_type * config , bool abs_path) {
  if (abs_path)
    return config->abs_path;
  else
    return config->config_file;
}


void config_parse(config_type * config , 
                  const char * filename, 
                  const char * comment_string , 
                  const char * include_kw ,
                  const char * define_kw , 
                  bool warn_unrecognized,
                  bool auto_add , 
                  bool validate) {
  char * config_path;
  char * config_file;
  char * tmp_file;
  char * extension;
  util_alloc_file_components(filename , &config_path , &tmp_file , &extension);
  config_set_config_file( config , filename );
  config_file = util_alloc_filename(NULL , tmp_file , extension);
  config_parse__(config , config_path , config_file , comment_string , include_kw , define_kw , warn_unrecognized , auto_add , validate);

  util_safe_free(tmp_file);
  util_safe_free(extension);
  util_safe_free(config_path);
  util_safe_free(config_file);
}



bool config_has_keys(const config_type * config, const char **ext_keys, int ext_num_keys, bool exactly)
{
  int i;

  int     config_num_keys;
  char ** config_keys;

  config_keys = config_alloc_active_list(config, &config_num_keys);

  if(exactly && (config_num_keys != ext_num_keys))
  {
    util_free_stringlist(config_keys,config_num_keys);
    return false;
  }

  for(i=0; i<ext_num_keys; i++)
  {
    if(!config_has_item(config,ext_keys[i]))
    {
      util_free_stringlist(config_keys,config_num_keys);
      return false;
    }
  }
 
  util_free_stringlist(config_keys,config_num_keys);
  return true;
}




/*****************************************************************/
/* 
   Here comes some xxx_get() functions - many of them will fail if
   the item has not been added in the right way (this is to ensure that
   the xxx_get() request is unambigous. 
*/


/**
   This function can be used to get the value of a config
   parameter. But to ensure that the get is unambigous we set the
   following requirements to the item corresponding to 'kw':

    * It has been added with append_arg == false.
    * argc_minmax has been set to 1,1

   If this is not the case - we die.
*/

/**
   Assume we installed a key 'KEY' which occurs three times in the final 
   config file:

   KEY  1    2     3
   KEY  11   22    33
   KEY  111  222   333


   Now when accessing these values the occurence variable will
   correspond to the linenumber, and the index will index along a line:

     config_iget_as_int( config , "KEY" , 0 , 2) => 3
     config_iget_as_int( config , "KEY" , 2 , 1) => 222
*/



bool config_iget_as_bool(const config_type * config , const char * kw, int occurence , int index) {
  config_item_type * item = config_get_item(config , kw);
  return config_item_iget_as_bool(item , occurence , index);
}


int config_iget_as_int(const config_type * config , const char * kw, int occurence , int index) {
  config_item_type * item = config_get_item(config , kw);
  return config_item_iget_as_int(item , occurence , index);
}


double config_iget_as_double(const config_type * config , const char * kw, int occurence , int index) {
  config_item_type * item = config_get_item(config , kw);
  return config_item_iget_as_double(item , occurence , index);
}


/** 
    As the config_get function, but the argc_minmax requiremnt has been removed.
*/
const char * config_iget(const config_type * config , const char * kw, int occurence , int index) {
  config_item_type * item = config_get_item(config , kw);
  
  return config_item_iget(item , occurence , index);
}


/**
   This function will return NULL is the item has not been set, 
   however it must be installed with config_add_item().
*/
const char * config_safe_iget(const config_type * config , const char *kw, int occurence , int index) {
  const char * value = NULL;

  config_item_type * item = config_get_item(config , kw);
  if (config_item_is_set(item)) {
    if (occurence < config_item_get_occurences( item )) {
      config_item_node_type * node = config_item_iget_node( item , occurence );
      value = stringlist_safe_iget( node->stringlist , index);
    }
  }
  return value;
}


static void assert_key_value(const config_item_type * item) {
  if (!((item->validate->argc_min == 1) && (item->validate->argc_min == 1)))
    util_abort("%s: item:%s before calling config_get_value() functions *without* index you must set argc_min == argc_max = 1 \n",__func__ , item->kw);
  
  if (item->append_arg) 
    util_abort("%s: must have append_arg == false for _get_value functions \n",__func__);
}

/**
   The config_get_value_??() functions are simpler wrappers for:

    1. Check that item has been added with append==false and
       arg_minmax = {1,1}, i.e. it has typically been added with
       config_add_key_value().

    2. Call config_iget_?? with both the occurence and index values
       set to 0.
*/

bool config_get_value_as_bool(const config_type * config , const char * kw) {
  config_item_type * item = config_get_item(config , kw);
  assert_key_value( item );
  return config_item_iget_as_bool(item , 0 , 0);
}

int config_get_value_as_int(const config_type * config , const char * kw) {
  config_item_type * item = config_get_item(config , kw);
  assert_key_value( item );
  return config_item_iget_as_int(item , 0 , 0);
}

double config_get_value_as_double(const config_type * config , const char * kw) {
  config_item_type * item = config_get_item(config , kw);
  assert_key_value( item );
  return config_item_iget_as_double(item , 0 , 0);
}

const char * config_get_value(const config_type * config , const char * kw) {
  config_item_type * item = config_get_item(config , kw);
  assert_key_value( item );
  return config_item_iget(item , 0 , 0);
}





const stringlist_type * config_get_stringlist_ref(const config_type * config , const char * kw) {
  config_item_type * item = config_get_item(config , kw);

  return config_item_get_stringlist_ref(item);
}



stringlist_type * config_iget_stringlist_ref(const config_type * config , const char * kw, int occurence) {
  config_item_type * item = config_get_item(config , kw);
  
  return config_item_iget_stringlist_ref(item , occurence);
}


/**
  This function allocates a new stringlist containing *ALL* the
  arguements for an item. With reference to the illustrated example at
  the top the function call:

     config_alloc_complete_strtinglist(config , "KEY1");

     would produce the list: ("ARG1" "ARG2" "ARG2" "VERBOSE"), i.e. the
  arguments for the various occurences of "KEY1" are collapsed to one
  stringlist.
*/

  
stringlist_type * config_alloc_complete_stringlist(const config_type* config , const char * kw) {
  bool copy = true;
  config_item_type * item = config_get_item(config , kw);
  return config_item_alloc_complete_stringlist(item , copy);
}



/**
   It is enforced that kw-item has been added with append_arg == false.
*/
stringlist_type * config_alloc_stringlist(const config_type * config , const char * kw) {
  config_item_type * item = config_get_item(config , kw);
  return config_item_alloc_stringlist(item , true);
}


/**
   Now accepts kw-items which have not been added with append_arg == false.
*/

char * config_alloc_joined_string(const config_type * config , const char * kw, const char * sep) {
  config_item_type * item = config_get_item(config , kw);
  return config_item_alloc_joined_string(item , sep);
}





/**
   Return the number of times a keyword has been set - dies on unknown 'kw';
*/

int config_get_occurences(const config_type * config, const char * kw) {
  return config_item_get_occurences(config_get_item(config , kw));
}

int config_get_occurence_size( const config_type * config , const char * kw , int occurence) {
  config_item_type      * item = config_get_item(config , kw);
  config_item_node_type * node = config_item_iget_node( item , occurence );
  return stringlist_get_size( node->stringlist );
}



/**
   Allocates a hash table for situations like this:

ENV   PATH              /some/path
ENV   LD_LIBARRY_PATH   /some/other/path
ENV   MALLOC            STRICT
....

the returned hash table will be: {"PATH": "/som/path", "LD_LIBARRY_PATH": "/some/other_path" , "MALLOC": "STRICT"}

It is enforced that:

 * item is allocated with append_arg = true
 * item is allocated with argc_minmax = 2,2
 
 The hash takes copy of the values in the hash so the config object
 can safefly be freed (opposite if copy == false).
*/


hash_type * config_alloc_hash(const config_type * config , const char * kw) {
  bool copy = true;
  config_item_type * item = config_get_item(config , kw);
  return config_item_alloc_hash(item , copy);
}



bool config_has_set_item(const config_type * config , const char * kw) {
  if (config_has_item(config , kw)) {
    config_item_type * item = config_get_item(config , kw);
    return config_item_is_set(item);
  } else
    return false;
}


/**
   This function adds an alias to an existing item; so that the
   value+++ of an item can be referred to by two different names.
*/


void config_add_alias(config_type * config , const char * src , const char * alias) {
  if (config_has_item(config , src)) {
    config_item_type * item = config_get_item(config , src);
    config_insert_item__(config , alias , item , true);
  } else
    util_abort("%s: item:%s not recognized \n",__func__ , src);
}



void config_install_message(config_type * config , const char * kw, const char * message) {
  hash_insert_hash_owned_ref(config->messages , kw , util_alloc_string_copy(message) , free);
}


void config_fprintf_item_list(const config_type * config , FILE * stream) {
  stringlist_type * items = hash_alloc_stringlist( config->items );
  stringlist_sort( items , NULL );
  {
    int i; 
    for (i=0; i < stringlist_get_size( items ); i++) 
      fprintf(stream , "%s \n",stringlist_iget( items , i));
  }

  stringlist_free( items );
}


/*****************************************************************/




