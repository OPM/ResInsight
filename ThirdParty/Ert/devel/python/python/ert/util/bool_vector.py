#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'vector_template.py' is part of ERT - Ensemble based Reservoir Tool.
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
import warnings


class BoolVector(VectorTemplate):
    default_format       = "%8d"

    def __init__(self, default_value=False, initial_size=0):
        super(BoolVector, self).__init__(default_value, initial_size)

    def count(self, value=True):
        """ @rtype: int """
        return BoolVector.cNamespace().count_equal(self, value)

    @classmethod
    def createActiveMask(cls, range_string):
        """
        Will create a BoolVector instance with the values from @range_string.

        The range_string input should be of the type "1,3-5,9,17",
        i.e. integer values separated by commas, and dashes to
        represent ranges. If the input string contains ANY invalid
        characters the returned active list will be empty:

           "1,4-7,10"  =>  {F,T,F,F,T,T,T,T,F,F,T}
           "1,4-7,10X" =>  {}
        
        The empty list will evaluate to false
        @rtype: BoolVector
        """
        return cls.cNamespace().create_active_mask(range_string)

    @classmethod
    def active_mask(cls, range_string):
        """
        Will create a BoolVector instance with the values from @range_string.

        The range_string input should be of the type "1,3-5,9,17",
        i.e. integer values separated by commas, and dashes to
        represent ranges. If the input string contains ANY invalid
        characters the returned active list will be empty:

           "1,4-7,10"  =>  {F,T,F,F,T,T,T,T,F,F,T}
           "1,4-7,10X" =>  {}

        The empty list will evaluate to false
        @rtype: BoolVector
        """
        warnings.warn("The active_mask(cls, rangs_string) method has been renamed: createActiveMask(cls, rangs_string)" , DeprecationWarning)
        return cls.cNamespace().create_active_mask(range_string)

    @classmethod
    def updateActiveMask(cls, range_string, bool_vector):
        """
        Updates a bool vector based on a range string.
        @type range_string: str
        @type bool_vector: BoolVector
        @rtype: bool
        """
        return cls.cNamespace().update_active_mask(range_string, bool_vector)

    @classmethod
    def createFromList(cls, size, source_list):
        """
        Allocates a bool vector from a Python list of indexes
        @rtype: BoolVector
        """
        bool_vector = BoolVector(False, size)

        for index in source_list:
            index = int(index)
            bool_vector[index] = True

        return bool_vector


    def createActiveList(self):
        """ @rtype: ert.util.IntVector """
        return BoolVector.cNamespace().active_list(self)


cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerObjectType("bool_vector", BoolVector)


BoolVector.cNamespace().alloc               = cwrapper.prototype("c_void_p   bool_vector_alloc( int , bool )")
BoolVector.cNamespace().alloc_copy          = cwrapper.prototype("bool_vector_obj bool_vector_alloc_copy( bool_vector )")
BoolVector.cNamespace().strided_copy        = cwrapper.prototype("bool_vector_obj bool_vector_alloc_strided_copy( bool_vector , int , int , int)")
BoolVector.cNamespace().free                = cwrapper.prototype("void   bool_vector_free( bool_vector )")
BoolVector.cNamespace().iget                = cwrapper.prototype("bool   bool_vector_iget( bool_vector , int )")
BoolVector.cNamespace().safe_iget           = cwrapper.prototype("bool   bool_vector_safe_iget( bool_vector , int )")
BoolVector.cNamespace().iset                = cwrapper.prototype("void   bool_vector_iset( bool_vector , int , bool)")
BoolVector.cNamespace().size                = cwrapper.prototype("int    bool_vector_size( bool_vector )")
BoolVector.cNamespace().append              = cwrapper.prototype("void   bool_vector_append( bool_vector , bool )")
BoolVector.cNamespace().idel_block          = cwrapper.prototype("void   bool_vector_idel_block( bool_vector , bool , bool )")
BoolVector.cNamespace().idel                = cwrapper.prototype("void   bool_vector_idel( bool_vector , int )")
BoolVector.cNamespace().pop                 = cwrapper.prototype("bool   bool_vector_pop( bool_vector )")
BoolVector.cNamespace().lshift              = cwrapper.prototype("void   bool_vector_lshift( bool_vector , int )")
BoolVector.cNamespace().rshift              = cwrapper.prototype("void   bool_vector_rshift( bool_vector , int )")
BoolVector.cNamespace().insert              = cwrapper.prototype("void   bool_vector_insert( bool_vector , int , bool)")
BoolVector.cNamespace().fprintf             = cwrapper.prototype("void   bool_vector_fprintf( bool_vector , FILE , char* , char*)")
BoolVector.cNamespace().sort                = cwrapper.prototype("void   bool_vector_sort( bool_vector )")
BoolVector.cNamespace().rsort               = cwrapper.prototype("void   bool_vector_rsort( bool_vector )")
BoolVector.cNamespace().reset               = cwrapper.prototype("void   bool_vector_reset( bool_vector )")
BoolVector.cNamespace().set_read_only       = cwrapper.prototype("void   bool_vector_set_read_only( bool_vector , bool )")
BoolVector.cNamespace().get_read_only       = cwrapper.prototype("bool   bool_vector_get_read_only( bool_vector )")
BoolVector.cNamespace().get_max             = cwrapper.prototype("bool   bool_vector_get_max( bool_vector )")
BoolVector.cNamespace().get_min             = cwrapper.prototype("bool   bool_vector_get_min( bool_vector )")
BoolVector.cNamespace().get_max_index       = cwrapper.prototype("int    bool_vector_get_max_index( bool_vector , bool)")
BoolVector.cNamespace().get_min_index       = cwrapper.prototype("int    bool_vector_get_min_index( bool_vector , bool)")
BoolVector.cNamespace().shift               = cwrapper.prototype("void   bool_vector_shift( bool_vector , bool )")
BoolVector.cNamespace().scale               = cwrapper.prototype("void   bool_vector_scale( bool_vector , bool )")
BoolVector.cNamespace().div                 = cwrapper.prototype("void   bool_vector_div( bool_vector , bool )")
BoolVector.cNamespace().inplace_add         = cwrapper.prototype("void   bool_vector_inplace_add( bool_vector , bool_vector )")
BoolVector.cNamespace().inplace_mul         = cwrapper.prototype("void   bool_vector_inplace_mul( bool_vector , bool_vector )")
BoolVector.cNamespace().assign              = cwrapper.prototype("void   bool_vector_set_all( bool_vector , bool)")
BoolVector.cNamespace().memcpy              = cwrapper.prototype("void   bool_vector_memcpy(bool_vector , bool_vector )")
BoolVector.cNamespace().set_default         = cwrapper.prototype("void   bool_vector_set_default( bool_vector , bool)")
BoolVector.cNamespace().get_default         = cwrapper.prototype("bool   bool_vector_get_default( bool_vector )")
BoolVector.cNamespace().element_size        = cwrapper.prototype("int    bool_vector_element_size( bool_vector )")

BoolVector.cNamespace().permute          = cwrapper.prototype("void bool_vector_permute(bool_vector, permutation_vector)")
BoolVector.cNamespace().sort_perm        = cwrapper.prototype("permutation_vector_obj bool_vector_alloc_sort_perm(bool_vector)")
BoolVector.cNamespace().rsort_perm       = cwrapper.prototype("permutation_vector_obj bool_vector_alloc_rsort_perm(bool_vector)")

BoolVector.cNamespace().create_active_mask = cwrapper.prototype("bool_vector_obj string_util_alloc_active_mask( char* )")
BoolVector.cNamespace().update_active_mask = cwrapper.prototype("bool string_util_update_active_mask(char*, bool_vector)")
BoolVector.cNamespace().active_list        = cwrapper.prototype("int_vector_obj bool_vector_alloc_active_list(bool_vector)")
BoolVector.cNamespace().contains           = cwrapper.prototype("bool bool_vector_contains(bool_vector, bool)")
BoolVector.cNamespace().select_unique          = cwrapper.prototype("void bool_vector_select_unique(bool_vector)")
BoolVector.cNamespace().element_sum       = cwrapper.prototype("bool bool_vector_sum(bool_vector)")
BoolVector.cNamespace().get_data_ptr      = cwrapper.prototype("bool* bool_vector_get_ptr(bool_vector)")
BoolVector.cNamespace().count_equal       = cwrapper.prototype("int bool_vector_count_equal(bool_vector, bool)")
