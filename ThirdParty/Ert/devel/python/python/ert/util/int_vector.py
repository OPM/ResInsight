#  Copyright (C) 2014  Statoil ASA, Norway.
#   
#  The file 'int_vector.py' is part of ERT - Ensemble based Reservoir Tool.
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 

from ert.cwrap import CWrapper
from ert.util import UTIL_LIB, VectorTemplate


class IntVector(VectorTemplate):
    default_format       = "%d"

    def __init__(self, default_value=0, initial_size=0):
        super(IntVector, self).__init__(default_value, initial_size)

    @classmethod
    def active_list(cls , range_string):
        """
        Will create a IntVector instance with the values from @range_string.

        The range_string input should be of the type "1,3-5,9,17",
        i.e. integer values separated by commas, and dashes to
        represent ranges. If the input string contains ANY invalid
        characters the returned active list will be empty:

           "1,4-7,10"  =>  {1,4,5,6,7,10}
           "1,4-7,10X" =>  {}
        
        The empty list will evaluate to false
        """
        return cls.cNamespace().create_active_list(range_string)
        
    def count(self, value):
        """ @rtype: int """
        return IntVector.cNamespace().count_equal(self, value)

cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerObjectType("int_vector", IntVector)


IntVector.cNamespace().alloc               = cwrapper.prototype("c_void_p   int_vector_alloc( int , int )")
IntVector.cNamespace().alloc_copy          = cwrapper.prototype("int_vector_obj int_vector_alloc_copy( int_vector )")
IntVector.cNamespace().strided_copy        = cwrapper.prototype("int_vector_obj int_vector_alloc_strided_copy( int_vector , int , int , int)")
IntVector.cNamespace().free                = cwrapper.prototype("void   int_vector_free( int_vector )")
IntVector.cNamespace().iget                = cwrapper.prototype("int    int_vector_iget( int_vector , int )")
IntVector.cNamespace().safe_iget           = cwrapper.prototype("int    int_vector_safe_iget( int_vector , int )")
IntVector.cNamespace().iset                = cwrapper.prototype("int    int_vector_iset( int_vector , int , int)")
IntVector.cNamespace().size                = cwrapper.prototype("int    int_vector_size( int_vector )")
IntVector.cNamespace().append              = cwrapper.prototype("void   int_vector_append( int_vector , int )")
IntVector.cNamespace().idel_block          = cwrapper.prototype("void   int_vector_idel_block( int_vector , int , int )")
IntVector.cNamespace().pop                 = cwrapper.prototype("int    int_vector_pop( int_vector )")
IntVector.cNamespace().idel                = cwrapper.prototype("void   int_vector_idel( int_vector , int )")
IntVector.cNamespace().insert              = cwrapper.prototype("void   int_vector_insert( int_vector , int , int)")
IntVector.cNamespace().lshift              = cwrapper.prototype("void   int_vector_lshift( int_vector , int )")
IntVector.cNamespace().rshift              = cwrapper.prototype("void   int_vector_rshift( int_vector , int )")
IntVector.cNamespace().fprintf             = cwrapper.prototype("void   int_vector_fprintf( int_vector , FILE , char* , char*)")
IntVector.cNamespace().sort                = cwrapper.prototype("void   int_vector_sort( int_vector )")
IntVector.cNamespace().rsort               = cwrapper.prototype("void   int_vector_rsort( int_vector )")
IntVector.cNamespace().reset               = cwrapper.prototype("void   int_vector_reset( int_vector )")
IntVector.cNamespace().set_read_only       = cwrapper.prototype("void   int_vector_set_read_only( int_vector , bool )")
IntVector.cNamespace().get_read_only       = cwrapper.prototype("bool   int_vector_get_read_only( int_vector )")
IntVector.cNamespace().get_max             = cwrapper.prototype("int    int_vector_get_max( int_vector )")
IntVector.cNamespace().get_min             = cwrapper.prototype("int    int_vector_get_min( int_vector )")
IntVector.cNamespace().get_max_index       = cwrapper.prototype("int    int_vector_get_max_index( int_vector , bool)")
IntVector.cNamespace().get_min_index       = cwrapper.prototype("int    int_vector_get_min_index( int_vector , bool)")
IntVector.cNamespace().shift               = cwrapper.prototype("void   int_vector_shift( int_vector , int )")
IntVector.cNamespace().scale               = cwrapper.prototype("void   int_vector_scale( int_vector , int )")
IntVector.cNamespace().div                 = cwrapper.prototype("void   int_vector_div( int_vector , int )")
IntVector.cNamespace().inplace_add         = cwrapper.prototype("void   int_vector_inplace_add( int_vector , int_vector )")
IntVector.cNamespace().inplace_mul         = cwrapper.prototype("void   int_vector_inplace_mul( int_vector , int_vector )")
IntVector.cNamespace().assign              = cwrapper.prototype("void   int_vector_set_all( int_vector , int)")
IntVector.cNamespace().memcpy              = cwrapper.prototype("void   int_vector_memcpy(int_vector , int_vector )")
IntVector.cNamespace().set_default         = cwrapper.prototype("void   int_vector_set_default( int_vector , int)")
IntVector.cNamespace().get_default         = cwrapper.prototype("int    int_vector_get_default( int_vector )")
IntVector.cNamespace().element_size        = cwrapper.prototype("int    int_vector_element_size( int_vector )")
IntVector.cNamespace().create_active_list  = cwrapper.prototype("int_vector_obj string_util_alloc_active_list( char* )")

IntVector.cNamespace().permute          = cwrapper.prototype("void int_vector_permute(int_vector, permutation_vector)")
IntVector.cNamespace().sort_perm        = cwrapper.prototype("permutation_vector_obj int_vector_alloc_sort_perm(int_vector)")
IntVector.cNamespace().rsort_perm       = cwrapper.prototype("permutation_vector_obj int_vector_alloc_rsort_perm(int_vector)")
IntVector.cNamespace().contains       = cwrapper.prototype("bool int_vector_contains(int_vector, int)")
IntVector.cNamespace().select_unique       = cwrapper.prototype("void int_vector_select_unique(int_vector)")
IntVector.cNamespace().element_sum       = cwrapper.prototype("int int_vector_sum(int_vector)")
IntVector.cNamespace().get_data_ptr      = cwrapper.prototype("int* int_vector_get_ptr(int_vector)")
IntVector.cNamespace().count_equal       = cwrapper.prototype("int int_vector_count_equal(int_vector, int)")

