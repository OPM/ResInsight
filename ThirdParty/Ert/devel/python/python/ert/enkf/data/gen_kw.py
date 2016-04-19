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

from ert.cwrap import BaseCClass, CWrapper, CFILE

from ert.util import DoubleVector

from ert.enkf import ENKF_LIB
from ert.enkf.config import GenKwConfig


class GenKw(BaseCClass):
    def __init__(self, gen_kw_config):
        """
         @type gen_kw_config: GenKwConfig
        """
        c_ptr = GenKw.cNamespace().alloc(gen_kw_config)
        super(GenKw, self).__init__(c_ptr)

    def exportParameters(self, file_name):
        """ @type: str """
        with open(file_name , "w") as py_file:
            cfile  = CFILE( py_file )
            GenKw.cNamespace().export_parameters(self, cfile)


    def exportTemplate(self, file_name):
        """ @type: str """
        GenKw.cNamespace().export_template(self, file_name)


    def __getitem__(self, key):
        """
        @type key: int or str
        @rtype: float
        """
        do_transform = False
        if isinstance(key, str):
            if not key in self:
                raise KeyError("Key %s does not exist" % (key))
            return GenKw.cNamespace().data_get(self, key, do_transform)
        elif isinstance(key, int):
            if not 0 <= key < len(self):
                raise IndexError("Index out of range 0 <= %d < %d" % (key, len(self)))
            return GenKw.cNamespace().data_iget(self, key, do_transform)
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
            GenKw.cNamespace().data_set(self, key, value)
        elif isinstance(key, int):
            if not 0 <= key < len(self):
                raise IndexError("Index out of range 0 <= %d < %d" % (key, len(self)))
            GenKw.cNamespace().data_iset(self, key, value)
        else:
            raise TypeError("Illegal type for indexing, must be int or str, got: %s" % (key))


    def eclWrite(self , path , filename , export_file = None):
        if not path is None:
            if not os.path.isdir(path):
                raise IOError("The directory:%s does not exist" % path)
                
                
        if export_file:
            with open(export_file , "w") as fileH:
                GenKw.cNamespace().ecl_write(self , path , filename , CFILE( fileH ))
        else:
            GenKw.cNamespace().ecl_write(self , path , filename , None )

            

    def setValues(self , values):
        if len(values) == len(self):
            if isinstance(values , DoubleVector):
                GenKw.cNamespace().set_values( self , d )
            else:
                d = DoubleVector()
                for (index,v) in enumerate(values):
                    if isinstance(v, (int,long,float)):
                        d[index] = v
                    else:
                        raise TypeError("Values must numeric: %s is invalid" % v)
                GenKw.cNamespace().set_values( self , d )
        else:
            raise ValueError("Size mismatch between GenKW and values")

    def __len__(self):
        """ @rtype: int """
        return GenKw.cNamespace().size(self)

    def __contains__(self, item):
        return GenKw.cNamespace().has_key(self, item)


    def free(self):
        GenKw.cNamespace().free(self)


    ##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("gen_kw", GenKw)

GenKw.cNamespace().free = cwrapper.prototype("void gen_kw_free(gen_kw_config)")
GenKw.cNamespace().alloc = cwrapper.prototype("c_void_p gen_kw_alloc(gen_kw_config)")
GenKw.cNamespace().export_parameters = cwrapper.prototype("void gen_kw_write_export_file(gen_kw , FILE)")
GenKw.cNamespace().export_template = cwrapper.prototype("void gen_kw_ecl_write_template(gen_kw , char* )")
GenKw.cNamespace().data_iget = cwrapper.prototype("double gen_kw_data_iget(gen_kw, int, bool)")
GenKw.cNamespace().data_iset = cwrapper.prototype("void gen_kw_data_iset(gen_kw, int, double)")
GenKw.cNamespace().set_values = cwrapper.prototype("void gen_kw_data_set_vector(gen_kw, double_vector)")
GenKw.cNamespace().data_get = cwrapper.prototype("double gen_kw_data_get(gen_kw, char*, bool)")
GenKw.cNamespace().data_set = cwrapper.prototype("void gen_kw_data_set(gen_kw, char*, double)")
GenKw.cNamespace().size = cwrapper.prototype("int gen_kw_data_size(gen_kw)")
GenKw.cNamespace().has_key = cwrapper.prototype("bool gen_kw_data_has_key(gen_kw, char*)")
GenKw.cNamespace().ecl_write = cwrapper.prototype("void gen_kw_ecl_write(gen_kw, char* , char* , FILE)")
