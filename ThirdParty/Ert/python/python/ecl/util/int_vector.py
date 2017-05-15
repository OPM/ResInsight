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

from ecl.util import VectorTemplate, UtilPrototype


class IntVector(VectorTemplate):
    default_format       = "%d"

    _alloc               = UtilPrototype("void*  int_vector_alloc( int , int )" , bind = False)
    _create_active_list  = UtilPrototype("int_vector_obj string_util_alloc_active_list( char*)" , bind = False)
    _create_value_list   = UtilPrototype("int_vector_obj string_util_alloc_value_list( char*)" , bind = False)
    _alloc_copy          = UtilPrototype("int_vector_obj int_vector_alloc_copy( int_vector )")
    _strided_copy        = UtilPrototype("int_vector_obj int_vector_alloc_strided_copy( int_vector , int , int , int)")
    _free                = UtilPrototype("void   int_vector_free( int_vector )")
    _iget                = UtilPrototype("int    int_vector_iget( int_vector , int )")
    _safe_iget           = UtilPrototype("int    int_vector_safe_iget( int_vector , int )")
    _iset                = UtilPrototype("int    int_vector_iset( int_vector , int , int)")
    _size                = UtilPrototype("int    int_vector_size( int_vector )")
    _append              = UtilPrototype("void   int_vector_append( int_vector , int )")
    _idel_block          = UtilPrototype("void   int_vector_idel_block( int_vector , int , int )")
    _pop                 = UtilPrototype("int    int_vector_pop( int_vector )")
    _idel                = UtilPrototype("void   int_vector_idel( int_vector , int )")
    _insert              = UtilPrototype("void   int_vector_insert( int_vector , int , int)")
    _lshift              = UtilPrototype("void   int_vector_lshift( int_vector , int )")
    _rshift              = UtilPrototype("void   int_vector_rshift( int_vector , int )")
    _fprintf             = UtilPrototype("void   int_vector_fprintf( int_vector , FILE , char* , char*)")
    _sort                = UtilPrototype("void   int_vector_sort( int_vector )")
    _rsort               = UtilPrototype("void   int_vector_rsort( int_vector )")
    _reset               = UtilPrototype("void   int_vector_reset( int_vector )")
    _set_read_only       = UtilPrototype("void   int_vector_set_read_only( int_vector , bool )")
    _get_read_only       = UtilPrototype("bool   int_vector_get_read_only( int_vector )")
    _get_max             = UtilPrototype("int    int_vector_get_max( int_vector )")
    _get_min             = UtilPrototype("int    int_vector_get_min( int_vector )")
    _get_max_index       = UtilPrototype("int    int_vector_get_max_index( int_vector , bool)")
    _get_min_index       = UtilPrototype("int    int_vector_get_min_index( int_vector , bool)")
    _shift               = UtilPrototype("void   int_vector_shift( int_vector , int )")
    _scale               = UtilPrototype("void   int_vector_scale( int_vector , int )")
    _div                 = UtilPrototype("void   int_vector_div( int_vector , int )")
    _inplace_add         = UtilPrototype("void   int_vector_inplace_add( int_vector , int_vector )")
    _inplace_mul         = UtilPrototype("void   int_vector_inplace_mul( int_vector , int_vector )")
    _assign              = UtilPrototype("void   int_vector_set_all( int_vector , int)")
    _memcpy              = UtilPrototype("void   int_vector_memcpy(int_vector , int_vector )")
    _set_default         = UtilPrototype("void   int_vector_set_default( int_vector , int)")
    _get_default         = UtilPrototype("int    int_vector_get_default( int_vector )")
    _element_size        = UtilPrototype("int    int_vector_element_size( int_vector )")

    _permute             = UtilPrototype("void int_vector_permute(int_vector, permutation_vector)")
    _sort_perm           = UtilPrototype("permutation_vector_obj int_vector_alloc_sort_perm(int_vector)")
    _rsort_perm          = UtilPrototype("permutation_vector_obj int_vector_alloc_rsort_perm(int_vector)")
    _contains            = UtilPrototype("bool int_vector_contains(int_vector, int)")
    _select_unique       = UtilPrototype("void int_vector_select_unique(int_vector)")
    _element_sum         = UtilPrototype("int int_vector_sum(int_vector)")
    _get_data_ptr        = UtilPrototype("int* int_vector_get_ptr(int_vector)")
    _count_equal         = UtilPrototype("int int_vector_count_equal(int_vector, int)")
    _init_range          = UtilPrototype("void int_vector_init_range(int_vector, int , int , int)")

    def __init__(self, default_value=0, initial_size=0):
        super(IntVector, self).__init__(default_value, initial_size)

    @classmethod
    def active_list(cls, range_string):
        """Will create a IntVector instance with the values from @range_string.

        The range_string input should be of the type "1,3-5,9,17",
        i.e. integer values separated by commas, and dashes to
        represent ranges. If the input string contains ANY invalid
        characters the returned active list will be empty:

           "1,4-7,10"  =>  {1,4,5,6,7,10}
           "1,4-7,10X" =>  {}
        
        The empty list will evaluate to false. The values in the input
        string are meant to indicate "active values", i.e. the output
        values are sorted and repeated values are only counted once:
        
           "1,1,7,2" => {1,2,7}
        
        """
        return cls._create_active_list(range_string)

    @classmethod
    def valueList(cls , range_string):
        """Will create a IntVecter of all the values in the @range_string.

        Will not sort the values, and not uniquiefy - in contrast to
        the active_list() method.

        """
        return cls._create_value_list(range_string)
    
    
    def count(self, value):
        """ @rtype: int """
        return self._count_equal(value)
