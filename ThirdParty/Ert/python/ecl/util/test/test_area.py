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

from cwrap import BaseCClass
from ecl import EclPrototype


class TestArea(BaseCClass):
    _test_area_alloc           = EclPrototype("void* test_work_area_alloc( char* )" , bind = False)
    _test_area_alloc_relative  = EclPrototype("void* test_work_area_alloc_relative( char* , char* )" , bind = False)
    _free                      = EclPrototype("void test_work_area_free( test_area )")
    _install_file              = EclPrototype("void test_work_area_install_file( test_area , char* )")
    _copy_directory            = EclPrototype("void test_work_area_copy_directory( test_area , char* )")
    _copy_file                 = EclPrototype("void test_work_area_copy_file( test_area , char* )")
    _copy_directory_content    = EclPrototype("void test_work_area_copy_directory_content( test_area , char* )")
    _copy_parent_directory     = EclPrototype("void test_work_area_copy_parent_directory( test_area , char* )")
    _copy_parent_content       = EclPrototype("void test_work_area_copy_parent_content( test_area , char* )")
    _get_cwd                   = EclPrototype("char* test_work_area_get_cwd( test_area )")
    _get_original_cwd          = EclPrototype("char* test_work_area_get_original_cwd( test_area )")
    _set_store                 = EclPrototype("void test_work_area_set_store( test_area , bool)")
    _sync                      = EclPrototype("void test_work_area_sync( test_area )")

    def __init__(self, test_name, prefix = None , store_area=False , c_ptr = None):

        if c_ptr is None:
            if prefix:
                if os.path.exists( prefix ):
                    c_ptr = self._test_area_alloc_relative(prefix , test_name)
                else:
                    raise IOError("The prefix path:%s must exist" % prefix)
            else:
                c_ptr = self._test_area_alloc(test_name)

        super(TestArea, self).__init__(c_ptr)
        self.set_store( store_area )


    def get_original_cwd(self):
        return self._get_original_cwd()

    def get_cwd(self):
        return self._get_cwd()

    def orgPath(self , path):
        if os.path.isabs( path ):
            return path
        else:
            return os.path.abspath( os.path.join( self.get_original_cwd( ) , path ) )


    # All the methods install_file() , copy_directory(),
    # copy_parent_directory(), copy_parent_content(),
    # copy_directory_content() and copy_file() expect an input
    # argument which is relative to the original CWD - or absolute.

    def install_file( self, filename):
        if os.path.isfile(self.orgPath(filename)):
            self._install_file(filename)
        else:
            raise IOError("No such file:%s" % filename)


    def copy_directory( self, directory):
        if os.path.isdir( self.orgPath(directory) ):
            self._copy_directory(directory)
        else:
            raise IOError("No such directory: %s" % directory)

    def copy_parent_directory( self , path):
        if os.path.exists( self.orgPath(path) ):
            self._copy_parent_directory( path)
        else:
            raise IOError("No such file or directory: %s" % path)


    def copy_parent_content( self , path):
        if os.path.exists( self.orgPath(path) ):
            self._copy_parent_content(path)
        else:
            raise IOError("No such file or directory: %s" % path)

    def copy_directory_content( self, directory):
        if os.path.isdir( self.orgPath(directory) ):
            self._copy_directory_content(directory)
        else:
            raise IOError("No such directory: %s" % directory )


    def copy_file( self, filename):
        if os.path.isfile( self.orgPath(filename) ):
            self._copy_file(filename)
        else:
            raise IOError("No such file:%s" % filename)


    def free(self):
        self._free()


    def set_store(self, store):
        self._set_store(store)


    def getFullPath(self , path):
        if not os.path.exists( path ):
            raise IOError("Path not found:%s" % path)

        if os.path.isabs( path ):
            raise IOError("Path:%s is already absolute" % path)

        return os.path.join( self.get_cwd() , path )


    def sync(self):
        return self._sync( )



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
        self.test_area.free()      # free the TestData object (and cd back to the original dir)
        self.test_area.free = None # avoid double free
        del self.test_area
        return False
