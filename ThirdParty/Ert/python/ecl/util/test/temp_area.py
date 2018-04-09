#  Copyright (C) 2016  Statoil ASA, Norway. 
#   
#  The file 'temp_area.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import os
import os.path
from   ecl import EclPrototype
from . import TestArea

class TempArea(TestArea):
    """TempArea class is essentially similar to the TestArea class, with
    the only difference that the cwd is *not* changed into the newly
    created area.
    """
    
    _temp_area_alloc           = EclPrototype("void* temp_area_alloc( char* )" , bind = False)
    _temp_area_alloc_relative  = EclPrototype("void* temp_area_alloc_relative( char* , char* )" , bind = False)

    def __init__(self, name, prefix = None , store_area=False):
        if prefix:
            if os.path.exists( prefix ):
                c_ptr = self._temp_area_alloc_relative(prefix , name)
            else:
                raise IOError("The prefix path:%s must exist" % prefix)
        else:
            c_ptr = self._temp_area_alloc(name)
        super(TempArea, self).__init__(name , c_ptr = c_ptr , store_area = store_area)


    def __str__(self):
        return self.getPath()

    
    def get_cwd(self):
        """
        Since the TempArea class does *not* change the cwd this method
        just returns the ordinary os.getcwd().
        """
        return os.getcwd()

    
    def getPath(self):
        """
        Will return the full path to the temporary working area.
        """
        return self._get_cwd( )

    

class TempAreaContext(object):
    def __init__(self, name, prefix = None , store_area=False):
        self.name = name
        self.store_area = store_area
        self.prefix = prefix

    def __enter__(self):
        """
        @rtype: TempArea
        """
        self.temp_area = TempArea(self.name, prefix = self.prefix , store_area = self.store_area )
        return self.temp_area


    def __exit__(self, exc_type, exc_val, exc_tb):
        del self.temp_area
        return False
    

    
