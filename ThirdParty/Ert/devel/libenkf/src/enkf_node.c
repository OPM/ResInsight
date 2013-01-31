/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/buffer.h>
#include <ert/util/msg.h>
#include <ert/util/rng.h>
#include <ert/util/vector.h>
#include <ert/util/path_fmt.h>

#include <ert/enkf/enkf_node.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/field.h>
#include <ert/enkf/surface.h>
#include <ert/enkf/summary.h>
#include <ert/enkf/ecl_static_kw.h>
#include <ert/enkf/gen_kw.h>
#include <ert/enkf/gen_data.h>
#include <ert/enkf/container.h>
#include <ert/enkf/enkf_serialize.h>

/**
   A small illustration (says more than thousand words ...) of how the
   enkf_node, enkf_config_node, field[1] and field_config[1] objects
   are linked.


   ================
   |              |   o-----------
   |  ================           |                =====================
   |  |              |   o--------                |                   |
   |  |  ================        |------------->  |                   |
   |  |  |              |        |                |  enkf_config_node |
   |  |  |              |        |                |                   |
   ===|  |  enkf_node   |  o------                |                   |
   o |  |              |                         |                   |
   | ===|              |                         =====================
   |  o |              |                                   o
   |  | ================                                   |
   |  |        o                                           |
   |  \        |                                           |
   |   \       |                                           |
   |    |      |                                           |
   |    |      |                                           |
   |    |      |                                           |
   |    |      |                                           |
   \|/   |      |                                           |
   ======|======|==                                        \|/
   |    \|/     | |   o-----------
   |  ==========|=====           |                =====================
   |  |        \|/   |   o--------                |                   |
   |  |  ================        |------------->  |                   |
   |  |  |              |        |                |  field_config     |
   |  |  |              |        |                |                   |
   ===|  |  field       |  o------                |                   |
   |  |              |                         |                   |
   ===|              |                         =====================
   |              |
   ================


   To summarize in words:

   * The enkf_node object is an abstract object, which again contains
   a spesific enkf_object, like e.g. the field objects shown
   here. In general we have an ensemble of enkf_node objects.

   * The enkf_node objects contain a pointer to a enkf_config_node
   object.

   * The enkf_config_node object contains a pointer to the spesific
   config object, i.e. field_config in this case.

   * All the field objects contain a pointer to a field_config object.


   [1]: field is just an example, and could be replaced with any of
   the enkf object types.
*/

/*-----------------------------------------------------------------*/

/**
   A note on memory 
   ================ 

   The enkf_nodes can consume large amounts of memory, and for large
   models/ensembles we have a situation where not all the
   members/fields can be in memory simultanouesly - such low-memory
   situations are not really supported at the moment, but we have
   implemented some support for such problems:

   o All enkf objects should have a xxx_realloc_data() function. This
   function should be implemented in such a way that it is always
   safe to call, i.e. if the object already has allocated data the
   function should just return.

   o All enkf objects should implement a xxx_free_data()
   function. This function free the data of the object, and set the
   data pointer to NULL.


   The following 'rules' apply to the memory treatment:
   ----------------------------------------------------

   o Functions writing to memory can always be called, and it is their
   responsibility to allocate memory before actually writing on it. The
   writer functions are:

   enkf_node_initialize()
   enkf_node_fread()
   enkf_node_forward_load()  

   These functions should all start with a call to
   enkf_node_ensure_memory(). The (re)allocation of data is done at
   the enkf_node level, and **NOT** in the low level object
   (altough that is where it is eventually done of course).

   o When it comes to functions reading memory it is a bit more
   tricky. It could be that if the functions are called without
   memory, that just means that the object is not active or
   something (and the function should just return). On the other
   hand trying to read a NULL pointer does indicate that program
   logic is not fully up to it? And should therefor maybe be
   punished?

   o The only memory operation which is exported to 'user-space'
   (i.e. the enkf_state object) is enkf_node_free_data(). 

*/


/**
   Keeeping track of node state.
   =============================

   To keep track of the state of the node's data (actually the data of
   the contained enkf_object, i.e. a field) we have three higly
   internal variables __state, __modified , __iens, and
   __report_step. These three variables are used/updated in the
   following manner:
   


   1. The nodes are created with (modified, report_step, state, iens) ==
   (true , -1 , undefined , -1).

   2. After initialization we set: report_step -> 0 , state ->
   analyzed, modified -> true, iens -> -1

   3. After load (both from ensemble and ECLIPSE). We set modified ->
   false, and report_step, state and iens according to the load
   arguments.
      
   4. After deserialize (i.e. update) we set modified -> true.
      
   5. After write (to ensemble) we set in the same way as after load.
  
   6. After free_data we invalidate according to the newly allocated
   status.

   7. In the ens_load routine we check if modified == false and the
   report_step and state arguments agree with the current
   values. IN THAT CASE WE JUST RETURN WITHOUT ACTUALLY HITTING
   THE FILESYSTEM. This performance gain is the main point of the
   whole excercise.
*/




struct enkf_node_struct {
  alloc_ftype                    * alloc;
  ecl_write_ftype                * ecl_write;
  forward_load_ftype             * forward_load;
  forward_load_vector_ftype      * forward_load_vector;
  free_data_ftype                * free_data;
  user_get_ftype                 * user_get;
  user_get_vector_ftype          * user_get_vector;
  set_inflation_ftype            * set_inflation;
  fload_ftype                    * fload;
  has_data_ftype                 * has_data;

  serialize_ftype                * serialize;
  deserialize_ftype              * deserialize;
  read_from_buffer_ftype         * read_from_buffer;
  write_to_buffer_ftype          * write_to_buffer;
  initialize_ftype               * initialize;
  node_free_ftype                * freef;
  clear_ftype                    * clear;
  node_copy_ftype                * copy;
  scale_ftype                    * scale;
  iadd_ftype                     * iadd;
  imul_ftype                     * imul;
  isqrt_ftype                    * isqrt;
  iaddsqr_ftype                  * iaddsqr;
  
  /******************************************************************/
  bool                         vector_storage;
  char                        *node_key;         /* The (hash)key this node is identified with. */
  void                        *data;             /* A pointer to the underlying enkf_object, i.e. gen_kw_type instance, or a field_type instance or ... */
  const enkf_config_node_type *config;           /* A pointer to a enkf_config_node instance (which again cointans a pointer to the config object of data). */
  /*****************************************************************/
  
  vector_type                 *container_nodes;
  
  /*****************************************************************/
  /* The variables below this line are VERY INTERNAL.              */
  bool                __modified;           /* __modified, __report_step, __iens and __state are internal variables trying  */
  node_id_type        __node_id;            /* to record the state of the in-memory reporesentation of the node->data. See */ 
                                            /* the documentation with heading "Keeping track of node state". */
                                            /* Observe that this __iens  variable "should not be used" - a node can change __iens value during run. */
  state_enum          __load_state;
};


const enkf_config_node_type * enkf_node_get_config(const enkf_node_type * node) {
  return node->config;
}


bool enkf_node_vector_storage( const enkf_node_type * node ) {
  return node->vector_storage;
}


/*****************************************************************/

/*
  All the function pointers REALLY should be in the config object ...
*/


#define FUNC_ASSERT(func) if (func == NULL) util_abort("%s: function handler: %s not registered for node:%s - aborting\n",__func__ , #func , enkf_node->node_key); 




void enkf_node_alloc_domain_object(enkf_node_type * node) {
  if (node->data != NULL)
    node->freef( node->data );
  node->data = node->alloc(enkf_config_node_get_ref(node->config));
}




enkf_node_type * enkf_node_copyc(const enkf_node_type * enkf_node) {
  FUNC_ASSERT(enkf_node->copy);  
  {
    const enkf_node_type * src = enkf_node;
    enkf_node_type * target;
    target = enkf_node_alloc(src->config);
    src->copy( src->data , target->data );  /* Calling the low level copy function */
    return target;
  }
}



bool enkf_node_include_type(const enkf_node_type * enkf_node, int mask) {
  return enkf_config_node_include_type(enkf_node->config , mask);
}


ert_impl_type enkf_node_get_impl_type(const enkf_node_type * enkf_node) {
  return enkf_config_node_get_impl_type(enkf_node->config);
}


enkf_var_type enkf_node_get_var_type(const enkf_node_type * enkf_node) {
  return enkf_config_node_get_var_type(enkf_node->config);
}





void * enkf_node_value_ptr(const enkf_node_type * enkf_node) {
  return enkf_node->data;
}



/**
   This function calls the node spesific ecl_write function. IF the
   ecl_file of the (node == NULL) *ONLY* the path is sent to the node
   spesific file.
*/

void enkf_node_ecl_write(const enkf_node_type *enkf_node , const char *path , fortio_type * restart_fortio , int report_step) {
  if (enkf_node->ecl_write != NULL) {
    char * node_eclfile = enkf_config_node_alloc_outfile(enkf_node->config , report_step); /* Will return NULL if the node does not have any outfile format. */
    /*
      If the node does not have a outfile (i.e. ecl_file), the
      ecl_write function will be called with file argument NULL. It
      is then the responsability of the low-level implementation to
      do "the right thing".
    */
    enkf_node->ecl_write(enkf_node->data , path , node_eclfile , restart_fortio);
    util_safe_free( node_eclfile );
  }
}



/**
   This function takes a string - key - as input an calls a node
   specific function to look up one scalar based on that key. The key
   is always a string, but the the type of content will vary for the
   different objects. For a field, the key will be a string of "i,j,k"
   for a cell.

   If the user has asked for something which does not exist the
   function SHOULD NOT FAIL; it should return false and set the *value
   to 0.
*/


bool enkf_node_user_get(enkf_node_type * enkf_node , enkf_fs_type * fs , const char * key , node_id_type node_id , double * value) {
  bool loadOK;
  FUNC_ASSERT( enkf_node->user_get );
  {
    loadOK = enkf_node_try_load( enkf_node , fs , node_id);
    
    if (loadOK) 
      return enkf_node->user_get(enkf_node->data , key , node_id.report_step,  node_id.state , value);
    else {
      *value = 0;
      return false;
    }

  }
}


bool enkf_node_user_get_vector( enkf_node_type * enkf_node , enkf_fs_type * fs , const char * key , int iens , state_enum state , double_vector_type * values) {
  if (enkf_node->vector_storage) {
    if (enkf_node_try_load_vector( enkf_node , fs , iens , state)) {
      enkf_node->user_get_vector( enkf_node->data , key , state , values);
      return true;
    } else
      return false;
  } else {
    util_abort("%s: internal error - function should only be called by nodes with vector storage.\n",__func__);
    return false;
  }
}



void enkf_node_fload( enkf_node_type * enkf_node , const char * filename ) {
  FUNC_ASSERT( enkf_node->fload );
  enkf_node->fload( enkf_node->data , filename );
}



/**
   This function loads (internalizes) ECLIPSE results, the ecl_file
   instance with restart data, and the ecl_sum instance with summary
   data must already be loaded by the calling function.

   IFF the enkf_node has registered a filename to load from, that is
   passed to the specific load function, otherwise the run_path is sent
   to the load function.

   If the node does not have a forward_load function, the function just
   returns.
*/


bool enkf_node_forward_load(enkf_node_type *enkf_node , const char * run_path , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_block , int report_step, int iens ) {
  bool loadOK;
  FUNC_ASSERT(enkf_node->forward_load);
  {
    if (enkf_node_get_impl_type(enkf_node) == SUMMARY)
      /* Fast path for loading summary data. */
      loadOK = enkf_node->forward_load(enkf_node->data , NULL  , ecl_sum , restart_block , report_step);
    else {
      char * input_file = enkf_config_node_alloc_infile(enkf_node->config , report_step);
      
      if (input_file != NULL) {
        char * file = util_alloc_filename( run_path , input_file , NULL);
        loadOK = enkf_node->forward_load(enkf_node->data , file  , ecl_sum , restart_block , report_step);
        free(file);
      } else
        loadOK = enkf_node->forward_load(enkf_node->data , run_path , ecl_sum , restart_block , report_step);
      
      util_safe_free( input_file );
    }
  }
  enkf_node->__node_id.report_step = report_step;
  enkf_node->__node_id.state       = FORECAST;
  enkf_node->__node_id.iens        = iens; 
  
  enkf_node->__modified            = false;
  return loadOK;
}


bool enkf_node_forward_load_vector(enkf_node_type *enkf_node , const char * run_path , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_block , int report_step1, int report_step2 , int iens ) {
  bool loadOK;
  FUNC_ASSERT(enkf_node->forward_load_vector);
  {
    loadOK = enkf_node->forward_load_vector(enkf_node->data , NULL  , ecl_sum , restart_block , report_step1 , report_step2);
  }
  // This is broken ....
  enkf_node->__node_id.report_step = report_step1;
  enkf_node->__node_id.state       = FORECAST;
  enkf_node->__node_id.iens        = iens; 
  
  enkf_node->__modified            = false;
  return loadOK;
}





void enkf_node_ecl_load_static(enkf_node_type * enkf_node , const ecl_kw_type * ecl_kw, int report_step, int iens) {
  ecl_static_kw_init(enkf_node_value_ptr(enkf_node) , ecl_kw);
  enkf_node->__node_id.report_step = report_step;
  enkf_node->__node_id.state       = FORECAST;
  enkf_node->__node_id.iens        = iens;
  enkf_node->__modified            = false;
}


/**
   This function compares the internal __report_step with the input
   report_step, and return true if they are equal. It is used in the
   calling scope to discard static nodes which are no longer in use.
*/



static bool enkf_node_store_buffer( enkf_node_type * enkf_node , enkf_fs_type * fs , int report_step , int iens , state_enum state) {
  FUNC_ASSERT(enkf_node->write_to_buffer);
  {
    bool data_written;
    buffer_type * buffer = buffer_alloc( 100 );
    const enkf_config_node_type * config_node = enkf_node_get_config( enkf_node ); 
    buffer_fwrite_time_t( buffer , time(NULL));
    data_written = enkf_node->write_to_buffer(enkf_node->data , buffer , report_step , state );
    if (data_written) {
      const char * node_key = enkf_config_node_get_key( config_node );
      enkf_var_type var_type = enkf_config_node_get_var_type( config_node );

      if (enkf_node->vector_storage)
        enkf_fs_fwrite_vector( fs , buffer , node_key , var_type , iens , state );
      else
        enkf_fs_fwrite_node( fs , buffer , node_key , var_type , report_step , iens , state );

    }
    buffer_free( buffer );
    return data_written;
  }
}

bool enkf_node_store_vector(enkf_node_type *enkf_node , enkf_fs_type * fs , int iens , state_enum state) {
  return enkf_node_store_buffer( enkf_node , fs , -1 , iens , state);
}



bool enkf_node_store(enkf_node_type * enkf_node , enkf_fs_type * fs , bool force_vectors , node_id_type node_id) {
  if (enkf_node->vector_storage) {
    if (force_vectors)
      return enkf_node_store_vector( enkf_node , fs , node_id.iens , node_id.state );
    else
      return false;
  } else {
    if (node_id.report_step == 0) {
      ert_impl_type impl_type = enkf_node_get_impl_type(enkf_node);
      if (impl_type == SUMMARY) 
        return false;             /* For report step == 0 the summary data is just garbage. */
    }

    {
      bool data_written = enkf_node_store_buffer( enkf_node , fs , node_id.report_step , node_id.iens , node_id.state );
      enkf_node->__node_id   = node_id;
      enkf_node->__modified  = false;
      return data_written;
    }
  }
}



/**
   This function will load a node from the filesystem if it is
   available; if not it will just return false.

   The state argument can be 'both' - in which case it will first try
   the analyzed, and then subsequently the forecast before giving up
   and returning false. If the function returns true with state ==
   'both' it is no way to determine which version was actually loaded.
*/

bool enkf_node_try_load(enkf_node_type *enkf_node , enkf_fs_type * fs , node_id_type node_id) {
  if (node_id.state == BOTH) {
    node_id_type local_id = node_id;
    local_id.state = ANALYZED;
    if (enkf_node_has_data( enkf_node , fs , local_id)) {
      enkf_node_load( enkf_node , fs , local_id );
      return true;
    } 
    
    local_id.state = FORECAST;
    if (enkf_node_has_data( enkf_node , fs , local_id)) {
      enkf_node_load( enkf_node , fs , local_id);
      return true;
    } else
      return false;
  } else {
    if (enkf_node_has_data( enkf_node , fs , node_id)) {
      enkf_node_load( enkf_node , fs , node_id);
      return true;
    } else
      return false;
  }
}


static void enkf_node_buffer_load( enkf_node_type * enkf_node , enkf_fs_type * fs , int report_step , int iens , state_enum state) {
  FUNC_ASSERT(enkf_node->read_from_buffer);
  {
    buffer_type * buffer                      = buffer_alloc( 100 );
    const enkf_config_node_type * config_node = enkf_node_get_config( enkf_node );
    const char * node_key                     = enkf_config_node_get_key( config_node );
    enkf_var_type var_type                    = enkf_config_node_get_var_type( config_node );
    
    if (enkf_node->vector_storage)
      enkf_fs_fread_vector( fs , buffer , node_key , var_type , iens , state);
    else
      enkf_fs_fread_node( fs , buffer , node_key , var_type , report_step , iens , state );
    
    buffer_fskip_time_t( buffer );
    enkf_node->read_from_buffer(enkf_node->data , buffer , report_step , state );
    buffer_free( buffer );
  }
}




static void enkf_node_load_vector( enkf_node_type * enkf_node , enkf_fs_type * fs , int iens , state_enum state) {
  if ((enkf_node->__load_state & state) &&
      (enkf_node->__node_id.iens == iens)) 
    return;
  else {
    enkf_node_buffer_load( enkf_node , fs , -1 , iens , state);
    enkf_node->__load_state   |= state;
    enkf_node->__node_id.iens  = iens;
  }
}



static void enkf_node_load_container( enkf_node_type * enkf_node , enkf_fs_type * fs , node_id_type node_id ) {
  for (int inode=0; inode < vector_get_size( enkf_node->container_nodes ); inode++) {
    enkf_node_type * child_node = vector_iget( enkf_node->container_nodes , inode );
    enkf_node_load( child_node , fs , node_id );
  }
}


void enkf_node_load(enkf_node_type * enkf_node , enkf_fs_type * fs , node_id_type node_id) {
  if (enkf_node_get_impl_type(enkf_node) == CONTAINER)
    enkf_node_load_container( enkf_node , fs , node_id );
  else {
    if (enkf_node->vector_storage)
      enkf_node_load_vector( enkf_node , fs , node_id.iens , node_id.state );
    else {
      if ((node_id.iens        == enkf_node->__node_id.iens) && 
          (node_id.state       == enkf_node->__node_id.state) && 
          (node_id.report_step == enkf_node->__node_id.report_step) && 
          (!enkf_node->__modified)) 
        return;  /* The in memory representation agrees with the buffer values */
      else {
        enkf_node_buffer_load( enkf_node , fs , node_id.report_step, node_id.iens , node_id.state );
        enkf_node->__node_id     = node_id;
        enkf_node->__modified    = false;
      }
    }
  }
}


bool enkf_node_try_load_vector(enkf_node_type *enkf_node , enkf_fs_type * fs , int iens , state_enum state) {
  if (enkf_config_node_has_vector( enkf_node->config , fs , iens, state)) {
    enkf_node_load_vector( enkf_node , fs , iens , state );
    return true;
  } else 
    return false;
}





/*
  In the case of nodes with vector storage this function
  will load the entire vector.
*/

enkf_node_type * enkf_node_load_alloc( const enkf_config_node_type * config_node , enkf_fs_type * fs , node_id_type node_id) {
  if (enkf_config_node_vector_storage( config_node )) {
    if (enkf_config_node_has_vector( config_node , fs , node_id.iens , node_id.state)) {
      enkf_node_type * node = enkf_node_alloc( config_node );
      enkf_node_load( node , fs , node_id );
      return node;
    } else {
      util_abort("%s: could not load vector:%s from iens:%d state:%d \n",__func__ , enkf_config_node_get_key( config_node ),
                 node_id.iens , node_id.state );
      return NULL;
    }
  } else {
    if (enkf_config_node_has_node( config_node , fs , node_id)) {
      enkf_node_type * node = enkf_node_alloc( config_node );
      enkf_node_load( node , fs , node_id );
      return node;
    } else {
      util_abort("%s: Could not load node: key:%s  iens:%d  report:%d  state:%d\n",
                 __func__ , 
                 enkf_config_node_get_key( config_node ) , 
                 node_id.iens , node_id.report_step , node_id.state );
      return NULL;
    }
  }
}



void enkf_node_copy(const enkf_config_node_type * config_node , 
                    enkf_fs_type * src_case, 
                    enkf_fs_type * target_case,
                    node_id_type src_id ,
                    node_id_type target_id) {
  
  enkf_node_type * enkf_node = enkf_node_load_alloc(config_node, src_case , src_id);
  
  
  /* Hack to ensure that size is set for the gen_data instances.
     This sneeks low level stuff into a high level scope. BAD. */
  {
    ert_impl_type impl_type = enkf_node_get_impl_type( enkf_node );
    if (impl_type == GEN_DATA) {
      /* Read the size at report_step_from */
      gen_data_type * gen_data = enkf_node_value_ptr( enkf_node );
      int size                 = gen_data_get_size( gen_data );
      
      /* Enforce the size at report_step_to */
      gen_data_assert_size( gen_data , size , target_id.report_step);
    }
  }

  enkf_node_store(enkf_node, target_case , true , target_id );
  enkf_node_free(enkf_node);
}

bool enkf_node_has_data( enkf_node_type * enkf_node , enkf_fs_type * fs , node_id_type node_id) {
  if (enkf_node->vector_storage) {
    FUNC_ASSERT(enkf_node->has_data);
    {
      int report_step  = node_id.report_step;
      int iens         = node_id.iens;
      state_enum state = node_id.state;

      if ((node_id.iens != enkf_node->__node_id.iens) || ((enkf_node->__load_state & state) == 0)) {
        // Try to load the vector.
        if (enkf_config_node_has_vector( enkf_node->config , fs , iens , state ))
          enkf_node_load_vector( enkf_node , fs , iens , state );
      }
      
      if ((node_id.iens == enkf_node->__node_id.iens) && (enkf_node->__load_state & state)) 
        // The vector is loaded. Check if we have the report_step/state asked for:
        return enkf_node->has_data( enkf_node->data , report_step , state );
      else
        return false; 
    }
  } else 
    return enkf_config_node_has_node( enkf_node->config , fs , node_id );
}


/**
   Copy an ensemble of nodes. Note that the limits are inclusive.
*/

void enkf_node_copy_ensemble(const enkf_config_node_type * config_node , 
                             enkf_fs_type * src_case , 
                             enkf_fs_type * target_case , 
                             int report_step_from, state_enum state_from,    /* src state */
                             int report_step_to  , state_enum state_to,      /* target state */
                             int ens_size, 
                             const int * permutations) {
  
  node_id_type src_id    = {.report_step = report_step_from , .iens = 0 , .state = state_from };
  node_id_type target_id = {.report_step = report_step_to   , .iens = 0 , .state = state_to   };
  
  for(int iens_from = 0; iens_from < ens_size; iens_from++) {
    int iens_to;
    if (permutations == NULL)
      iens_to = iens_from;
    else
      iens_to = permutations[iens_from];
    
    src_id.iens = iens_from;
    target_id.iens = iens_to;

    enkf_node_copy(config_node , src_case , target_case , src_id , target_id );
  }
}
  


enkf_node_type ** enkf_node_load_alloc_ensemble( const enkf_config_node_type * config_node , enkf_fs_type * fs , 
                                                 int report_step , int iens1 , int iens2 , state_enum state) {
  enkf_node_type ** ensemble = util_calloc( (iens2 - iens1) , sizeof * ensemble );
  for (int iens = iens1; iens < iens2; iens++) {
    node_id_type node_id = {.report_step = report_step , .iens = iens };
    state_enum load_state;
    ensemble[iens - iens1] = NULL;

    
    if (state == BOTH) {
      node_id.state = ANALYZED;
      if (enkf_config_node_has_node( config_node , fs , node_id))
        load_state = ANALYZED;
      else {
        node_id.state = FORECAST;
        if (enkf_config_node_has_node( config_node , fs , node_id))
          load_state = FORECAST;
        else
          load_state = UNDEFINED;
      }
    } else 
      load_state = state;

    node_id.state = load_state;
    if (load_state != UNDEFINED)
      ensemble[iens - iens1] = enkf_node_load_alloc(config_node , fs , node_id);
  }
  
  return ensemble;
}




void enkf_node_serialize(enkf_node_type *enkf_node , enkf_fs_type * fs, node_id_type node_id , 
                         const active_list_type * active_list , matrix_type * A , int row_offset , int column) {

  FUNC_ASSERT(enkf_node->serialize);
  enkf_node_load( enkf_node , fs , node_id);
  enkf_node->serialize(enkf_node->data , node_id , active_list , A , row_offset , column);
  
}



void enkf_node_deserialize(enkf_node_type *enkf_node , enkf_fs_type * fs , node_id_type node_id,
                           const active_list_type * active_list , const matrix_type * A , int row_offset , int column) {

  FUNC_ASSERT(enkf_node->deserialize);
  enkf_node->deserialize(enkf_node->data , node_id , active_list , A , row_offset , column);
  enkf_node->__modified = true;
  enkf_node_store( enkf_node , fs , true , node_id );
}



void enkf_node_set_inflation( enkf_node_type * inflation , const enkf_node_type * std , const enkf_node_type * min_std) {
  {
    enkf_node_type * enkf_node = inflation;
    FUNC_ASSERT(enkf_node->set_inflation);
  }
  inflation->set_inflation( inflation->data , std->data , min_std->data );
}


void enkf_node_sqrt(enkf_node_type *enkf_node) {
  FUNC_ASSERT(enkf_node->isqrt);
  enkf_node->isqrt(enkf_node->data);
  enkf_node->__modified = true;
}


void enkf_node_scale(enkf_node_type *enkf_node , double scale_factor) {
  FUNC_ASSERT(enkf_node->scale);
  enkf_node->scale(enkf_node->data , scale_factor);
  enkf_node->__modified = true;
}


void enkf_node_iadd(enkf_node_type *enkf_node , const enkf_node_type * delta_node) {
  FUNC_ASSERT(enkf_node->iadd);
  enkf_node->iadd(enkf_node->data , delta_node->data);
  enkf_node->__modified = true;
}


void enkf_node_iaddsqr(enkf_node_type *enkf_node , const enkf_node_type * delta_node) {
  FUNC_ASSERT(enkf_node->iaddsqr);
  enkf_node->iaddsqr(enkf_node->data , delta_node->data);
  enkf_node->__modified = true;
}


void enkf_node_imul(enkf_node_type *enkf_node , const enkf_node_type * delta_node) {
  FUNC_ASSERT(enkf_node->imul);
  enkf_node->imul(enkf_node->data , delta_node->data);
  enkf_node->__modified = true;
}



/**
   The return value is whether any initialization has actually taken
   place. If the function returns false it is for instance not
   necessary to internalize anything.
*/

bool enkf_node_initialize(enkf_node_type *enkf_node, int iens , rng_type * rng) {
  if (enkf_node->initialize != NULL) {
    char * init_file = enkf_config_node_alloc_initfile( enkf_node->config , iens );
    bool   init = enkf_node->initialize(enkf_node->data , iens , init_file, rng);
    if (init) {
      enkf_node->__node_id.report_step = 0;
      enkf_node->__node_id.state       = ANALYZED;
      enkf_node->__node_id.iens        = iens;
      enkf_node->__modified    = true;
    } 
    util_safe_free( init_file );
    return init;
  } else
    return false;  /* No init performed */
}


/**
   Only the STATIC keywords actually support this operation.
*/

void enkf_node_free_data(enkf_node_type * enkf_node) {
  FUNC_ASSERT(enkf_node->free_data);
  enkf_node->free_data(enkf_node->data);
  enkf_node->__modified         = true;
  enkf_node->__node_id.state            = UNDEFINED;
  enkf_node->__node_id.report_step      = -1;  
  enkf_node->__node_id.iens             = -1;
}



void enkf_node_clear(enkf_node_type *enkf_node) {
  FUNC_ASSERT(enkf_node->clear);
  enkf_node->clear(enkf_node->data);
}


void enkf_node_free(enkf_node_type *enkf_node) {
  if (enkf_node->freef != NULL)
    enkf_node->freef(enkf_node->data);
  free(enkf_node->node_key);
  free(enkf_node);
}


void enkf_node_free__(void *void_node) {
  enkf_node_free((enkf_node_type *) void_node);
}

const char * enkf_node_get_key(const enkf_node_type * enkf_node) {
  return enkf_node->node_key;
}


#undef FUNC_ASSERT



/*****************************************************************/

/**
   This function has been implemented to ensure/force a reload of
   nodes when the case has changed.
*/
   
void enkf_node_invalidate_cache( enkf_node_type * node ) {
  node->__modified            = true;
  node->__node_id.report_step = -1;
  node->__node_id.iens        = -1;
  node->__node_id.state       = UNDEFINED;
  node->__load_state          = false;
}


/* Manual inheritance - .... */
static enkf_node_type * enkf_node_alloc_empty(const enkf_config_node_type *config ) {
  const char *node_key     = enkf_config_node_get_key(config);
  ert_impl_type impl_type  = enkf_config_node_get_impl_type(config);
  enkf_node_type * node    = util_malloc(sizeof * node );
  node->vector_storage     = enkf_config_node_vector_storage( config );
  node->config             = config;
  node->node_key           = util_alloc_string_copy(node_key);
  node->data               = NULL;
  node->container_nodes    = vector_alloc_new( );
  enkf_node_invalidate_cache( node );
  
  /*
    Start by initializing all function pointers to NULL.
  */
  node->alloc                = NULL;
  node->ecl_write            = NULL;
  node->forward_load         = NULL;
  node->forward_load_vector  = NULL;
  node->copy                 = NULL;
  node->initialize           = NULL;
  node->freef                = NULL;
  node->free_data            = NULL;
  node->user_get             = NULL;
  node->user_get_vector      = NULL;
  node->fload                = NULL; 
  node->read_from_buffer     = NULL;
  node->write_to_buffer      = NULL;
  node->serialize            = NULL; 
  node->deserialize          = NULL;
  node->clear                = NULL;
  node->set_inflation        = NULL;
  node->has_data             = NULL;

  /* Math operations: */
  node->iadd                 = NULL;
  node->scale                = NULL;
  node->isqrt                = NULL;
  node->iaddsqr              = NULL;
  node->imul                 = NULL;

  switch (impl_type) {
  case(CONTAINER):
    node->alloc              = container_alloc__;
    node->freef              = container_free__;
    break;
  case(GEN_KW):
    node->alloc              = gen_kw_alloc__;
    node->ecl_write          = gen_kw_ecl_write__;
    node->copy               = gen_kw_copy__;
    node->initialize         = gen_kw_initialize__;
    node->freef              = gen_kw_free__;
    node->user_get           = gen_kw_user_get__; 
    node->write_to_buffer    = gen_kw_write_to_buffer__;
    node->read_from_buffer   = gen_kw_read_from_buffer__;
    node->serialize          = gen_kw_serialize__;
    node->deserialize        = gen_kw_deserialize__;
    node->clear              = gen_kw_clear__;
    node->iadd               = gen_kw_iadd__;
    node->scale              = gen_kw_scale__;
    node->iaddsqr            = gen_kw_iaddsqr__;
    node->imul               = gen_kw_imul__;
    node->isqrt              = gen_kw_isqrt__;
    node->set_inflation      = gen_kw_set_inflation__;
    node->fload              = gen_kw_fload__;
    break;
  case(SUMMARY):
    node->forward_load         = summary_forward_load__;
    node->forward_load_vector  = summary_forward_load_vector__;
    node->alloc                = summary_alloc__;
    node->copy                 = summary_copy__;
    node->freef                = summary_free__;
    node->user_get             = summary_user_get__; 
    node->user_get_vector      = summary_user_get_vector__;
    node->read_from_buffer     = summary_read_from_buffer__;
    node->write_to_buffer      = summary_write_to_buffer__;
    node->serialize            = summary_serialize__;
    node->deserialize          = summary_deserialize__;
    node->clear                = summary_clear__;
    node->has_data             = summary_has_data__;
    /*
      node->iadd               = summary_iadd__;
      node->scale              = summary_scale__;
      node->iaddsqr            = summary_iaddsqr__;
      node->imul               = summary_imul__;
      node->isqrt              = summary_isqrt__;
    */
    break;
  case(SURFACE):
    node->initialize         = surface_initialize__;
    node->ecl_write          = surface_ecl_write__; 
    node->alloc              = surface_alloc__;
    node->copy               = surface_copy__;
    node->freef              = surface_free__;
    node->user_get           = surface_user_get__; 
    node->read_from_buffer   = surface_read_from_buffer__;
    node->write_to_buffer    = surface_write_to_buffer__;
    node->serialize          = surface_serialize__;
    node->deserialize        = surface_deserialize__;
    node->clear              = surface_clear__;
    node->iadd               = surface_iadd__;
    node->scale              = surface_scale__;
    node->iaddsqr            = surface_iaddsqr__;
    node->imul               = surface_imul__;
    node->isqrt              = surface_isqrt__;
    break;
  case(FIELD):
    node->alloc              = field_alloc__;
    node->ecl_write          = field_ecl_write__; 
    node->forward_load       = field_forward_load__;  
    node->copy               = field_copy__;
    node->initialize         = field_initialize__;
    node->freef              = field_free__;
    node->user_get           = field_user_get__;
    node->read_from_buffer   = field_read_from_buffer__;
    node->write_to_buffer    = field_write_to_buffer__;
    node->serialize          = field_serialize__;
    node->deserialize        = field_deserialize__;

    node->clear              = field_clear__; 
    node->set_inflation      = field_set_inflation__;
    node->iadd               = field_iadd__;
    node->scale              = field_scale__;
    node->iaddsqr            = field_iaddsqr__;
    node->imul               = field_imul__; 
    node->isqrt              = field_isqrt__;
    node->fload              = field_fload__;
    break;
  case(STATIC):
    node->ecl_write          = ecl_static_kw_ecl_write__; 
    node->alloc              = ecl_static_kw_alloc__;
    node->copy               = ecl_static_kw_copy__;
    node->freef              = ecl_static_kw_free__;
    node->free_data          = ecl_static_kw_free_data__;
    node->read_from_buffer   = ecl_static_kw_read_from_buffer__;
    node->write_to_buffer    = ecl_static_kw_write_to_buffer__;
    break;
  case(GEN_DATA):
    node->alloc              = gen_data_alloc__;
    node->initialize         = gen_data_initialize__;
    node->copy               = gen_data_copy__;
    node->freef              = gen_data_free__;
    node->ecl_write          = gen_data_ecl_write__;
    node->forward_load       = gen_data_forward_load__;
    node->user_get           = gen_data_user_get__;
    node->read_from_buffer   = gen_data_read_from_buffer__;
    node->write_to_buffer    = gen_data_write_to_buffer__;
    node->serialize          = gen_data_serialize__;
    node->deserialize        = gen_data_deserialize__;
    node->set_inflation      = gen_data_set_inflation__;

    node->clear              = gen_data_clear__; 
    node->iadd               = gen_data_iadd__;
    node->scale              = gen_data_scale__;
    node->iaddsqr            = gen_data_iaddsqr__;
    node->imul               = gen_data_imul__;
    node->isqrt              = gen_data_isqrt__;
    node->fload              = gen_data_fload__;
    break;
  default:
    util_abort("%s: implementation type: %d unknown - all hell is loose - aborting \n",__func__ , impl_type);
  }
  return node;
}



#define CASE_SET(type , func) case(type): has_func = (func != NULL); break;
bool enkf_node_has_func(const enkf_node_type * node , node_function_type function_type) {
  bool has_func = false;
  switch (function_type) {
    CASE_SET(alloc_func                     , node->alloc);
    CASE_SET(ecl_write_func                 , node->ecl_write);
    CASE_SET(forward_load_func              , node->forward_load);
    CASE_SET(copy_func                      , node->copy);
    CASE_SET(initialize_func                , node->initialize);
    CASE_SET(free_func                      , node->freef);
  default:
    fprintf(stderr,"%s: node_function_identifier: %d not recognized - aborting \n",__func__ , function_type);
  }
  return has_func;
}
#undef CASE_SET



enkf_node_type * enkf_node_alloc(const enkf_config_node_type * config) {
  enkf_node_type * node    = enkf_node_alloc_empty(config);
  enkf_node_alloc_domain_object(node);
  return node;
}


static void enkf_node_container_add( enkf_node_type * node , const enkf_node_type * child_node ) {
  vector_append_ref( node->container_nodes , child_node );
}



enkf_node_type * enkf_node_container_alloc(const enkf_config_node_type * config, hash_type * node_hash) {
  enkf_node_type * container_node = enkf_node_alloc( config );
  {
    for (int i=0; i < enkf_config_node_container_size( config ); i++) {
      const enkf_config_node_type * child_config = enkf_config_node_container_iget( config , i );
      enkf_node_type * child_node = hash_get( node_hash , enkf_config_node_get_key( child_config ));
      
      enkf_node_container_add( container_node , child_node );
      container_add_node( enkf_node_value_ptr( container_node ) , enkf_node_value_ptr( child_node ));
    }
  }
  return container_node;
}


bool enkf_node_internalize(const enkf_node_type * node, int report_step) {
  return enkf_config_node_internalize( node->config , report_step );
}


/*****************************************************************/


ecl_write_ftype * enkf_node_get_func_pointer( const enkf_node_type * node ) {
  return node->ecl_write;
}
