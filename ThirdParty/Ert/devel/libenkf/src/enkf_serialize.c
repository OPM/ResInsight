/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_serialize.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <stdlib.h>

#include <ert/util/util.h>

#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/active_list.h>

/** This is heavy shit ... */


/**
   This file handles serialization and deserialization of the
   enkf_nodes. This is at the very core of the EnKF update
   algorithm. The final update step is written:

       A' = XA

   i.e. it is linear algeabra, and we need(?) to write the various
   objects in the form of an ensemble matrix, this is the process we
   call serialization. Then the linear algebra update is performed,
   and afterwards we must read the data back from the ensemble matrix
   to the enkf_node object, this is deserialization.


                                         
   
  ===============                                    ===============
  | PORO-1      |                                    | PORO-2      |
  |--------------                    Member 2        |--------------   
  |             |                    ========        |             |       
  | P0 ... P10  |        Member 1       |            | P0 ... P10  |
  ===============        ========       |            ===============
       /|\                   |      -----                 /|\ 
        |                    |      |                      |
        |                   \|/    \|/                     |
        |                                                  | 
        |                  [ P0     P0 ]                   |
        |                  [ P1     P1 ]                   |
        ------------------>[ P2     P2 ]<-------------------
                           [ P3     P3 ]
                           [ P4     P4 ]    
                           [ ··········]             ==============     
                           [ R1     R1 ]             | RELPERM-2  |
  ==============    ------>[ R2     R2 ]<----------->|------------|
  | RELPERM-1  |    |      [ R3     R3 ]             |            |
  |------------|<----      [ ··········]             | R0 ... R5  |
  |            |           [ F2     F2 ]             ==============
  | R0 ... R5  |      ---->[ F3     F3 ]
  ==============      |    [ F4     F4 ]<-----
                      |    [ F6     F6 ]     |
                      |                      |
                      |                      |           ==============
                      |                      |           | FAULT-1    |
                      |                      ----------->|------------|
  ==============      |                                  |            |
  | FAULT-1    |      |                                  | F0 ... F6  |
  |------------|<------                                  ==============
  |            |
  | F0 ... F6  |
  ==============



This figure shows the following:

 1. Three different nodes called PORO, RELPERM and FAULT
    respectively. The PORO node consists of eleven elements (P0
    ... P10), whereas the RELPERM and FAULT nodes contain six and
    seven elements.

 2. The ensemble consists of two members (i.e. there is PORO-1 and
    PORO-2.).

 3. The members have been serialized into a a large vector where
    everything comes ordered. Observe that *NOT* all elements from the
    members have been inserted into the large vector, i.e. for the
    PORO fields we only have elements P0 .. P4; this is because (for
    some reason) not all elements were active.


Each of the enkf_node functions have their own xxx_serialize and
xxx_deserialize functions, however these functions SHOULD call the
enkf_serialize() and enkf_deserialize() functions in this
file. (Rolling your own serialize / deserialize functions at the
lowest level is a SERIOUS CRIME.)

The illustration above shows three different enkf_node objects which
have been COMPLETELY serialized. One of the reasons the code is so
complex is that it is supposed to handle situations where the serial
vector is to small to hold everything, and repeated calls to serialize
& deserialize must be performed to complete the thing.


About stride
============
In the enkf update the ensemble matrix A is just that - a matrix,
however in this elegant high-level language it is of course
implemented as one long linear vector. The matrix is implemented such
that the 'member-direction' is fastest running index.  Consider the
following ensemble matrix, consisting of five ensemble members:



             

                           Member 5
      Member 2 --·           |
                 |           |
                 |           |
Member 1 ----·   |           |
             |   |           |
            \|/ \|/         \|/
           [ P0  P0  P0  P0  P0 ]
           [ P1  P1  P1  P1  P1 ]
           [ P2  P2  P2  P2  P2 ]
           [ P3  P3  P3  P3  P3 ]
           [ R0  R0  R0  R0  R0 ]
       A = [ R1  R1  R1  R1  R1 ]
           [ R2  R2  R2  R2  R2 ]
           [ F0  F0  F0  F0  F0 ]
           [ F1  F1  F1  F1  F1 ]
           [ F2  F2  F2  F2  F2 ]
           [ F3  F3  F3  F3  F3 ]
        

The in memory the matrix will look like this:

                                     
                                      Member 2
                                          | 
                            ______________|______________  
                           /              |              \  
                           |              |              |
             ______________|______________|______________|______________
            /              |              |              |              \
            |              |              |              |              |
            |              |              |              |              | 
           \|/            \|/            \|/            \|/            \|/         ...........    
   A =[ P0 P0 P0 P0 P0 P1 P1 P1 P1 P1 P2 P2 P2 P2 P2 P3 P3 P3 P3 P3 R0 R0 R0 R0 R0 R1 R1 R1 R1 R1 R2 R2 R2 R2 R2 F0 F0 F0 F0 F0 F1 F1 F1 F1 F1 F2 F2 F2 F2 F2 F3 F3 F3 F3 F3 ...]
       /|\    X1      /|\    X2      /|\            /|\            /|\              ........
        |              |              |              |              |
        |              |              |              |              |
        \______________|______________|______________|______________/
                       |              |              | 
                       |              |              |
                       \______________|______________/
                                      |
                                      | 
                                  Member 1

The stride in the serial_vector_type object is the number of elements
between consecutive elements in the same member, i.e. it is five in
the vector above. (Starting at e.g. P0 for member three (marked with
X1 in the figure), P1 for the same member is five elements down in the
vector (marked with X2 above)). Now - that was clear ehhh?

*****************************************************************/



/* 
   It will be very costly to make it thread-safe if we manipulate the
   shape of the A matrix from here.
*/
   

void enkf_matrix_serialize(const void * __node_data               , 
                           int node_size                          ,      
                           ecl_type_enum node_type                ,           
                           const active_list_type * __active_list , 
                           matrix_type * A                        ,
                           int row_offset,
                           int column) {
  
  int active_size;
  const int   * active_list    = active_list_get_active( __active_list ); 
  active_size = active_list_get_active_size( __active_list , node_size);

  if (node_type == ECL_DOUBLE_TYPE) {
    const double * node_data = (const double *) __node_data;
    if (active_size == node_size) /** All elements active */
      matrix_set_many_on_column( A , row_offset , node_size , node_data , column);
    else {
      int row_index;
      int node_index;
      for (row_index = 0; row_index < active_size; row_index++) {
        node_index = active_list[ row_index ];
        matrix_iset( A , row_index + row_offset , column , node_data[node_index] );
      }
    }
  } else if (node_type == ECL_FLOAT_TYPE) {
    const float * node_data = (const float *) __node_data;
    int row_index;
    if (active_size == node_size) {/** All elements active */
      for (row_index = 0; row_index < node_size; row_index++)
        matrix_iset( A , row_index + row_offset , column , node_data[ row_index ]);  /* Must have float -> double conversion; can not use memcpy() based approach */
    } else {
      int row_index;
      int node_index;
      for (row_index = 0; row_index < active_size; row_index++) {
        node_index = active_list[ row_index ];
        matrix_iset( A , row_index + row_offset , column , node_data[node_index] );
      }
    }      
  } else 
      util_abort("%s: internal error: trying to serialize unserializable type:%s \n",__func__ , ecl_util_get_type_name( node_type ));
}


void enkf_matrix_deserialize(void * __node_data                 , 
                             int node_size                      ,      
                             ecl_type_enum node_type            ,           
                             const active_list_type * __active_list , 
                             const matrix_type * A,
                             int row_offset,
                             int column) {
  
  int active_size;
  const int   * active_list    = active_list_get_active( __active_list ); 
  active_size = active_list_get_active_size( __active_list , node_size );
    
  if (node_type == ECL_DOUBLE_TYPE) {
    double * node_data = (double *) __node_data;

    if (active_size == node_size) { /** All elements active */
      int row_index;
      for (row_index = 0; row_index < active_size; row_index++) 
        node_data[row_index] = matrix_iget( A , row_index + row_offset , column);
    } else {
      int row_index;
      int node_index;
      for (row_index = 0; row_index < active_size; row_index++) {
        node_index = active_list[ row_index ];
        node_data[node_index] = matrix_iget( A , row_index + row_offset , column);
      }
    }
    
  } else if (node_type == ECL_FLOAT_TYPE) {
    float * node_data = (float *) __node_data;
    
    if (active_size == node_size) { /** All elements active */
      int row_index;
      for (row_index = 0; row_index < active_size; row_index++) 
        node_data[row_index] = matrix_iget( A , row_index + row_offset , column);
    } else {
      int row_index;
      int node_index;
      for (row_index = 0; row_index < active_size; row_index++) {
        node_index = active_list[ row_index ];
        node_data[node_index] = matrix_iget( A , row_index + row_offset , column);
      }
    }
  } else 
    util_abort("%s: internal error: trying to serialize unserializable type:%s \n",__func__ , ecl_util_get_type_name( node_type ));
}
                           
