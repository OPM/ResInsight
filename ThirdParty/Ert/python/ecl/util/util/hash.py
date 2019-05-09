#  Copyright (C) 2011  Equinor ASA, Norway. 
#   
#  The file 'hash.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ctypes import c_void_p

from cwrap import BaseCClass
from ecl import EclPrototype
from ecl.util.util import StringList


class Hash(BaseCClass):
    _alloc =      EclPrototype("void* hash_alloc()" , bind = False)
    _free =       EclPrototype("void hash_free(hash)")
    _size =       EclPrototype("int hash_get_size(hash)")
    _keys =       EclPrototype("stringlist_obj hash_alloc_stringlist(hash)")
    _has_key =    EclPrototype("bool hash_has_key(hash, char*)")
    _get =        EclPrototype("void* hash_get(hash, char*)")
    _insert_ref = EclPrototype("void hash_insert_ref(hash, char*, void*)")

    """
    Base hash class that supports string:void* values
    """

    def __init__(self):
        c_ptr = self._alloc()
        super(Hash, self).__init__(c_ptr)

    def __len__(self):
        return self._size()

    def __getitem__(self, key):
        if self._has_key(key):
            return self._get(key)
        else:
            raise KeyError("Hash does not have key: %s" % key)

    def __setitem__(self, key, value):
        if isinstance(value, c_void_p):
            self._insert_ref(key, value)
        else:
            raise ValueError("Hash does not support type: %s" % value.__class__)

    def __contains__(self, key):
        """ @rtype: bool """
        return self._has_key(key)

    def __iter__(self):
        for key in self.keys():
            yield key

    def keys(self):
        """ @rtype: StringList """
        return self._keys()

    def free(self):
        self._free()

    def __str__(self):
        return str(["%s: %s" % (key, self[key]) for key in self.keys()])


class StringHash(Hash):
    _get_string = EclPrototype("char* hash_get_string(hash, char*)")
    _insert_string = EclPrototype("void hash_insert_string(hash, char*, char*)")

    def __init__(self):
        super(StringHash, self).__init__()

    def __setitem__(self, key, value):
        if isinstance(value, str):
            self._insert_string(key, value)
        else:
            raise ValueError("StringHash does not support type: %s" % value.__class__)

    def __getitem__(self, key):
        if key in self:
            return self._get_string(key)
        else:
            raise KeyError("Hash does not have key: %s" % key)


class IntegerHash(Hash):
    _get_int = EclPrototype("int hash_get_int(hash, char*)")
    _insert_int = EclPrototype("void hash_insert_int(hash, char*, int)")

    def __init__(self):
        super(IntegerHash, self).__init__()

    def __setitem__(self, key, value):
        if isinstance(value, int):
            self._insert_int(key, value)
        else:
            raise ValueError("IntegerHash does not support type: %s" % value.__class__)

    def __getitem__(self, key):
        if key in self:
            return self._get_int(key)
        else:
            raise KeyError("Hash does not have key: %s" % key)


class DoubleHash(Hash):
    _get_double = EclPrototype("double hash_get_double(hash, char*)")
    _insert_double = EclPrototype("void hash_insert_double(hash, char*, double)")

    def __init__(self):
        super(DoubleHash, self).__init__()

    def __setitem__(self, key, value):
        if isinstance(value, int):
            value = float(value)

        if isinstance(value, float):
            self._insert_double(key, value)
        else:
            raise ValueError("DoubleHash does not support type: %s" % value.__class__)

    def __getitem__(self, key):
        if key in self:
            return self._get_double( key)
        else:
            raise KeyError("Hash does not have key: %s" % key)
