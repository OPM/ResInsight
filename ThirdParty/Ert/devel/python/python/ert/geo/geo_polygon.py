#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_kw.py' is part of ERT - Ensemble based Reservoir Tool. 
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
"""
Create a polygon
"""


import ctypes
import libgeo
from    ert.cwrap.cwrap  import *
from    ert.cwrap.cclass import CClass

class GeoPolygon(CClass):

    def __init__(self , points):
        c_ptr = cfunc.alloc_new()
        self.init_cobj( c_ptr , cfunc.free )
        for (xc,yz) in points:
            self.add_point( self , xc , yc )


    def add_point( self , xc , yc ):
        cfunc.add_point( self , xc , yc )


#################################################################

cwrapper = CWrapper( libgeo.lib )
cwrapper.registerType( "geo_polygon" , GeoPolygon )

cfunc                   = CWrapperNameSpace("geo_polygon")
cfunc.alloc_new         = cwrapper.prototype("c_void_p geo_polygon_alloc( )")
cfunc.add_point         = cwrapper.prototype("void     geo_polygon_add_point( geo_polygon , double , double )")
cfunc.free              = cwrapper.prototype("void     geo_polygon_free( geo_polygon )")
