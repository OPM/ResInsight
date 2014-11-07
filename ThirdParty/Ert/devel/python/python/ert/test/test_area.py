#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'test_work_area.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert.cwrap import clib, BaseCClass, CWrapper
from ert.util import UTIL_LIB


class TestArea(BaseCClass):
    def __init__(self, test_name, prefix = None , store_area=False):
        if prefix:
            if os.path.exists( prefix ):
                c_ptr = TestArea.cNamespace().test_area_alloc__(prefix , test_name)
            else:
                raise IOError("The prefix path:%s must exist" % prefix)
        else:
            c_ptr = TestArea.cNamespace().test_area_alloc(test_name)
        super(TestArea, self).__init__(c_ptr)
        self.set_store( store_area )

    def get_original_cwd(self):
        return TestArea.cNamespace().get_original_cwd(self)

    def get_cwd(self):
        return TestArea.cNamespace().get_cwd(self)
        

    def install_file( self, filename):
        TestArea.cNamespace().install_file(self, filename)


    def copy_directory( self, directory):
        TestArea.cNamespace().copy_directory(self, directory)

    def copy_parent_directory( self , path):
        TestArea.cNamespace().copy_parent_directory(self , path)

    def copy_parent_content( self , path):
        TestArea.cNamespace().copy_parent_content(self , path)

    def copy_directory_content( self, directory):
        TestArea.cNamespace().copy_directory_content(self, directory)

    def copy_file( self, filename):
        TestArea.cNamespace().copy_file(self, filename)

    def free(self):
        TestArea.cNamespace().free(self)

    def set_store(self, store):
        TestArea.cNamespace().set_store(self , store)

    def getFullPath(self , path):
        if not os.path.exists( path ):
            raise IOError("Path not found:%s" % path)

        if os.path.isabs( path ):
            raise IOError("Path:%s is already absolute" % path)

        return os.path.join( self.get_cwd() , path )



class TestAreaContext(object):
    def __init__(self, test_name, prefix = None , store_area=False):
        self.test_name = test_name
        self.store_area = store_area
        self.prefix = prefix

    def __enter__(self):
        """
         @rtype: TestArea
        """
        self.test_area = TestArea(self.test_name, prefix = self.prefix , store_area = self.store_area )
        return self.test_area


    def __exit__(self, exc_type, exc_val, exc_tb):
        del self.test_area
        return False



cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerType("test_area", TestArea)
CWrapper.registerType("test_area_obj", TestArea.createPythonObject)
CWrapper.registerType("test_area_ref", TestArea.createCReference)

TestArea.cNamespace().test_area_alloc   = cwrapper.prototype("c_void_p test_work_area_alloc( char* )")
TestArea.cNamespace().test_area_alloc__ = cwrapper.prototype("c_void_p test_work_area_alloc__( char* , char* )")
TestArea.cNamespace().free = cwrapper.prototype("void test_work_area_free( test_area )")
TestArea.cNamespace().install_file = cwrapper.prototype("void test_work_area_install_file( test_area , char* )")
TestArea.cNamespace().copy_directory = cwrapper.prototype("void test_work_area_copy_directory( test_area , char* )")
TestArea.cNamespace().copy_file = cwrapper.prototype("void test_work_area_copy_file( test_area , char* )")
TestArea.cNamespace().copy_directory_content = cwrapper.prototype("void test_work_area_copy_directory_content( test_area , char* )")
TestArea.cNamespace().copy_parent_directory = cwrapper.prototype("void test_work_area_copy_parent_directory( test_area , char* )")
TestArea.cNamespace().copy_parent_content = cwrapper.prototype("void test_work_area_copy_parent_content( test_area , char* )")
TestArea.cNamespace().get_cwd = cwrapper.prototype("char* test_work_area_get_cwd( test_area )")
TestArea.cNamespace().get_original_cwd = cwrapper.prototype("char* test_work_area_get_original_cwd( test_area )")
TestArea.cNamespace().set_store = cwrapper.prototype("void test_work_area_set_store( test_area , bool)")
