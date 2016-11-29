/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'README.new_type.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

/**
The two files README.new_type.c and README.new_type_config.c (along
with the corresponding header files) are meant to serve as a
documentation and reference on how to add new object types to the enkf
system.

The enkf object system is based on the enkf_node_type as a sort of
abstract class, this object has a void pointer which will point to the
actual data instance (i.e. for instance an instance of the type
"new_type" implemented here), along with several function pointers to
manipulate the data object. 

   _______________________________________                 _______________________________
  | enkf_node_type instance               |               |                               |
  | ------------------------------------- |               | Actual data , e.g. a field or |
  | * void * data                         |-------------->| a multiplier.                 |
  | * Function pointers to manipulate     |               |_______________________________|
  |   data                                |    
  | * Something more - not relevant here  |
  |_______________________________________|


The enkf_node object can contain pointers to all types of objects.







new_type.c (should read README.new_type.h first).
==========
For all the enkf objects we have split the member spesific data, and
the configuration information in two different files (objects):

                        ______________
                       |              |
      _________________| Config       |_______________
     |                 | information  |               |
     |                /|______________|\              |
     |               /        |         \             |
     |              /         |          \            |
     |             /          |           |           |
     |            |           |           |           |
    _|__        __|_         _|__        _|__        _|__  
   |    |      |    |       |    |      |    |      |    | 
   | 01 |      | 02 |       | 03 |      | 04 |      | 05 | 
   |____|      |____|       |____|      |____|      |____| 


The figure shows an ensemble of 5 members (of some type); they all
have a pointer to a common configuration object. The thought behind
this is that all members should share configuration information,
i.e. for instance all the members in a permeability field should have
the same active/inactive cells, all the relperm instances should use
the same relperm model e.t.c. This file is about implementing the data
members, i.e. the small boxes. The implementation of the config
object, is discussed in README.new_type_config.c.


1.  alloc_ftype: new_type * new_type_alloc(const new_type_config *);
    --------------------------------------------

    This function takes a pointer to a config object of the right type
    (i.e. new_type_config_type in this case), reads the required
    configuration information from this object, and returns a new new_type
    instance.


2.  ecl_write_ftype: void new_type_ecl_write(const new_type * , const char *);
    --------------------------------------------------------------------------
    This function takes a const pointer to a new_type instance, along
    with a filename. The function should write eclipse data to the
    filename specified in the second argument. 



3.  fread_ftype: void new_type_fread(new_type * , FILE * );
    ------------------------------------------------
    This function should take a pointer to a new_type object, and a
    stream opened for reading as input, and then read in data for the
    object from disk.


4.  fwrite_ftype: void new_type_fwrite(new_type * , FILE * );
    ------------------------------------------------
    This function should take a pointer to a new_type object, and a
    stream opened for writing as input, and then write the data from
    the object to disk. The two functions instances fread_ftype - and
    fwrite_ftype must of course match, but apart from that they are
    quite free.


5.  copyc_ftype: new_type * new_type_copyc(const new_type *);
    ---------------------------------------------------------------
    This function takes a const pointer to a new_type instance. A new
    new_type copy instance with identical data is created. The two
    instance share config object.


6.  initialize_ftype: void new_type_initialize(new_type * , int);
    ------------------------------------------------------------
    This function takes a pointer to a new_type instance, and a an
    ensemble number as input; it should then initialize the data of
    the new_type object - this can either be done by sampling a random
    number according to some distribution, by reading in input from an
    external program or by calling an external program (i.e. RMS).


7.  serialize_ftype: int (const new_type * , int , size_t , double * , size_t , size_t , bool *);
    --------------------------------------------------------------------------------------------
    This one is a bit tricky ... The enkf_state object, holding among
    others things instances of new_type * can be illustrated as this:


    --------------
    | enkf_state |
    ==============         ---------------- 
    | PRESSURE   |-------->| Pressure data|     
    |------------|         ---------------- 
    | New_type   |---|
    --------------   |         -----------------
    | multz      |   |-------->| new_type data |
    --------------             -----------------  
       |  
       |       -------------- 
       |------>| multz data |
               --------------  

    The key point of this figure is that the various data we want to
    update with enkf are at random locations in memory. To be able to
    do the
               A' = AX
 
    matrix multiplication we must assemble this data in a long
    vector. That is what is meant by serializing. The serialize
    routine is so complicated for (at least) two reasons:

     o We can allocate a certain amount of memory, and then the
       serialize routines should continue until the available memory
       is exhausted, update and the resume.

     o For efficiency reasons a "funny" stride should be used.



8.  deserialize_ftype: int new_type_deserialize(new_type * , int , size_t , const double * , size_t , size_t);
    ---------------------------------------------------------------------------------------------------------
    This is "just" the opposite of serialize, take data back after the
    update (matrix multiplication).


9.  free_ftype: void new_type_free(new_type *);
    -------------------------------------------
    This function should free all the memory used by the new_type
    instance. Observe that the config pointer should be left alone,
    that will be collected elsewhere.

10. free_data_ftype: void new_type_free_data(new_type *);
    -----------------------------------------------------
    This function should free the data of the new_type instance, but
    retain the new_type holding structure. This function is called
    after an instance has swapped to disk.

11. realloc_data_ftype: void (new_type *);
    --------------------------------------------------------------
    This function is the "opposite" of function number 10, i.e. it
    is to reallocate memory to hold the actual data.


12. ensemble_fprintf_results_ftype * fprintf_results 
    --------------------------------------------------------------
    This function is used to print the reuslts in a formatted nice way. Only 
    applicable for variables with small amounts of data, like gen_kw.

13. ecl_load_ftype: void (new_type * , const char * run_path , const char * eclbase, const ecl_sum_type , int report_step)
    -----------------------------------------------------------------
    This function is used to load ECLIPSE results from a complete
    forward simulation. Observe that the restart data are not loaded
    through this interface, and that the summary datka get an
    ecl_sum_type * instance for convenience. If you want to add a new
    which should be loaded from a eclipse directory, you must manage
    with the run_path ankd ecl_base input.


Now - since the enkf_node objects can point to arbitrary types of
object the data pointer is a void pointer, and the function pointer
expect (void *) as input, instead of pointers to e.g. new_type
instances. To cast from the typed functions listed above, to functions
accepting (void *) there are several utility functions, i.e. the macro
VOID_ECL_WRITE() will create a void * version of the XXX_ecl_write()
function:

The macro call: VOID_ECL_WRITE(new_type) will generate the following code:

   void new_type_ecl_write__(void *__new_type) {
        new_type_ecl_write( (new_type *) __new_type);
   }

And the macro VOID_ECL_WRITE_HEADER(new_type) will generate the
corresponding header. These (void) functions are the ones used when
initializing the function pointers in the enkf_node object. For
instance for registering the new_type object in the enkf_node
implementation (function: enkf_node_alloc_empty)

....
....
case(NEW_TYPE):
   node->alloc    = new_type_alloc__;
   node->fwrite_f = new_type_fwrite__;
   ...

*/



/*
OK - here comes the implementation:
*/

/*
  These are standard header files needed by almost any C-program:
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/*
  Lots of small utility functions for: memory handling, string
  handling and file handling. Implemented in the libutil library.
*/
#include <util.h>


/*
  Lots of (ugly) C-macros to generate anonymous versions of the various
  functions listed above.
*/
#include <enkf_macros.h>


/*
  This file contains several enum typedefs, the most important one in
  this context is the ert_impl_type; the NEW_TYPE should be added to
  this (at the end).
*/
#include <enkf_types.h>


/*
  The header file for the actual new_type object.
*/
#include <new_type.h>



/*
  These two #define statements, along with the macros found in
  enkf_debug.h allow for some simple run_time checks of the various
  casts. Observe that the header file enkf_debug.h is included from
  the current directory; it is *not* installed in an include/
  directory.
*/
#define  TARGET_TYPE NEW_TYPE   /* The variable type in this file - according to the ert_impl_type classification */
#define  DEBUG                  /* We want debugging */  
#include "enkf_debug.h"



/* 
   Here comes the new_type_struct defintion: 
*/

struct new_type_struct {
  DEBUG_DECLARE
  const new_type_config_type *config;
  
  1: scalar_type    *scalar;
  2: double         *data;
};

/*
  Observe the following properties of the struct:

  1. The statement DEBUG_DECLARE inserts a variable __impl_type as the
     first element of the struct (if debugging is enabled) with
     #define DBEUG. This should be the first element in the struct.

  2. The struct contains a pointer to a new_type_config_type object;
     this pointer is const, as the new_type instances are not allowed
     to write the configuration information (it is shared among many
     instances), only read it.

  3. The actual data is stored in either a scalar instance, or just as
     double * data. (1: and 2: is *NOT* some new funny C-syntax).

     The scalar_type object is described further down. If use of
     scalar_type is not appropriate, you must store the data in some
     other way. There are no formal rules to this, but if you stick to
     the suggested "double *data;" convention suggested above, you
     will get several macros for algebraic manipulation (adding,
     scaling, squaring +++) of new_type instances for free. 

     Hence - it is *strongly* recommended to use either:

       scalar_type * scalar;
  
       or
 
       double * data;
      
     to hold the actual data.
*/


/*
scalar_type
===========
This type is implemented in scalar.c / scalar_config.c.
*/




