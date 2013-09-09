#  Copyright (C) 2011  Statoil ASA, Norway. 
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

from ert.cwrap import CWrapper, BaseCClass
from ert.util import UTIL_LIB, StringList



class Hash(BaseCClass):
    """
    Base hash class that supports string c_void_p values
    """
    def __init__(self):
        c_ptr = Hash.cNamespace().alloc()
        super(Hash, self).__init__(c_ptr)

    def __len__(self):
        return Hash.cNamespace().size(self)

    def __getitem__(self, key):
        if Hash.cNamespace().has_key(self, key):
            return Hash.cNamespace().get(self, key)
        else:
            raise KeyError("Hash does not have key: %s" % key)

    def __setitem__(self, key, value):
        if isinstance(value, c_void_p):
            Hash.cNamespace().insert_ref(self, key, value)
        else:
            raise ValueError("Hash does not support type: %s" % value.__class__)

    def __contains__(self, key):
        """ @rtype: bool """
        return Hash.cNamespace().has_key(self, key)

    def __iter__(self):
        for key in self.keys():
            yield key

    def keys(self):
        """ @rtype: StringList """
        return Hash.cNamespace().keys(self)

    def free(self):
        Hash.cNamespace().free(self)


class StringHash(Hash):
    def __init__(self):
        super(StringHash, self).__init__()

    def __setitem__(self, key, value):
        if isinstance(value, str):
            StringHash.cNamespace().insert_string(self, key, value)
        else:
            raise ValueError("StringHash does not support type: %s" % value.__class__)


    def __getitem__(self, key):
        if key in self:
            return StringHash.cNamespace().get_string(self, key)
        else:
            raise KeyError("Hash does not have key: %s" % key)

class IntegerHash(Hash):
    def __init__(self):
        super(IntegerHash, self).__init__()

    def __setitem__(self, key, value):
        if isinstance(value, int):
            IntegerHash.cNamespace().insert_int(self, key, value)
        else:
            raise ValueError("IntegerHash does not support type: %s" % value.__class__)


    def __getitem__(self, key):
        if key in self:
            return IntegerHash.cNamespace().get_int(self, key)
        else:
            raise KeyError("Hash does not have key: %s" % key)

class DoubleHash(Hash):
    def __init__(self):
        super(DoubleHash, self).__init__()

    def __setitem__(self, key, value):
        if isinstance(value, int):
            value = float(value)

        if isinstance(value, float):
            DoubleHash.cNamespace().insert_double(self, key, value)
        else:
            raise ValueError("DoubleHash does not support type: %s" % value.__class__)


    def __getitem__(self, key):
        if key in self:
            return DoubleHash.cNamespace().get_double(self, key)
        else:
            raise KeyError("Hash does not have key: %s" % key)


cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerType("hash", Hash) #c_void_p type
CWrapper.registerType("integer_hash", IntegerHash)
CWrapper.registerType("string_has", StringHash)
CWrapper.registerType("double_hash", DoubleHash)

CWrapper.registerType("hash_obj", Hash.createPythonObject) #c_void_p type
CWrapper.registerType("string_hash_obj", StringHash.createPythonObject)
CWrapper.registerType("integer_hash_obj", IntegerHash.createPythonObject)
CWrapper.registerType("double_hash_obj", DoubleHash.createPythonObject)

CWrapper.registerType("hash_ref", Hash.createCReference) #c_void_p type
CWrapper.registerType("string_hash_ref", StringHash.createCReference)
CWrapper.registerType("integer_hash_ref", IntegerHash.createCReference)
CWrapper.registerType("double_hash_ref", DoubleHash.createCReference)


Hash.cNamespace().alloc = cwrapper.prototype("long hash_alloc()")
Hash.cNamespace().free = cwrapper.prototype("void hash_free(hash)")
Hash.cNamespace().size = cwrapper.prototype("int hash_get_size(hash)")
Hash.cNamespace().keys = cwrapper.prototype("stringlist_obj hash_alloc_stringlist(hash)")
Hash.cNamespace().has_key = cwrapper.prototype("bool hash_has_key(hash, char*)")

Hash.cNamespace().get = cwrapper.prototype("c_void_p hash_get(hash, char*)")
IntegerHash.cNamespace().get_int = cwrapper.prototype("int hash_get_int(hash, char*)")
DoubleHash.cNamespace().get_double = cwrapper.prototype("double hash_get_double(hash, char*)")
StringHash.cNamespace().get_string = cwrapper.prototype("char* hash_get_string(hash, char*)")

Hash.cNamespace().insert_ref = cwrapper.prototype("void hash_insert_ref(hash, char*, c_void_p)")
IntegerHash.cNamespace().insert_int = cwrapper.prototype("void hash_insert_int(hash, char*, int)")
DoubleHash.cNamespace().insert_double = cwrapper.prototype("void hash_insert_double(hash, char*, double)")
StringHash.cNamespace().insert_string = cwrapper.prototype("void hash_insert_string(hash, char*, char*)")
