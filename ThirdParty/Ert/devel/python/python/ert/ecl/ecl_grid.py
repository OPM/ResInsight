#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_grid.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Module to load and query ECLIPSE GRID/EGRID files.

The ecl_grid module contains functionality to load and query an
ECLIPSE grid file; it is currently not possible to manipulate or let
alone create a grid with ecl_grid module. The functionality is
implemented in the EclGrid class. The ecl_grid module is a thin
wrapper around the ecl_grid.c implementation from the libecl library.
"""
import ctypes

import numpy
import sys
import warnings
import os.path
import math
from ert.cwrap import CClass, CFILE, CWrapper, CWrapperNameSpace
from ert.util import IntVector
from ert.ecl import EclTypeEnum, EclKW, ECL_LIB, FortIO


class EclGrid(CClass):
    """
    Class for loading and internalizing ECLIPSE GRID/EGRID files.
    """
    
    @classmethod
    def loadFromGrdecl(cls , filename):
        """Will create a new EclGrid instance from grdecl file.

        This function will scan the input file @filename and look for
        the keywords required to build a grid. The following keywords
        are required:
         
              SPECGRID   ZCORN   COORD

        In addition the function will look for and use the ACTNUM and
        MAPAXES keywords if they are found; if ACTNUM is not found all
        cells are assumed to be active.

        Slightly more exotic grid concepts like dual porosity, NNC
        mapping, LGR and coarsened cells will be completely ignored;
        if you need such concepts you must have an EGRID file and use
        the default EclGrid() constructor - that is also considerably
        faster.
        """

        if os.path.isfile(filename):
            with open(filename) as f:
                specgrid = EclKW.read_grdecl(f, "SPECGRID", ecl_type=EclTypeEnum.ECL_INT_TYPE, strict=False)
                zcorn = EclKW.read_grdecl(f, "ZCORN")
                coord = EclKW.read_grdecl(f, "COORD")
                actnum = EclKW.read_grdecl(f, "ACTNUM", ecl_type=EclTypeEnum.ECL_INT_TYPE)
                mapaxes = EclKW.read_grdecl(f, "MAPAXES")

            if specgrid is None:
                raise ValueError("The grdecl file:%s was invalid - could not find SPECGRID keyword" % filename)

            if zcorn is None:
                raise ValueError("The grdecl file:%s was invalid - could not find ZCORN keyword" % filename)

            if coord is None:
                raise ValueError("The grdecl file:%s was invalid - could not find COORD keyword" % filename)

            return EclGrid.create( specgrid , zcorn , coord , actnum , mapaxes )
        else:
            raise IOError("No such file:%s" % filename)

    @classmethod
    def loadFromFile(cls , filename):
        """
        Will inspect the @filename argument and create a new EclGrid instance.
        """
        if FortIO.isFortranFile( filename ):
            return EclGrid( filename )
        else:
            return EclGrid.loadFromGrdecl( filename )
            

    @classmethod
    def create(cls , specgrid , zcorn , coord , actnum , mapaxes = None ):

        """
        Create a new grid instance from existing keywords.

        This is a class method which can be used to create an EclGrid
        instance based on the EclKW instances @specgrid, @zcorn,
        @coord and @actnum. An ECLIPSE EGRID file contains the
        SPECGRID, ZCORN, COORD and ACTNUM keywords, so a somewhat
        involved way to create a EclGrid instance could be:

          file = ecl.EclFile( "ECLIPSE.EGRID" )
          specgrid_kw = file.iget_named_kw( "SPECGRID" , 0)
          zcorn_kw = file.iget_named_kw( "ZCORN" , 0)
          coord_kw = file.iget_named_kw( "COORD" , 0)
          actnum_kw = file.iget_named_kw( "ACTNUM" , 0 )
          
          grid = EclGrid.create( specgrid_kw , zcorn_kw , coord_kw , actnum_kw)
          
        If you are so inclined ...  
        """
        obj = object.__new__( cls )
        c_ptr = cfunc.grdecl_create( specgrid[0] , specgrid[1] , specgrid[2] , zcorn , coord , actnum , mapaxes) 
        obj.init_cobj( c_ptr , cfunc.free )
        return obj


    @classmethod
    def create_rectangular(cls , dims , dV , actnum = None):
        return cls.createRectangular( dims , dV , actnum )


    @classmethod
    def createRectangular(cls , dims , dV , actnum = None):
        """
        Will create a new rectangular grid. @dims = (nx,ny,nz)  @dVg = (dx,dy,dz)
        
        With the default value @actnum == None all cells will be active, 
        """
        obj = object.__new__( cls )
        if actnum is None:
            c_ptr = cfunc.alloc_rectangular( dims[0] , dims[1] , dims[2] , dV[0] , dV[1] , dV[2] , None )
        else:
            if not isinstance(actnum , IntVector):
                tmp = IntVector(initial_size = len(actnum))
                for (index , value) in enumerate(actnum):
                    tmp[index] = value
                actnum = tmp
            
            if not len(actnum) == dims[0] * dims[1] * dims[2]:
                raise ValueError("ACTNUM size mismatch: len(ACTNUM):%d  Expected:%d" % (len(actnum) , dims[0] * dims[1] * dims[2]))
            c_ptr = cfunc.alloc_rectangular( dims[0] , dims[1] , dims[2] , dV[0] , dV[1] , dV[2] , actnum.getDataPtr() )
            
        obj.init_cobj( c_ptr , cfunc.free )
        return obj
        

    def __new__(cls , filename , lgr = None , parent = None):
        if filename:
            c_ptr = cfunc.fread_alloc( filename )
        elif lgr:
            c_ptr = lgr
            
        if c_ptr:
            obj = object.__new__( cls )
            if lgr:
                obj.init_cref( c_ptr , parent )
            else:
                obj.init_cobj( c_ptr , cfunc.free )
            return obj
        else:
            raise IOError("Loading grid from:%s failed" % filename)


    def equal(self , other , include_lgr = True , include_nnc = False , verbose = False):
        """
        Compare the current grid with the other grid.
        """
        if not isinstance(other , EclGrid):
            raise TypeError("The other argument must be an EclGrid instance")
        return cfunc.equal( self , other , include_lgr , include_nnc , verbose)

    @property
    def dual_grid( self ):
        """Is this grid dual porosity model?"""
        return cfunc.dual_grid( self ) 


    @property
    def nx( self ):
        """The number of cells in the i direction - nx."""
        return cfunc.get_nx( self )

    @property
    def ny( self ):
        """The number of cells in the j direction - ny."""
        return cfunc.get_ny( self )

    @property
    def nz( self ):
        """The number of cells in the k direction - nz."""
        return cfunc.get_nz( self )

    @property
    def size( self ):
        """The total number of cells in the grid, i.e. nx*ny*nz."""
        return cfunc.get_global_size( self )

    @property
    def nactive( self ):
        """The number of active cells in the grid."""
        return self.getNumActive()

    @property
    def nactive_fracture( self ):
        """The number of active cells fracture in the grid - for dual porosity."""
        return cfunc.get_active_fracture( self )


    @property
    def dims( self ):
        warnings.warn("The dims property is deprecated - use getDims() method instead" , DeprecationWarning)
        return self.getDims( )


    def getDims(self):
        """A tuple of four elements: (nx , ny , nz , nactive)."""
        return ( cfunc.get_nx( self ) ,
                 cfunc.get_ny( self ) ,
                 cfunc.get_nz( self ) ,
                 cfunc.get_active( self ) )


    def getNX(self):
        """ The number of elements in the x direction"""
        return cfunc.get_nx( self )

    def getNY(self):
        """ The number of elements in the y direction"""
        return cfunc.get_ny( self )

    def getNZ(self):
        """ The number of elements in the z direction"""
        return cfunc.get_nz( self )

    def getGlobalSize(self):
        """Returns the total number of cells in this grid"""
        return cfunc.get_global_size( self )

    def getNumActive(self):
        """The number of active cells in the grid."""
        return cfunc.get_active( self )


    def getBoundingBox2D(self , layer = 0 , lower_left = None , upper_right = None):
        if 0 <= layer <= self.getNZ():
            x = ctypes.c_double()
            y = ctypes.c_double()
            z = ctypes.c_double()
            
            if lower_left is None:
                i1 = 0
                j1 = 0
            else:
                i1,j1 = lower_left
                if not 0 < i1 < self.getNX():
                    raise ValueError("lower_left i coordinate invalid")

                if not 0 < j1 < self.getNY():
                    raise ValueError("lower_left j coordinate invalid")

                
            if upper_right is None:
                i2 = self.getNX()
                j2 = self.getNY()
            else:
                i2,j2 = upper_right
                
                if not 1 < i2 <= self.getNX():
                    raise ValueError("upper_right i coordinate invalid")

                if not 1 < j2 <= self.getNY():
                    raise ValueError("upper_right j coordinate invalid")
                    
            if not i1 < i2:
                raise ValueError("Must have lower_left < upper_right")
            
            if not j1 < j2:
                raise ValueError("Must have lower_left < upper_right")



            cfunc.get_corner_xyz( self , i1 , j1 , layer , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z) )
            p0 = (x.value , y.value )

            cfunc.get_corner_xyz( self , i2 , j1 , layer , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z) )
            p1 = (x.value , y.value  )

            cfunc.get_corner_xyz( self , i2 , j2 , layer , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z) )
            p2 = (x.value , y.value  )

            cfunc.get_corner_xyz( self , i1 , j2 , layer , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z) )
            p3 = (x.value , y.value  )

            return (p0,p1,p2,p3)
        else:
            raise ValueError("Invalid layer value:%d  Valid range: [0,%d]" % (layer , self.getNZ()))



    @property
    def name( self ):
        """
        Name of the current grid.
        
        For the main grid this is the filename given to the
        constructor when loading the grid; for an LGR this is the name
        of the LGR. If the grid instance has been created with the
        create() classmethod this can be None.
        """
        return cfunc.get_name( self )

    def global_index( self , active_index = None, ijk = None):
        """
        Will convert either active_index or (i,j,k) to global index.
        """
        return self.__global_index( active_index = active_index , ijk = ijk )

    def __global_index( self , active_index = None , global_index = None , ijk = None):
        """
        Will convert @active_index or @ijk to global_index.

        This method will convert @active_index or @ijk to a global
        index. Exactly one of the arguments @active_index,
        @global_index or @ijk must be supplied. 

        The method is used extensively internally in the EclGrid
        class; most methods which take coordinate input pass through
        this method to normalize the coordinate representation.
        """

        set_count = 0
        if not active_index is None:
            set_count += 1

        if not global_index is None:
            set_count += 1

        if ijk:
            set_count += 1
            
        if not set_count == 1:
            raise ValueError("Exactly one of the kewyord arguments active_index, global_index or ijk must be set")
        
        if not active_index is None:
            global_index = cfunc.get_global_index1A( self , active_index )
        elif ijk:
            nx = self.getNX()
            ny = self.getNY()
            nz = self.getNZ()
            
            i,j,k = ijk

            if not 0 <= i < nx:
                raise IndexError("Invalid value i:%d  Range: [%d,%d)" % (i , 0 , nx)) 

            if not 0 <= j < ny:
                raise IndexError("Invalid value j:%d  Range: [%d,%d)" % (j , 0 , ny)) 
                
            if not 0 <= k < nz:
                raise IndexError("Invalid value k:%d  Range: [%d,%d)" % (k , 0 , nz)) 

            global_index = cfunc.get_global_index3( self , i,j,k)
        else:
            if not 0 <= global_index < self.size:
                raise IndexError("Invalid value global_index:%d  Range: [%d,%d)" % (global_index , 0 , self.size)) 
        return global_index
                 

    def get_active_index( self , ijk = None , global_index = None):
        """
        Lookup active index based on ijk or global index.

        Will determine the active_index of a cell, based on either
        @ijk = (i,j,k) or @global_index. If the cell specified by the
        input arguments is not active the function will return -1.
        """
        gi = self.__global_index( global_index = global_index , ijk = ijk)
        return cfunc.get_active_index1( self , gi)


    def get_active_fracture_index( self , ijk = None , global_index = None):
        """
        For dual porosity - get the active fracture index.
        """
        gi = self.__global_index( global_index = global_index , ijk = ijk)
        return cfunc.get_active_fracture_index1( self , gi )


    def get_global_index1F( self , active_fracture_index):
        """
        Will return the global index corresponding to active fracture index.
        """
        return cfunc.get_global_index1F( self , active_fracture_index )


    def cell_invalid( self , ijk = None , global_index = None , active_index = None):
        """
        Tries to check if a cell is invalid.

        Cells which are used to represent numerical aquifers are
        typically located in UTM position (0,0); these cells have
        completely whacked up shape and size, and should **NOT** be
        used in calculations involving real world coordinates. To
        protect against this a heuristic is used identify such cells
        and mark them as invalid. There might be other sources than
        numerical aquifers to this problem.
        """
        gi = self.__global_index( global_index = global_index , ijk = ijk , active_index = active_index)
        return cfunc.invalid_cell( self , gi )


    def active( self , ijk = None , global_index = None):
        """
        Is the cell active?

        See documentation og get_xyz() for explanation of parameters
        @ijk and @global_index.
        """
        gi = self.__global_index( global_index = global_index , ijk = ijk)
        active_index = cfunc.get_active_index1( self , gi)
        if active_index >= 0:
            return True
        else:
            return False


    def get_global_index( self , ijk = None , active_index = None):
        """
        Lookup global index based on ijk or active index.
        """
        gi = self.__global_index( active_index = active_index , ijk = ijk)
        return gi


    def get_ijk( self, active_index = None , global_index = None):
        """
        Lookup (i,j,k) for a cell, based on either active index or global index.

        The return value is a tuple with three elements (i,j,k).
        """
        i = ctypes.c_int()
        j = ctypes.c_int()
        k = ctypes.c_int()

        gi = self.__global_index( active_index = active_index , global_index = global_index)
        cfunc.get_ijk1( self , gi , ctypes.byref(i) , ctypes.byref(j) , ctypes.byref(k))

        return (i.value , j.value , k.value)


    def get_xyz( self, active_index = None , global_index = None , ijk = None):
        """
        Find true position of cell center.

        Will return world position of the center of a cell in the
        grid. The return value is a tuple of three elements: 
        (utm_x , utm_y , depth).
        
        The cells of a grid can be specified in three different ways:

           (i,j,k)      : As a tuple of i,j,k values.

           global_index : A number in the range [0,nx*ny*nz). The
                          global index is related to (i,j,k) as:

                            global_index = i + j*nx + k*nx*ny
           
           active_index : A number in the range [0,nactive).
           
        For many of the EclGrid methods a cell can be specified using
        any of these three methods. Observe that one and only method is
        allowed:

        OK:
            pos1 = grid.get_xyz( active_index = 100 )                    
            pos2 = grid.get_xyz( ijk = (10,20,7 ))                       

        Crash and burn:
            pos3 = grid.get_xyz( ijk = (10,20,7 ) , global_index = 10)   
            pos4 = grid.get_xyz()
            
        All the indices in the EclGrid() class are zero offset, this
        is in contrast to ECLIPSE which has an offset 1 interface.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)

        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()
        cfunc.get_xyz1( self , gi , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z))
        return (x.value , y.value , z.value)


    def getNodePos(self , i , j , k):
        """Will return the (x,y,z) for the node given by (i,j,k).

        Observe that this method does not consider cells, but the
        nodes in the grid. This means that the valid input range for
        i,j and k are are upper end inclusive. To get the four
        bounding points of the lower layer of the grid:

           p0 = grid.getNodePos(0 , 0 , 0)
           p1 = grid.getNodePos(grid.getNX() , 0 , 0)
           p2 = grid.getNodePos(0 , grid.getNY() , 0)
           p3 = grid.getNodePos(grid.getNX() , grid.getNY() , 0)

        """
        if not 0 <= i <= self.getNX():
            raise IndexError("Invalid I value:%d - valid range: [0,%d]" % (i , self.getNX()))

        if not 0 <= j <= self.getNY():
            raise IndexError("Invalid J value:%d - valid range: [0,%d]" % (j , self.getNY()))

        if not 0 <= k <= self.getNZ():
            raise IndexError("Invalid K value:%d - valid range: [0,%d]" % (k , self.getNZ()))
            
        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()
        cfunc.get_corner_xyz( self , i,j,k , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z))
        return (x.value , y.value , z.value)


    def getCellCorner(self , corner_nr , active_index = None , global_index = None , ijk = None):
        """
        Will look up xyz of corner nr @corner_nr

        
        lower layer:   upper layer  
                    
         2---3           6---7
         |   |           |   |
         0---1           4---5

        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()
        cfunc.get_cell_corner_xyz1( self , gi , corner_nr , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z))
        return (x.value , y.value , z.value)


    def get_corner_xyz(self, corner_nr , active_index = None , global_index = None , ijk = None):
        warnings.warn("The get_corner_xyz() method has been renamed: getCellCorner()" , DeprecationWarning)
        return self.getCellCorner(corner_nr , active_index , global_index , ijk)


    def getNodeXYZ(self , i,j,k):
        """
        This function returns the position of Vertex (i,j,k).
    
        The coordinates are in the inclusive interval [0,nx] x [0,ny] x [0,nz].
        """
        nx = self.getNX()
        ny = self.getNY()
        nz = self.getNZ()

        corner = 0
        
        if i == nx:
            i -= 1
            corner += 1

        if j == ny:
            j -= 1
            corner += 2

        if k == nz:
            k -= 1
            corner += 4

        if cfunc.ijk_valid( self , i , j , k):
            return self.get_corner_xyz( corner , global_index = i + j*nx + k*nx*ny )
        else:
            raise IndexError("Invalid coordinates: (%d,%d,%d) " % (i,j,k))



    def getLayerXYZ(self , xy_corner , layer):
        nx = self.getNX()
        
        (j , i) = divmod(xy_corner , nx + 1)
        k = layer
        return self.getNodeXYZ(i,j,k)
        


    def distance( self , global_index1 , global_index2):
        dx = ctypes.c_double()
        dy = ctypes.c_double()
        dz = ctypes.c_double()
        cfunc.get_distance( self , global_index1 , global_index2 , ctypes.byref(dx) , ctypes.byref(dy) , ctypes.byref(dz))
        return (dx.value , dy.value , dz.value)


    def depth( self , active_index = None , global_index = None , ijk = None):
        """
        Depth of the center of a cell.

        Returns the depth of the center of the cell given by
        @active_index, @global_index or @ijk. See method get_xyz() for
        documentation of @active_index, @global_index and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        return cfunc.get_depth( self , gi )

    def top( self , i , j ):
        """
        Top of the reservoir; in the column (@i , @j).
        """
        return cfunc.get_top( self , i , j ) 

    def bottom( self , i , j ):
        """
        Bottom of the reservoir; in the column (@i , @j).
        """
        return cfunc.get_bottom( self , i , j ) 

    def locate_depth( self , depth , i , j ):
        """
        Will locate the k value of cell containing specified depth.

        Will scan through the grid column specified by the input
        arguments @i and @j and search for a cell containing the depth
        given by input argument @depth. The return value is the k
        value of cell containing @depth.

        If @depth is above the top of the reservoir the function will
        return -1, and if @depth is below the bottom of the reservoir
        the function will return -nz.
        """
        return cfunc.locate_depth( self , depth , i , j)


    def find_cell( self , x , y , z , start_ijk = None):
        """
        Lookup cell containg true position (x,y,z).

        Will locate the cell in the grid which contains the true
        position (@x,@y,@z), the return value is as a triplet
        (i,j,k). The underlying C implementation is not veeery
        efficient, and can potentially take quite long time. If you
        provide a good intial guess with the parameter @start_ijk (a
        tuple (i,j,k)) things can speed up quite substantially.

        If the location (@x,@y,@z) can not be found in the grid, the
        method will return None.
        """

        if start_ijk:
            start_index = self.__global_index( ijk = start_ijk )
        else:
            start_index = 0
        global_index = cfunc.get_ijk_xyz( self , x , y , z , start_index)
        if global_index >= 0:
            i = ctypes.c_int()
            j = ctypes.c_int()
            k = ctypes.c_int()
            cfunc.get_ijk1( self , global_index , ctypes.byref(i) , ctypes.byref(j) , ctypes.byref(k))        
            return (i.value , j.value , k.value)
        else:
            return None

    def cell_contains( self , x , y , z , active_index = None , global_index = None , ijk = None):
        """
        Will check if the cell contains point given by world
        coordinates (x,y,z).

        See method get_xyz() for documentation of @active_index,
        @global_index and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        return cfunc.cell_contains( self , gi , x,y,z)


    def findCellXY(self , x, y , k):
        """Will find the i,j of cell with utm coordinates x,y.
        
        The @k input is the layer you are interested in, the allowed
        values for k are [0,nz]. If the coordinates (x,y) are found to
        be outside the grid a ValueError exception is raised.
        """
        if 0 <= k <= self.getNZ():
            i = ctypes.c_int()
            j = ctypes.c_int()
            ok = cfunc.get_ij_xy( self , x,y,k , ctypes.byref(i) , ctypes.byref(j))
            if ok:
                return (i.value , j.value)
            else:
                raise ValueError("Could not find the point:(%g,%g) in layer:%d" % (x,y,k))
        else:
            raise IndexError("Invalid layer value:%d" % k)
        

    @staticmethod
    def d_cmp(a,b):
        return cmp(a[0] , b[0])


    def findCellCornerXY(self , x, y , k):
        """Will find the corner nr of corner closest to utm coordinates x,y.
        
        The @k input is the layer you are interested in, the allowed
        values for k are [0,nz]. If the coordinates (x,y) are found to
        be outside the grid a ValueError exception is raised.
        """
        i,j = self.findCellXY(x,y,k)
        if k == self.getNZ():
            k -= 1
            corner_shift = 4
        else:
            corner_shift = 0
        
        nx = self.getNX()
        x0,y0,z0 = self.getCellCorner( corner_shift , ijk = (i,j,k))
        d0 = math.sqrt( (x0 - x)*(x0 - x) + (y0 - y)*(y0 - y))
        c0 = i + j*(nx + 1)

        x1,y1,z1 = self.getCellCorner( 1 + corner_shift , ijk = (i,j,k))
        d1 = math.sqrt( (x1 - x)*(x1 - x) + (y1 - y)*(y1 - y))
        c1 = i + 1 + j*(nx + 1)

        x2,y2,z2 = self.getCellCorner( 2 + corner_shift , ijk = (i,j,k))
        d2 = math.sqrt( (x2 - x)*(x2 - x) + (y2 - y)*(y2 - y))
        c2 = i + (j + 1)*(nx + 1)

        x3,y3,z3 = self.getCellCorner( 3 + corner_shift , ijk = (i,j,k))
        d3 = math.sqrt( (x3 - x)*(x3 - x) + (y3 - y)*(y3 - y))
        c3 = i + 1 + (j + 1)*(nx + 1)

        l = [(d0 , c0) , (d1,c1) , (d2 , c2) , (d3,c3)]
        l.sort( EclGrid.d_cmp )
        return l[0][1]
        


    def cell_regular(self, active_index = None , global_index = None , ijk = None):
        """
        The ECLIPSE grid models often contain various degenerate cells,
        which are twisted, have overlapping corners or what not. This
        function gives a moderate sanity check on a cell, essentially
        what the function does is to check if the cell contains it's
        own centerpoint - which is actually not as trivial as it
        sounds.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        return cfunc.cell_regular( self , gi )


    def cell_volume( self, active_index = None , global_index = None , ijk = None):
        """
        Calculate the volume of a cell.

        Will calculate the total volume of the cell. See method
        get_xyz() for documentation of @active_index, @global_index
        and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        return cfunc.get_cell_volume( self , gi)
            

    def cell_dz( self , active_index = None , global_index = None , ijk = None):
        """
        The thickness of a cell.

        Will calculate the (average) thickness of the cell. See method
        get_xyz() for documentation of @active_index, @global_index
        and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index )
        return cfunc.get_cell_thickness( self , gi )


    @property
    def num_lgr( self ):
        """
        How many LGRs are attached to this main grid?

        How many LGRs are attached to this main grid; the grid
        instance doing the query must itself be a main grid.
        """
        return cfunc.num_lgr( self )


    def has_lgr( self , lgr_name ):
        """
        Query if the grid has an LGR with name @lgr_name.
        """
        if cfunc.has_lgr( self , lgr_name ):
            return True
        else:
            return False


    def get_lgr( self , lgr_name ):
        """
        Get EclGrid instance with LGR content.
        
        Return an EclGrid instance based on the LGR named
        @lgr_name. The LGR grid instance is in most questions like an
        ordinary grid instance; the only difference is that it can not
        be used for further queries about LGRs.

        If the grid does not contain an LGR with this name the method
        will return None.
        """
        if cfunc.has_lgr(self , lgr_name ):
            lgr = EclGrid( None , lgr = cfunc.get_lgr( self , lgr_name ) , parent = self)
            return lgr
        else:
            return None
        

    def get_cell_lgr( self, active_index = None , global_index = None , ijk = None):
        """
        Get EclGrid instance located in cell.
        
        Will query the current grid instance if the cell given by
        @active_index, @global_index or @ijk has been refined with an
        LGR. Will return None if the cell in question has not been
        refined, the return value can be used for further queries.
        
        See get_xyz() for documentation of the input parameters.
        """
        gi  = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        lgr = cfunc.get_cell_lgr( self , gi )
        if lgr:
            return EclGrid( None , lgr = lgr , parent = self)
        else:
            return None

    
    def grid_value( self , kw , i , j , k):
        """
        Will evalute @kw in location (@i,@j,@k).

        The ECLIPSE properties and solution vectors are stored in
        restart and init files as 1D vectors of length nx*nx*nz or
        nactive. The grid_value() method is a minor convenience
        function to convert the (@i,@j,@k) input values to an
        appropriate 1D index.

        Depending on the length of kw the input arguments are
        converted either to an active index or to a global index. If
        the length of kw does not fit with either the global size of
        the grid or the active size of the grid things will fail hard.
        """
        return cfunc.grid_value( self , kw , i , j , k)


    def load_column( self , kw , i , j , column):
        """
        Load the values of @kw from the column specified by (@i,@j).

        The method will scan through all k values of the input field
        @kw for fixed values of i and j. The size of @kw must be
        either nactive or nx*ny*nz.

        The input argument @column should be a DoubleVector instance,
        observe that if size of @kw == nactive k values corresponding
        to inactive cells will not be modified in the @column
        instance; in that case it is important that @column is
        initialized with a suitable default value.
        """
        cfunc.load_column( self , kw , i , j , column)
    

    def createKW( self , array , kw_name , pack):
        """
        Creates an EclKW instance based on existing 3D numpy object.

        The method create3D() does the inverse operation; creating a
        3D numpy object from an EclKW instance. If the argument @pack
        is true the resulting keyword will have length 'nactive',
        otherwise the element will have length nx*ny*nz.
        """
        if array.ndim == 3:
            dims = array.shape
            if dims[0] == self.nx and dims[1] == self.ny and dims[2] == self.nz:
                dtype = array.dtype
                if dtype == numpy.int32:
                    type = EclTypeEnum.ECL_INT_TYPE
                elif dtype == numpy.float32:
                    type = EclTypeEnum.ECL_FLOAT_TYPE
                elif dtype == numpy.float64:
                    type = EclTypeEnum.ECL_DOUBLE_TYPE
                else:
                    sys.exit("Do not know how to create ecl_kw from type:%s" % dtype)
  
                if pack:
                    size = self.nactive
                else:
                    size = self.size
                    
                if len(kw_name) > 8:
                    # Silently truncate to length 8 - ECLIPSE has it's challenges.
                    kw_name = kw_name[0:8]  

                kw = EclKW.new( kw_name , size , type )
                active_index = 0
                global_index = 0
                for k in range( self.nz ):
                    for j in range( self.ny ):
                        for i in range( self.nx ):
                            if pack:
                                if self.active( global_index = global_index ):
                                    kw[active_index] = array[i,j,k]
                                    active_index += 1
                            else:
                                if dtype == numpy.int32:
                                    kw[global_index] = int( array[i,j,k] )
                                else:
                                    kw[global_index] = array[i,j,k]
                                
                            global_index += 1
                return kw
        raise ValueError("Wrong size / dimension on array")

    
    def coarse_groups(self):
        """
        Will return the number of coarse groups in this grid. 
        """
        return cfunc.num_coarse_groups( self )


    def in_coarse_group(self , global_index = None , ijk = None , active_index = None):
        """
        Will return True or False if the cell is part of coarse group.
        """
        global_index = self.__global_index( active_index = active_index , ijk = ijk , global_index = global_index)
        return cfunc.in_coarse_group1( self , global_index )


    def create3D( self , ecl_kw , default = 0):
        """
        Creates a 3D numpy array object with the data from  @ecl_kw.

        Observe that 3D numpy object is a copy of the data in the
        EclKW instance, i.e. modification to the numpy object will not
        be reflected in the ECLIPSE keyword.

        The methods createKW() does the inverse operation; creating an
        EclKW instance from a 3D numpy object.

        Alternative: Creating the numpy array object is not very
        efficient; if you only need a limited number of elements from
        the ecl_kw instance it might be wiser to use the grid_value()
        method:

           value = grid.grid_value( ecl_kw , i , j , k )
           
        """
        if ecl_kw.size == self.nactive or ecl_kw.size == self.size:
            array = numpy.ones( [ self.nx , self.ny , self.nz] , dtype = ecl_kw.dtype) * default
            array = numpy.ones( [ self.size ] , dtype = ecl_kw.dtype) * default
            kwa = ecl_kw.array
            if ecl_kw.size == self.size:
                for i in range(kwa.size):
                    array[i] = kwa[i]
            else:
                data_index = 0
                for global_index in range(self.size):
                    if self.active( global_index = global_index ):
                        array[global_index] = kwa[data_index]
                        data_index += 1
                        
            array = array.reshape( [self.nx , self.ny , self.nz] , order = 'F')
            return array
        else:
            raise ValueError("Keyword: %s has invalid size(%d), must be either nactive:%d  or nx*ny*nz:%d" % (ecl_kw.name , ecl_kw.size , self.nactive ,self.size))
        
    def save_grdecl(self , pyfile):
        """
        Will write the the grid content as grdecl formatted keywords.

        Will only write the main grid.
        """
        cfile = CFILE( pyfile )
        cfunc.fprintf_grdecl( self , cfile )

    def save_EGRID( self , filename , output_metric = True):
        """
        Will save the current grid as a EGRID file.
        """
        cfunc.fwrite_EGRID( self , filename, output_metric )

    def save_GRID( self , filename ):
        """
        Will save the current grid as a EGRID file.
        """
        cfunc.fwrite_GRID( self , filename )

        
    def write_grdecl( self , ecl_kw , pyfile , special_header = None , default_value = 0):
        """
        Writes an EclKW instance as an ECLIPSE grdecl formatted file.

        The input argument @ecl_kw must be an EclKW instance of size
        nactive or nx*ny*nz. If the size is nactive the inactive cells
        will be filled with @default_value; hence the function will
        always write nx*ny*nz elements. 
        
        The data in the @ecl_kw argument can be of type integer,
        float, double or bool. In the case of bool the default value
        must be specified as 1 (True) or 0 (False).

        The input argument @pyfile should be a valid python filehandle
        opened for writing; i.e.

           pyfile = open("PORO.GRDECL" , "w")
           grid.write_grdecl( poro_kw  , pyfile , default_value = 0.0)
           grid.write_grdecl( permx_kw , pyfile , default_value = 0.0)
           pyfile.close()

        """
        
        if ecl_kw.size == self.nactive or ecl_kw.size == self.size:
            cfile = CFILE( pyfile )
            cfunc.fwrite_grdecl( self , ecl_kw , special_header , cfile , default_value )
        else:
            raise ValueError("Keyword: %s has invalid size(%d), must be either nactive:%d  or nx*ny*nz:%d" % (ecl_kw.name , ecl_kw.size , self.nactive , self.size))


    def exportACTNUM(self):
        actnum = IntVector( initial_size = self.getGlobalSize() )
        cfunc.init_actnum( self , actnum.getDataPtr() )
        return actnum


    def compressedKWCopy(self, kw):
        if len(kw) == self.getNumActive():
            return EclKW.copy( kw )
        elif len(kw) == self.getGlobalSize():
            kw_copy = EclKW.create( kw.getName() , self.getNumActive() , kw.getEclType())
            cfunc.compressed_kw_copy( self , kw_copy , kw)
            return kw_copy
        else:
            raise ValueError("The input keyword must have nx*n*nz or nactive elements. Size:%d invalid" % len(kw))

    def globalKWCopy(self, kw , default_value):
        if len(kw) == self.getGlobalSize( ):
            return EclKW.copy( kw )
        elif len(kw) == self.getNumActive():
            kw_copy = EclKW.create( kw.getName() , self.getGlobalSize() , kw.getEclType())
            kw_copy.assign( default_value )
            cfunc.global_kw_copy( self , kw_copy , kw)
            return kw_copy
        else:
            raise ValueError("The input keyword must have nx*n*nz or nactive elements. Size:%d invalid" % len(kw))

        

# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : ecl_kw <-> EclKW
cwrapper = CWrapper(ECL_LIB)
cwrapper.registerType( "ecl_grid" , EclGrid )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ecl_grid")



cfunc.fread_alloc                  = cwrapper.prototype("c_void_p ecl_grid_load_case( char* )")
cfunc.grdecl_create                = cwrapper.prototype("c_void_p ecl_grid_alloc_GRDECL_kw( int , int , int , ecl_kw , ecl_kw , ecl_kw , ecl_kw)") 
cfunc.get_lgr                      = cwrapper.prototype("c_void_p ecl_grid_get_lgr( ecl_grid , char* )")
cfunc.get_cell_lgr                 = cwrapper.prototype("c_void_p ecl_grid_get_cell_lgr1( ecl_grid , int )")
cfunc.alloc_rectangular            = cwrapper.prototype("c_void_p ecl_grid_alloc_rectangular( int , int , int , double , double , double , int*)")

cfunc.num_coarse_groups            = cwrapper.prototype("int  ecl_grid_get_num_coarse_groups( ecl_grid )")
cfunc.in_coarse_group1             = cwrapper.prototype("bool ecl_grid_cell_in_coarse_group1( ecl_grid , int)")

cfunc.exists                       = cwrapper.prototype("bool ecl_grid_exists( char* )")
cfunc.free                         = cwrapper.prototype("void ecl_grid_free( ecl_grid )")     
cfunc.get_nx                       = cwrapper.prototype("int ecl_grid_get_nx( ecl_grid )")
cfunc.get_ny                       = cwrapper.prototype("int ecl_grid_get_ny( ecl_grid )")
cfunc.get_nz                       = cwrapper.prototype("int ecl_grid_get_nz( ecl_grid )")
cfunc.get_global_size              = cwrapper.prototype("int ecl_grid_get_global_size( ecl_grid )")
cfunc.get_active                   = cwrapper.prototype("int ecl_grid_get_active_size( ecl_grid )")
cfunc.get_active_fracture          = cwrapper.prototype("int ecl_grid_get_nactive_fracture( ecl_grid )")
cfunc.get_name                     = cwrapper.prototype("char* ecl_grid_get_name( ecl_grid )")
cfunc.ijk_valid                    = cwrapper.prototype("bool ecl_grid_ijk_valid(ecl_grid , int , int , int)")
cfunc.get_active_index3            = cwrapper.prototype("int ecl_grid_get_active_index3( ecl_grid , int , int , int)")
cfunc.get_global_index3            = cwrapper.prototype("int ecl_grid_get_global_index3( ecl_grid , int , int , int)") 
cfunc.get_active_index1            = cwrapper.prototype("int ecl_grid_get_active_index1( ecl_grid , int )") 
cfunc.get_active_fracture_index1   = cwrapper.prototype("int ecl_grid_get_active_fracture_index1( ecl_grid , int )") 
cfunc.get_global_index1A           = cwrapper.prototype("int ecl_grid_get_global_index1A( ecl_grid , int )") 
cfunc.get_global_index1F           = cwrapper.prototype("int ecl_grid_get_global_index1F( ecl_grid , int )") 
cfunc.get_ijk1                     = cwrapper.prototype("void ecl_grid_get_ijk1( ecl_grid , int , int* , int* , int*)")
cfunc.get_ijk1A                    = cwrapper.prototype("void ecl_grid_get_ijk1A( ecl_grid , int , int* , int* , int*)") 
cfunc.get_xyz3                     = cwrapper.prototype("void ecl_grid_get_xyz3( ecl_grid , int , int , int , double* , double* , double*)")
cfunc.get_xyz1                     = cwrapper.prototype("void ecl_grid_get_xyz1( ecl_grid , int , double* , double* , double*)")
cfunc.get_cell_corner_xyz1         = cwrapper.prototype("void ecl_grid_get_cell_corner_xyz1( ecl_grid , int , int , double* , double* , double*)")
cfunc.get_corner_xyz               = cwrapper.prototype("void ecl_grid_get_corner_xyz( ecl_grid , int , int , int, double* , double* , double*)")
cfunc.get_xyz1A                    = cwrapper.prototype("void ecl_grid_get_xyz1A( ecl_grid , int , double* , double* , double*)")
cfunc.get_ij_xy                    = cwrapper.prototype("bool ecl_grid_get_ij_from_xy( ecl_grid , double , double , int , int* , int*)")
cfunc.get_ijk_xyz                  = cwrapper.prototype("int  ecl_grid_get_global_index_from_xyz( ecl_grid , double , double , double , int)")
cfunc.cell_contains                = cwrapper.prototype("bool ecl_grid_cell_contains_xyz1( ecl_grid , int , double , double , double )")
cfunc.cell_regular                 = cwrapper.prototype("bool ecl_grid_cell_regular1( ecl_grid , int)")
cfunc.num_lgr                      = cwrapper.prototype("int  ecl_grid_get_num_lgr( ecl_grid )")
cfunc.has_lgr                      = cwrapper.prototype("bool ecl_grid_has_lgr( ecl_grid , char* )")
cfunc.grid_value                   = cwrapper.prototype("double ecl_grid_get_property( ecl_grid , ecl_kw , int , int , int)")
cfunc.get_cell_volume              = cwrapper.prototype("double ecl_grid_get_cell_volume1( ecl_grid , int )")
cfunc.get_cell_thickness           = cwrapper.prototype("double ecl_grid_get_cell_thickness1( ecl_grid , int )")
cfunc.get_depth                    = cwrapper.prototype("double ecl_grid_get_cdepth1( ecl_grid , int )")
cfunc.fwrite_grdecl                = cwrapper.prototype("void   ecl_grid_grdecl_fprintf_kw( ecl_grid , ecl_kw , char* , FILE , double)") 
cfunc.load_column                  = cwrapper.prototype("void   ecl_grid_get_column_property( ecl_grid , ecl_kw , int , int , double_vector)")
cfunc.get_top                      = cwrapper.prototype("double ecl_grid_get_top2( ecl_grid , int , int )") 
cfunc.get_bottom                   = cwrapper.prototype("double ecl_grid_get_bottom2( ecl_grid , int , int )") 
cfunc.locate_depth                 = cwrapper.prototype("int    ecl_grid_locate_depth( ecl_grid , double , int , int )") 
cfunc.invalid_cell                 = cwrapper.prototype("bool   ecl_grid_cell_invalid1( ecl_grid , int)")
cfunc.get_distance                 = cwrapper.prototype("void   ecl_grid_get_distance( ecl_grid , int , int , double* , double* , double*)")
cfunc.fprintf_grdecl               = cwrapper.prototype("void   ecl_grid_fprintf_grdecl( ecl_grid , FILE) ")
cfunc.fwrite_GRID                  = cwrapper.prototype("void   ecl_grid_fwrite_GRID( ecl_grid , char* )")
cfunc.fwrite_EGRID                 = cwrapper.prototype("void   ecl_grid_fwrite_EGRID( ecl_grid , char*, bool )")
cfunc.equal                        = cwrapper.prototype("bool   ecl_grid_compare(ecl_grid , ecl_grid , bool, bool)")
cfunc.dual_grid                    = cwrapper.prototype("bool   ecl_grid_dual_grid( ecl_grid )")
cfunc.init_actnum                  = cwrapper.prototype("void   ecl_grid_init_actnum_data( ecl_grid , int* )")
cfunc.compressed_kw_copy           = cwrapper.prototype("void   ecl_grid_compressed_kw_copy( ecl_grid , ecl_kw , ecl_kw)")
cfunc.global_kw_copy               = cwrapper.prototype("void   ecl_grid_global_kw_copy( ecl_grid , ecl_kw , ecl_kw)")
