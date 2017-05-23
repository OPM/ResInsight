#  Copyright (C) 2014  Statoil ASA, Norway.
#   
#  The file 'double_vector.py' is part of ERT - Ensemble based Reservoir Tool.
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


class DoubleVector(VectorTemplate):
    default_format = "%8.4f"

    _alloc            = UtilPrototype("void*  double_vector_alloc( int , double )" , bind = False)
    _alloc_copy       = UtilPrototype("double_vector_obj   double_vector_alloc_copy( double_vector )")
    _strided_copy     = UtilPrototype("double_vector_obj   double_vector_alloc_strided_copy( double_vector , int , int , int)")
    _free             = UtilPrototype("void   double_vector_free( double_vector )")
    _iget             = UtilPrototype("double double_vector_iget( double_vector , int )")
    _safe_iget        = UtilPrototype("double double_vector_safe_iget(double_vector , int )")
    _iset             = UtilPrototype("double double_vector_iset( double_vector , int , double)")
    _size             = UtilPrototype("int    double_vector_size( double_vector )")
    _append           = UtilPrototype("void   double_vector_append( double_vector , double )")
    _idel_block       = UtilPrototype("void   double_vector_idel_block( double_vector , int , int )")
    _pop              = UtilPrototype("double double_vector_pop( double_vector )")
    _idel             = UtilPrototype("void   double_vector_idel( double_vector , int )")
    _lshift           = UtilPrototype("void   double_vector_lshift( double_vector , int )")
    _rshift           = UtilPrototype("void   double_vector_rshift( double_vector , int )")
    _insert           = UtilPrototype("void   double_vector_insert( double_vector , int , double)")
    _fprintf          = UtilPrototype("void   double_vector_fprintf( double_vector , FILE , char* , char*)")
    _sort             = UtilPrototype("void   double_vector_sort( double_vector )")
    _rsort            = UtilPrototype("void   double_vector_rsort( double_vector )")
    _reset            = UtilPrototype("void   double_vector_reset( double_vector )")
    _get_read_only    = UtilPrototype("bool   double_vector_get_read_only( double_vector )")
    _set_read_only    = UtilPrototype("void   double_vector_set_read_only( double_vector , bool )")
    _get_max          = UtilPrototype("double double_vector_get_max( double_vector )")
    _get_min          = UtilPrototype("double double_vector_get_min( double_vector )")
    _get_max_index    = UtilPrototype("int    double_vector_get_max_index( double_vector , bool)")
    _get_min_index    = UtilPrototype("int    double_vector_get_min_index( double_vector , bool)")
    _shift            = UtilPrototype("void   double_vector_shift( double_vector , double )")
    _scale            = UtilPrototype("void   double_vector_scale( double_vector , double )")
    _div              = UtilPrototype("void   double_vector_div( double_vector , double )")
    _inplace_add      = UtilPrototype("void   double_vector_inplace_add( double_vector , double_vector )")
    _inplace_mul      = UtilPrototype("void   double_vector_inplace_mul( double_vector , double_vector )")
    _assign           = UtilPrototype("void   double_vector_set_all( double_vector , double)")
    _memcpy           = UtilPrototype("void   double_vector_memcpy(double_vector , double_vector )")
    _set_default      = UtilPrototype("void   double_vector_set_default( double_vector , double)")
    _get_default      = UtilPrototype("double double_vector_get_default( double_vector )")
    _element_size     = UtilPrototype("int    double_vector_element_size( double_vector )")

    _permute          = UtilPrototype("void double_vector_permute(double_vector, permutation_vector)")
    _sort_perm        = UtilPrototype("permutation_vector_obj double_vector_alloc_sort_perm(double_vector)")
    _rsort_perm       = UtilPrototype("permutation_vector_obj double_vector_alloc_rsort_perm(double_vector)")
    _contains         = UtilPrototype("bool double_vector_contains(double_vector, double)")
    _select_unique    = UtilPrototype("void double_vector_select_unique(double_vector)")
    _element_sum      = UtilPrototype("double double_vector_sum(double_vector)")
    _get_data_ptr     = UtilPrototype("double* double_vector_get_ptr(double_vector)")
    _count_equal      = UtilPrototype("int double_vector_count_equal(double_vector, double)")
    _init_range       = UtilPrototype("void double_vector_init_range(double_vector, double , double , double)")

    def __init__(self, default_value=0, initial_size=0):
        super(DoubleVector, self).__init__(default_value, initial_size)
