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

from ert.cwrap import clib, BaseCClass, CWrapper


lib = clib.ert_load("libtest_util.so")


class TestArea(BaseCClass):
    def __init__(self, test_name, store_area=False):
        c_ptr = TestArea.cNamespace().test_area_alloc(test_name, store_area)
        super(TestArea, self).__init__(c_ptr)



    def install_file( self, filename):
        TestArea.cNamespace().install_file(self, filename)


    def copy_directory( self, directory):
        TestArea.cNamespace().copy_directory(self, directory)


    def copy_directory_content( self, directory):
        TestArea.cNamespace().copy_directory_content(self, directory)

    def copy_file( self, filename):
        TestArea.cNamespace().copy_file(self, filename)

    def free(self):
        TestArea.cNamespace().free(self)


class TestAreaContext(object):
    def __init__(self, test_name, store_area=False):
        self.test_name = test_name
        self.store_area = store_area

    def __enter__(self):
        """
         @rtype: TestArea
        """
        self.test_area = TestArea(self.test_name, self.store_area)
        return self.test_area


    def __exit__(self, exc_type, exc_val, exc_tb):
        del self.test_area
        return False



cwrapper = CWrapper(lib)
CWrapper.registerType("test_area", TestArea)
CWrapper.registerType("test_area_obj", TestArea.createPythonObject)
CWrapper.registerType("test_area_ref", TestArea.createCReference)

TestArea.cNamespace().test_area_alloc = cwrapper.prototype("c_void_p test_work_area_alloc( char* , bool)")
TestArea.cNamespace().free = cwrapper.prototype("void test_work_area_free( test_area )")
TestArea.cNamespace().install_file = cwrapper.prototype("void test_work_area_install_file( test_area , char* )")
TestArea.cNamespace().copy_directory = cwrapper.prototype("void test_work_area_copy_directory( test_area , char* )")
TestArea.cNamespace().copy_file = cwrapper.prototype("void test_work_area_copy_file( test_area , char* )")
TestArea.cNamespace().copy_directory_content = cwrapper.prototype("void test_work_area_copy_directory_content( test_area , char* )")
