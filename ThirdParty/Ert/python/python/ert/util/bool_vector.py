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

from ert.util import VectorTemplate, UtilPrototype


class BoolVector(VectorTemplate):
    default_format       = "%8d"

    _alloc               = UtilPrototype("void*  bool_vector_alloc( int , bool )" , bind = False)
    _create_active_mask  = UtilPrototype("bool_vector_obj string_util_alloc_active_mask( char* )" , bind = False)
    _active_list         = UtilPrototype("int_vector_obj bool_vector_alloc_active_list(bool_vector)", bind = False)
    _alloc_copy          = UtilPrototype("bool_vector_obj bool_vector_alloc_copy( bool_vector )")
    _update_active_mask  = UtilPrototype("bool string_util_update_active_mask(char*, bool_vector)" , bind = False)

    _strided_copy        = UtilPrototype("bool_vector_obj bool_vector_alloc_strided_copy( bool_vector , int , int , int)")
    _free                = UtilPrototype("void   bool_vector_free( bool_vector )")
    _iget                = UtilPrototype("bool   bool_vector_iget( bool_vector , int )")
    _safe_iget           = UtilPrototype("bool   bool_vector_safe_iget( bool_vector , int )")
    _iset                = UtilPrototype("void   bool_vector_iset( bool_vector , int , bool)")
    _size                = UtilPrototype("int    bool_vector_size( bool_vector )")
    _append              = UtilPrototype("void   bool_vector_append( bool_vector , bool )")
    _idel_block          = UtilPrototype("void   bool_vector_idel_block( bool_vector , bool , bool )")
    _idel                = UtilPrototype("void   bool_vector_idel( bool_vector , int )")
    _pop                 = UtilPrototype("bool   bool_vector_pop( bool_vector )")
    _lshift              = UtilPrototype("void   bool_vector_lshift( bool_vector , int )")
    _rshift              = UtilPrototype("void   bool_vector_rshift( bool_vector , int )")
    _insert              = UtilPrototype("void   bool_vector_insert( bool_vector , int , bool)")
    _fprintf             = UtilPrototype("void   bool_vector_fprintf( bool_vector , FILE , char* , char*)")
    _sort                = UtilPrototype("void   bool_vector_sort( bool_vector )")
    _rsort               = UtilPrototype("void   bool_vector_rsort( bool_vector )")
    _reset               = UtilPrototype("void   bool_vector_reset( bool_vector )")
    _set_read_only       = UtilPrototype("void   bool_vector_set_read_only( bool_vector , bool )")
    _get_read_only       = UtilPrototype("bool   bool_vector_get_read_only( bool_vector )")
    _get_max             = UtilPrototype("bool   bool_vector_get_max( bool_vector )")
    _get_min             = UtilPrototype("bool   bool_vector_get_min( bool_vector )")
    _get_max_index       = UtilPrototype("int    bool_vector_get_max_index( bool_vector , bool)")
    _get_min_index       = UtilPrototype("int    bool_vector_get_min_index( bool_vector , bool)")
    _shift               = UtilPrototype("void   bool_vector_shift( bool_vector , bool )")
    _scale               = UtilPrototype("void   bool_vector_scale( bool_vector , bool )")
    _div                 = UtilPrototype("void   bool_vector_div( bool_vector , bool )")
    _inplace_add         = UtilPrototype("void   bool_vector_inplace_add( bool_vector , bool_vector )")
    _inplace_mul         = UtilPrototype("void   bool_vector_inplace_mul( bool_vector , bool_vector )")
    _assign              = UtilPrototype("void   bool_vector_set_all( bool_vector , bool)")
    _memcpy              = UtilPrototype("void   bool_vector_memcpy(bool_vector , bool_vector )")
    _set_default         = UtilPrototype("void   bool_vector_set_default( bool_vector , bool)")
    _get_default         = UtilPrototype("bool   bool_vector_get_default( bool_vector )")
    _element_size        = UtilPrototype("int    bool_vector_element_size( bool_vector )")

    _permute             = UtilPrototype("void bool_vector_permute(bool_vector, permutation_vector)")
    _sort_perm           = UtilPrototype("permutation_vector_obj bool_vector_alloc_sort_perm(bool_vector)")
    _rsort_perm          = UtilPrototype("permutation_vector_obj bool_vector_alloc_rsort_perm(bool_vector)")

    _contains            = UtilPrototype("bool bool_vector_contains(bool_vector, bool)")
    _select_unique       = UtilPrototype("void bool_vector_select_unique(bool_vector)")
    _element_sum         = UtilPrototype("bool bool_vector_sum(bool_vector)")
    _get_data_ptr        = UtilPrototype("bool* bool_vector_get_ptr(bool_vector)")
    _count_equal         = UtilPrototype("int bool_vector_count_equal(bool_vector, bool)")

    def __init__(self, default_value=False, initial_size=0):
        super(BoolVector, self).__init__(default_value, initial_size)

    def count(self, value=True):
        """ @rtype: int """
        return self._count_equal(self, value)

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
        return cls._create_active_mask(range_string)

    def updateActiveMask(self, range_string):
        """
        Updates a bool vector based on a range string.
        @type range_string: str
        @type bool_vector: BoolVector
        @rtype: bool
        """
        return self._update_active_mask(range_string , self)

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
        return self._active_list(self)

    def _tostr(self, arr = None):
        if arr is None:
            arr = self
        return "".join(['1' if x else '0' for x in arr])

    def __repr__(self):
        """Will return BoolVector(size = 4, content = "0010") at 0x1729 and
        if size > 10, will return content = "0001...100", i.e., |content|<=10.
        """
        cnt = ''
        ls = len(self)
        if ls <= 20:
            cnt = self._tostr()
        else:
            a,b  = self[:9], self[-8:]
            cnt  = self._tostr(a)
            cnt += "..."
            cnt += self._tostr(b)
        return 'BoolVector(size = %d, content = "%s") %s' % (ls, cnt, self._ad_str())
