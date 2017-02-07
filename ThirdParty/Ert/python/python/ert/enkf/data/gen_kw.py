#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_kw.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os.path

from cwrap import BaseCClass, CFILE

from ert.util import DoubleVector
from ert.enkf import EnkfPrototype
from ert.enkf.config import GenKwConfig


class GenKw(BaseCClass):
    TYPE_NAME = "gen_kw"
    _alloc             = EnkfPrototype("void*  gen_kw_alloc(gen_kw_config)", bind = False)
    _free              = EnkfPrototype("void   gen_kw_free(gen_kw_config)")
    _export_parameters = EnkfPrototype("void   gen_kw_write_export_file(gen_kw , FILE)")
    _export_template   = EnkfPrototype("void   gen_kw_ecl_write_template(gen_kw , char* )")
    _data_iget         = EnkfPrototype("double gen_kw_data_iget(gen_kw, int, bool)")
    _data_iset         = EnkfPrototype("void   gen_kw_data_iset(gen_kw, int, double)")
    _set_values        = EnkfPrototype("void   gen_kw_data_set_vector(gen_kw, double_vector)")
    _data_get          = EnkfPrototype("double gen_kw_data_get(gen_kw, char*, bool)")
    _data_set          = EnkfPrototype("void   gen_kw_data_set(gen_kw, char*, double)")
    _size              = EnkfPrototype("int    gen_kw_data_size(gen_kw)")
    _has_key           = EnkfPrototype("bool   gen_kw_data_has_key(gen_kw, char*)")
    _ecl_write         = EnkfPrototype("void   gen_kw_ecl_write(gen_kw,    char* , char* , FILE)")
    _iget_key          = EnkfPrototype("char*  gen_kw_get_name(gen_kw, int)")
    

    def __init__(self, gen_kw_config):
        """
         @type gen_kw_config: GenKwConfig
        """
        c_ptr = self._alloc(gen_kw_config)

        if c_ptr:
            super(GenKw, self).__init__(c_ptr)
            self.__str__ = self.__repr__
        else:
            raise ValueError('Cannot issue a GenKw from the given keyword config: %s.' % str(gen_kw_config))
    

    def exportParameters(self, file_name):
        """ @type: str """
        with open(file_name , "w") as py_file:
            cfile  = CFILE( py_file )
            self._export_parameters(cfile)


    def exportTemplate(self, file_name):
        """ @type: str """
        self._export_template(file_name)


    def __getitem__(self, key):
        """
        @type key: int or str
        @rtype: float
        """
        do_transform = False
        if isinstance(key, str):
            if not key in self:
                raise KeyError("Key %s does not exist" % (key))
            return self._data_get(key, do_transform)
        elif isinstance(key, int):
            if not 0 <= key < len(self):
                raise IndexError("Index out of range 0 <= %d < %d" % (key, len(self)))
            return self._data_iget(key, do_transform)
        else:
            raise TypeError("Illegal type for indexing, must be int or str, got: %s" % (key))


    def __setitem__(self, key, value):
        """
        @type key: int or str
        @type value: float
        """
        if isinstance(key, str):
            if not key in self:
                raise KeyError("Key %s does not exist" % (key))
            self._data_set(key, value)
        elif isinstance(key, int):
            if not 0 <= key < len(self):
                raise IndexError("Index out of range 0 <= %d < %d" % (key, len(self)))
            self._data_iset(key, value)
        else:
            raise TypeError("Illegal type for indexing, must be int or str, got: %s" % (key))


    def items(self):
        do_transform = False
        v = []
        for index in range(len(self)):
            v.append( ( self._iget_key( index ) ,
                        self._data_iget(index, do_transform)) )
        return v

        
    def eclWrite(self , path , filename , export_file = None):
        if not path is None:
            if not os.path.isdir(path):
                raise IOError("The directory:%s does not exist" % path)
        if export_file:
            with open(export_file , "w") as fileH:
                self._ecl_write(path , filename , CFILE( fileH ))
        else:
            self._ecl_write( path , filename , None )


    def setValues(self , values):
        if len(values) == len(self):
            if isinstance(values , DoubleVector):
                self._set_values( d )
            else:
                d = DoubleVector()
                for (index,v) in enumerate(values):
                    if isinstance(v, (int,long,float)):
                        d[index] = v
                    else:
                        raise TypeError("Values must numeric: %s is invalid" % v)
                self._set_values( d )
        else:
            raise ValueError("Size mismatch between GenKW and values")

    def __len__(self):
        """ @rtype: int """
        return self._size( )

    
    def __contains__(self, item):
        return self._has_key(item)


    def free(self):
        self._free()

    def __repr__(self):
        return 'GenKw(len = %d) at 0x%x' % (len(self), self._address())


