#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'ecl_sum_keyword_vector.py' is part of ERT - Ensemble based Reservoir Tool.
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


import numpy
import datetime

# Observe that there is some convention conflict with the C code
# regarding order of arguments: The C code generally takes the time
# index as the first argument and the key/key_index as second
# argument. In the python code this order has been reversed.

from cwrap import BaseCClass
from ecl.ecl import EclPrototype



class EclSumKeyWordVector(BaseCClass):
    TYPE_NAME       = "ecl_sum_vector"
    _alloc          = EclPrototype("void* ecl_sum_vector_alloc(ecl_sum )" , bind = False)
    _free           = EclPrototype("void ecl_sum_vector_free(ecl_sum_vector )")
    _add            = EclPrototype("bool ecl_sum_vector_add_key( ecl_sum_vector ,  char* )")
    _add_multiple   = EclPrototype("void ecl_sum_vector_add_keys( ecl_sum_vector ,  char* )")
    _get_size       = EclPrototype("int ecl_sum_vector_get_size( ecl_sum_vector )")

    

    def __init__(self, ecl_sum):
        c_pointer = self._alloc(ecl_sum)
        super(EclSumKeyWordVector, self).__init__(c_pointer)
        
    def __len__(self):
        return self._get_size( )

    def free(self):
        self._free( )

    def addKeyword(self, keyword):
        success = self._add(keyword)
        if not success:
            raise KeyError("Failed to add keyword to vector")

    def addKeywords(self, keyword_pattern):
        self._add_multiple(keyword_pattern)




