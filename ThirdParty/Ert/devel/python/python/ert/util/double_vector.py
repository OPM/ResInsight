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

from ert.cwrap import CWrapper
from ert.util import UTIL_LIB, VectorTemplate


class DoubleVector(VectorTemplate):
    default_format       = "%8.4f"

    def __init__(self, default_value=0, initial_size=0):
        super(DoubleVector, self).__init__(default_value, initial_size)


cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerObjectType("double_vector", DoubleVector)


DoubleVector.cNamespace().alloc            = cwrapper.prototype("c_void_p   double_vector_alloc( int , double )")
DoubleVector.cNamespace().alloc_copy       = cwrapper.prototype("double_vector_obj   double_vector_alloc_copy( double_vector )")
DoubleVector.cNamespace().strided_copy     = cwrapper.prototype("double_vector_obj   double_vector_alloc_strided_copy( double_vector , int , int , int)")
DoubleVector.cNamespace().free             = cwrapper.prototype("void   double_vector_free( double_vector )")
DoubleVector.cNamespace().iget             = cwrapper.prototype("double double_vector_iget( double_vector , int )")
DoubleVector.cNamespace().safe_iget        = cwrapper.prototype("double double_vector_safe_iget(double_vector , int )")
DoubleVector.cNamespace().iset             = cwrapper.prototype("double double_vector_iset( double_vector , int , double)")
DoubleVector.cNamespace().size             = cwrapper.prototype("int    double_vector_size( double_vector )")
DoubleVector.cNamespace().append           = cwrapper.prototype("void   double_vector_append( double_vector , double )")
DoubleVector.cNamespace().idel_block       = cwrapper.prototype("void   double_vector_idel_block( double_vector , int , int )")
DoubleVector.cNamespace().pop              = cwrapper.prototype("double double_vector_pop( double_vector )")
DoubleVector.cNamespace().idel             = cwrapper.prototype("void   double_vector_idel( double_vector , int )")
DoubleVector.cNamespace().lshift           = cwrapper.prototype("void   double_vector_lshift( double_vector , int )")
DoubleVector.cNamespace().rshift           = cwrapper.prototype("void   double_vector_rshift( double_vector , int )")
DoubleVector.cNamespace().insert           = cwrapper.prototype("void   double_vector_insert( double_vector , int , double)")
DoubleVector.cNamespace().fprintf          = cwrapper.prototype("void   double_vector_fprintf( double_vector , FILE , char* , char*)")
DoubleVector.cNamespace().sort             = cwrapper.prototype("void   double_vector_sort( double_vector )")
DoubleVector.cNamespace().rsort            = cwrapper.prototype("void   double_vector_rsort( double_vector )")
DoubleVector.cNamespace().reset            = cwrapper.prototype("void   double_vector_reset( double_vector )")
DoubleVector.cNamespace().get_read_only    = cwrapper.prototype("bool   double_vector_get_read_only( double_vector )")
DoubleVector.cNamespace().set_read_only    = cwrapper.prototype("void   double_vector_set_read_only( double_vector , bool )")
DoubleVector.cNamespace().get_max          = cwrapper.prototype("double    double_vector_get_max( double_vector )")
DoubleVector.cNamespace().get_min          = cwrapper.prototype("double    double_vector_get_min( double_vector )")
DoubleVector.cNamespace().get_max_index    = cwrapper.prototype("int    double_vector_get_max_index( double_vector , bool)")
DoubleVector.cNamespace().get_min_index    = cwrapper.prototype("int    double_vector_get_min_index( double_vector , bool)")
DoubleVector.cNamespace().shift            = cwrapper.prototype("void   double_vector_shift( double_vector , double )")
DoubleVector.cNamespace().scale            = cwrapper.prototype("void   double_vector_scale( double_vector , double )")
DoubleVector.cNamespace().div              = cwrapper.prototype("void   double_vector_div( double_vector , double )")
DoubleVector.cNamespace().inplace_add      = cwrapper.prototype("void   double_vector_inplace_add( double_vector , double_vector )")
DoubleVector.cNamespace().inplace_mul      = cwrapper.prototype("void   double_vector_inplace_mul( double_vector , double_vector )")
DoubleVector.cNamespace().assign           = cwrapper.prototype("void   double_vector_set_all( double_vector , double)")
DoubleVector.cNamespace().memcpy           = cwrapper.prototype("void   double_vector_memcpy(double_vector , double_vector )")
DoubleVector.cNamespace().set_default      = cwrapper.prototype("void   double_vector_set_default( double_vector , double)")
DoubleVector.cNamespace().get_default      = cwrapper.prototype("double    double_vector_get_default( double_vector )")
DoubleVector.cNamespace().element_size     = cwrapper.prototype("int      double_vector_element_size( double_vector )")

DoubleVector.cNamespace().permute          = cwrapper.prototype("void double_vector_permute(double_vector, permutation_vector)")
DoubleVector.cNamespace().sort_perm        = cwrapper.prototype("permutation_vector_obj double_vector_alloc_sort_perm(double_vector)")
DoubleVector.cNamespace().rsort_perm       = cwrapper.prototype("permutation_vector_obj double_vector_alloc_rsort_perm(double_vector)")
DoubleVector.cNamespace().contains       = cwrapper.prototype("bool double_vector_contains(double_vector, double)")
DoubleVector.cNamespace().select_unique       = cwrapper.prototype("void double_vector_select_unique(double_vector)")
DoubleVector.cNamespace().element_sum       = cwrapper.prototype("double double_vector_sum(double_vector)")
DoubleVector.cNamespace().get_data_ptr      = cwrapper.prototype("double* double_vector_get_ptr(double_vector)")
DoubleVector.cNamespace().count_equal       = cwrapper.prototype("int double_vector_count_equal(double_vector, double)")
