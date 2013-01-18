#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_region.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Module used to select cells based on many different criteria.

This module implements the class EclRegion which can be used to select
cells in a grid matching a criteria. A wide range of different
criteria are supported. Many of the special functions for implementing
mathematical operations are implemented, so that regions can be
combined e.g. with logical &.

When the selection process is complete the region instance can be
queried for the corresponding list of indices.
"""
import warnings
import libecl
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass
from   ert.util.tvector      import IntVector
from   ert.geo.geo_polygon   import GeoPolygon
from   ecl_kw                import ECL_INT_TYPE , ECL_FLOAT_TYPE , ECL_DOUBLE_TYPE, EclKW
import ecl_grid


class EclRegion(CClass):
    
    
    def __init__(self , grid , preselect , c_ptr = None):
        """
        Create a new region selector for cells in @grid.

        Will create a new region selector to select and deselect the
        cells in the grid given by @grid. The input argument @grid
        should be a EclGrid instance. You can start with either all
        cells, or no cells, selected, depending on the value of
        @preselect.
        """

        self.grid = grid
        self.active_index = False
        if not c_ptr:
            c_ptr = cfunc.alloc( grid , preselect )
        self.init_cobj( c_ptr , cfunc.free )


            
    def __deep_copy__(self , memo):
        """
        Creates a deep copy of the current region.
        """
        return EclRegion( self.grid , False , c_ptr = cfunc.alloc_copy( self ))


    def __iand__(self , other):
        """
        Will perform set intersection operation inplace.

        Will update the current region selection so that the elements
        selected in self are also selected in @other. Bound to the
        inplace & operator, i.e.

           reg1 &= reg2
           
        will eventually call this method.
        """
        if isinstance(other , EclRegion):
            cfunc.intersect( self, other)
        else:
            raise TypeError("Ecl region can only intersect with other EclRegion instances")
            
        return self


    def __isub__(self , other):
        """
        Inplace "subtract" one selection from another.

        Bound to reg -= reg2
        """
        if isinstance( other , EclRegion ):
            cfunc.subtract( self , other)
        else:
            raise TypeError("Ecl region can only subtract with other EclRegion instances")

        return self
        

    def __ior__(self , other):
        """
        Will perform set operation union in place.

        The current region selection will be updated to contain all
        the elements which are selected either in the current region,
        or in @other; bound to to inplace | operator, so you can write e.g.

            reg1 |= reg2

        to update reg1 with the selections from reg2.
        """
        if isinstance( other , EclRegion):
            cfunc.combine( self, other)
        else:
            raise TypeError("Ecl region can only be combined with other EclRegion instances")
            
        return self

    def __iadd__(self , other):
        """
        Combines to regions - see __ior__().
        """
        return self.__ior__( other )

    def __or__(self , other):
        """
        Creates a new region which is the union of @self and other.

        The method will create a new region which selection status is
        given by the logical or of regions @self and @other; the two
        initial regions will not be modified. Bound to the unary |
        operator:

            new_reg = reg1 | reg2
            
        """
        new_region = self.copy()
        new_region.__ior__( other )
        return new_region

    def __and__(self , other):
        """
        Creates a new region which is the intersection of @self and other.

        The method will create a new region which selection status is
        given by the logical and of regions @self and @other; the two
        initial regions will not be modified. Bound to the unary &
        operator:

            new_reg = reg1 & reg2
        """
        new_region = self.copy()
        new_region.__iand__( other )
        return new_region

    def __add__(self , other):
        """
        Unary add operator for two regions - implemented by __or__().
        """
        return self.__or__( other )


    def __sub__( self, other):
        """
        Unary del operator for two regions.
        """
        new_region = self.copy()
        new_region.__isub__( other )
        return new_region


    def union_with( self, other):
        """
        Will update self with the union of @self and @other.
        
        See doscumentation of __ior__(). 
        """
        return self.__ior__( other )

    def intersect_with( self, other):
        """
        Will update self with the intersection of @self and @other.
        
        See doscumentation of __iand__(). 
        """
        return self.__iand__( other )
    

    def copy( self ):
        return self.__deep_copy__( {} )

    def reset(self):
        """
        Clear selections according to constructor argument @preselect.

        Will clear all selections, depending on the value of the
        constructor argument @preselect. If @preselect is true
        everything will be selected after calling reset(), otherwise
        no cells will be selected after calling reset().
        """
        cfunc.reset( self )

    ##################################################################

    
    def select_method(select):
        """
        The select_method method decorator is applied to all the
        select_xxx() methods. The purpose of this decorator is to
        allow the select_xxx() methods to have an optional argument
        @intersect. If the @intersect argument is True the results of
        the current select method will be and'ed with the current
        selection, instead of or'ed which is the default.

        Consider this example:

           region = EclRegion( grid , False )
           region.select_islice(0 , 5)     # Selects all cells with i:[0:5]
           region.select_jslice(0 , 5)     # Selects all cells with j:[0:5]

        When these two calls have completed selection will contain all
        the cells which are either in i-interval [0:5] or in
        j-interval [0:5]. If we supply the @intersect argument in the
        second call the j selection will only be applied to the cells
        in i:[0:5] interval:

           region = EclRegion( grid , False )
           region.select_islice(0 , 5)     # Selects all cells with i:[0:5]
           region.select_jslice(0 , 5)     # Selects all cells with j:[0:5] AND i:[0:5]
        """
        
        def select_wrapper(self , *args ,  **kwargs):
            intersect = kwargs.has_key('intersect') and kwargs['intersect']
            if intersect:
                new_region = EclRegion( self.grid , False )
                select(new_region , *args )
                
                self &= new_region
            else:
                select(self , *args )

        return select_wrapper

            
    @select_method
    def select_more( self , ecl_kw , limit , intersect = False):
        """
        Select all cells where keyword @ecl_kw is above @limit.

        This method is used to select all the cells where an arbitrary
        field, contained in @ecl_kw, is above a limiting value
        @limit. The EclKW instance must have either nactive or
        nx*ny*nz elements; if this is not satisfied method will fail
        hard. The datatype of @ecl_kw must be numeric,
        i.e. ECL_INT_TYPE, ECL_DOUBLE_TYPE or ECL_FLOAT_TYPE. In the
        example below we select all the cells with water saturation
        above 0.85:

           restart_file = ecl.EclFile( "ECLIPSE.X0067" )
           swat_kw = restart_file["SWAT"][0]
           grid = ecl.EclGrid( "ECLIPSE.EGRID" )
           region = ecl.EclRegion( grid , False )
           
           region.select_more( swat_kw , 0.85 )

        """
        cfunc.select_more( self , ecl_kw , limit )

        

    def deselect_more( self , ecl_kw , limit):
        """
        Deselects cells with value above limit.

        See select_more() for further documentation.
        """
        cfunc.deselect_more( self , ecl_kw , limit )

    @select_method
    def select_less( self , ecl_kw , limit , intersect = False):
        """
        Select all cells where keyword @ecl_kw is below @limit.
        
        See select_more() for further documentation.
        """
        cfunc.select_less( self , ecl_kw , limit )

    def deselect_less( self , ecl_kw , limit):
        """
        Deselect all cells where keyword @ecl_kw is below @limit.
        
        See select_more() for further documentation.
        """
        cfunc.deselect_less( self , ecl_kw , limit )

    @select_method
    def select_equal( self , ecl_kw , value , intersect = False):
        """
        Select all cells where @ecl_kw is equal to @value.

        The EclKW instance @ecl_kw must be of size nactive or
        nx*ny*nz, and it must be of integer type; testing for equality
        is not supported for floating point numbers. In the example
        below we select all the cells in PVT regions 2 and 4:

           init_file = ecl.EclFile( "ECLIPSE.INIT" )
           pvtnum_kw = init_file.iget_named_kw( "PVTNUM" , 0 )
           grid = ecl.EclGrid( "ECLIPSE.GRID" )
           region = ecl.EclRegion( grid , False )
           
           region.select_equal( pvtnum_kw , 2 )
           region.select_equal( pvtnum_kw , 4 )

        """
        cfunc.select_equal( self , ecl_kw , value )


    def deselect_equal( self , ecl_kw , value ):
        """
        Select all cells where @ecl_kw is equal to @value.

        See select_equal() for further documentation.
        """
        cfunc.deselect_equal( self , ecl_kw , value )

    @select_method
    def select_in_range( self , ecl_kw , lower_limit , upper_limit , select = False):
        """
        Select all cells where @ecl_kw is in the half-open interval [ , ).

        Will select all the cells where EclKW instance @ecl_kw has
        value in the half-open interval [@lower_limit ,
        @upper_limit). The input argument @ecl_kw must have size
        nactive or nx*ny*nz, and it must be of type ECL_FLOAT_TYPE. 
        
        The following example will select all cells with porosity in
        the range [0.15,0.20):

           init_file = ecl.EclFile( "ECLIPSE.INIT" )
           poro_kw = init_file.iget_named_kw( "PORO" , 0 )
           grid = ecl.EclGrid( "ECLIPSE.GRID" )
           region = ecl.EclRegion( grid , False )
           
           region.select_in_range( poro_kw , 0.15, 0.20 )
          
        """
        cfunc.select_in_interval( self , ecl_kw , lower_limit , upper_limit)

    def deselect_in_range( self , ecl_kw , lower_limit , upper_limit):
        """
        Deselect all cells where @ecl_kw is in the half-open interval [ , ).

        See select_in_range() for further documentation.
        """
        cfunc.deselect_in_interval( self , ecl_kw , lower_limit , upper_limit)


    @select_method
    def select_cmp_less( self , kw1 , kw2 , intersect = False):
        """
        Will select all cells where kw2 < kw1.

        Will compare the ECLIPSE keywords @kw1 and @kw2, and select
        all the cells where the numerical value of @kw1 is less than
        the numerical value of @kw2. The ECLIPSE keywords @kw1 and
        @kw2 must both be of the same size, nactive or nx*ny*nz. In
        addition they must both be of type type ECL_FLOAT_TYPE. In the
        example below we select all the cells where the pressure has
        dropped:

           restart_file = ecl.EclFile("ECLIPSE.UNRST")
           pressure1 = restart_file.iget_named_kw( "PRESSURE" , 0)
           pressure2 = restart_file.iget_named_kw( "PRESSURE" , 100)

           region.select_cmp_less( pressure2 , pressure1)

        """
        cfunc.select_cmp_less( self , kw1 , kw2 )

    def deselect_cmp_less( self , kw1 , kw2):
        """
        Will deselect all cells where kw2 < kw1.

        See select_cmp_less() for further documentation.
        """
        cfunc.deselect_cmp_less( self , kw1 , kw2 )

    @select_method
    def select_cmp_more( self , kw1 , kw2 , intersect = False):
        """
        Will select all cells where kw2 > kw1.
        
        See select_cmp_less() for further documentation.
        """
        cfunc.select_cmp_more( self , kw1 , kw2 )

    def deselect_cmp_more( self , kw1 , kw2):
        """
        Will deselect all cells where kw2 > kw1.
        
        See select_cmp_less() for further documentation.
        """
        cfunc.deselect_cmp_more( self , kw1 , kw2 )

    @select_method
    def select_active( self , intersect = False):
        """
        Will select all the active grid cells.
        """
        cfunc.select_active( self )

    def deselect_active( self ):
        """
        Will deselect all the active grid cells.
        """
        cfunc.deselect_active( self )

    @select_method
    def select_inactive( self , intersect = False):
        """
        Will select all the inactive grid cells.
        """
        cfunc.select_inactive( self )


    def deselect_inactive( self ):
        """
        Will deselect all the inactive grid cells.
        """
        cfunc.deselect_inactive( self )


    def select_all( self ):
        """
        Will select all the cells.
        """
        cfunc.select_all( self )

    def deselect_all( self ):
        """
        Will deselect all the cells.
        """
        cfunc.deselect_all( self )

    def clear( self ):
        """
        Will deselect all cells.
        """
        self.deselect_all()

    @select_method 
    def select_deep( self, depth , intersect = False):
        """
        Will select all cells below @depth.
        """
        cfunc.select_deep_cells(self , depth)

    def deselect_deep( self, depth):
        """
        Will deselect all cells below @depth.
        """
        cfunc.deselect_deep_cells(self , depth)

    @select_method 
    def select_shallow( self, depth , intersect = False):
        """
        Will select all cells above @depth.
        """
        cfunc.select_shallow_cells(self , depth)

    def deselect_shallow( self, depth):
        """
        Will deselect all cells above @depth.
        """
        cfunc.deselect_shallow_cells(self , depth)

    @select_method
    def select_small( self , size_limit , intersect = False):
        """
        Will select all cells smaller than @size_limit.
        """
        cfunc.select_small( self , size_limit )

    def deselect_small( self , size_limit ):
        """
        Will deselect all cells smaller than @size_limit.
        """
        cfunc.deselect_small( self , size_limit )

    @select_method
    def select_large( self , size_limit , intersect = False):
        """
        Will select all cells larger than @size_limit.
        """
        cfunc.select_large( self , size_limit )

    def deselect_large( self , size_limit ):
        """
        Will deselect all cells larger than @size_limit.
        """
        cfunc.deselect_large( self , size_limit )

    @select_method
    def select_thin( self , size_limit , intersect = False):
        """
        Will select all cells thinner than @size_limit.
        """
        cfunc.select_thin( self , size_limit )

    def deselect_thin( self , size_limit ):
        """
        Will deselect all cells thinner than @size_limit.
        """
        cfunc.deselect_thin( self , size_limit )

    @select_method
    def select_thick( self , size_limit , intersect = False):
        """
        Will select all cells thicker than @size_limit.
        """
        cfunc.select_thick( self , size_limit )

    def deselect_thick( self , size_limit ):
        """
        Will deselect all cells thicker than @size_limit.
        """
        cfunc.deselect_thick( self , size_limit )

    @select_method
    def select_box( self , ijk1 , ijk2 , intersect = False):
        """
        Will select all cells in box.

        Will select all the the cells in the box given by @ijk1 and
        @ijk2. The two arguments @ijk1 and @ijk2 are tuples (1,j,k)
        representing two arbitrary - diagonally opposed corners - of a
        box. All the elements in @ijk1 and @ijk2 are inclusive, i.e.

           select_box( (10,12,8) , (8 , 16,4) )

        will select the box defined by [8,10] x [12,16] x [4,8].
        """
        cfunc.select_box( self , ijk1[0] , ijk2[0] , ijk1[1] , ijk2[1] , ijk1[2] , ijk2[2])

    def deselect_box( self , ijk1 , ijk2 ):
        """
        Will deselect all elements in box.
        
        See select_box() for further documentation.
        """
        cfunc.deselect_box( self , ijk1[0] , ijk2[0] , ijk1[1] , ijk2[1] , ijk1[2] , ijk2[2])

    @select_method
    def select_islice( self , i1 , i2, intersect = False):
        """
        Will select all cells with i in [@i1, @i2]. @i1 and @i2 are zero offset.
        """
        cfunc.select_islice( self , i1,i2)

    def deselect_islice( self , i1 , i2):
        """
        Will deselect all cells with i in [@i1, @i2]. @i1 and @i2 are zero offset.
        """
        cfunc.deselect_islice( self , i1,i2)

    @select_method
    def select_jslice( self , j1 , j2 , intersect = False):
        """
        Will select all cells with j in [@j1, @j2]. @i1 and @i2 are zero offset.
        """
        cfunc.select_jslice( self , j1,j2)

    def deselect_jslice( self , j1 , j2):
        """
        Will deselect all cells with j in [@j1, @j2]. @i1 and @i2 are zero offset.
        """
        cfunc.deselect_jslice( self , j1,j2)

    @select_method
    def select_kslice( self , k1 , k2 , intersect = False):
        """
        Will select all cells with k in [@k1, @k2]. @i1 and @i2 are zero offset. 
        """
        cfunc.select_kslice( self , k1,k2)

    def deselect_kslice( self , k1 , k2):
        """
        Will deselect all cells with k in [@k1, @k2]. @i1 and @i2 are zero offset.
        """
        cfunc.deselect_kslice( self , k1,k2)

    def invert( self ):
        """
        Will invert the current selection.
        """
        cfunc.invert_selection( self )

    def __init_plane_select( self , n , p ):
        n_vec = ctypes.cast( (ctypes.c_double * 3)() , ctypes.POINTER( ctypes.c_double ))
        p_vec = ctypes.cast( (ctypes.c_double * 3)() , ctypes.POINTER( ctypes.c_double ))
        for i in range(3):
            n_vec[i] = n[i]
            p_vec[i] = p[i]
        return ( n_vec , p_vec )

    @select_method
    def select_above_plane( self , n , p , intersect = False):
        """
        Will select all the cells 'above' the plane defined by n & p.

        @n is the surface normal vector of the plane in question and
        @p is a point on the plane surface. The point @p should be
        given in (utm_x , utm_y , tvd) coordinates. The term 'above'
        means that the cell center has a positive distance to the
        plain; correspondingly 'below' means that the cell center has
        a negative disatnce to the plane.
        """
        (n_vec , p_vec) = self.__init_plane_select( n , p )
        cfunc.select_above_plane( self , n_vec , p_vec )

    @select_method
    def select_below_plane( self , n , p , interscet = False):
        """
        Will select all the cells 'below' the plane defined by n & p.

        See method 'select_above_plane' for further documentation.
        """
        (n_vec , p_vec) = self.__init_plane_select( n , p )
        cfunc.select_below_plane( self , n_vec , p_vec )

    def deselect_above_plane( self , n , p):
        """
        Will deselect all the cells 'above' the plane defined by n & p.

        See method 'select_above_plane' for further documentation.
        """
        (n_vec , p_vec) = self.__init_plane_deselect( n , p )
        cfunc.deselect_above_plane( self , n_vec , p_vec )

    def deselect_below_plane( self , n , p):
        """
        Will deselect all the cells 'below' the plane defined by n & p.
        
        See method 'select_above_plane' for further documentation.
        """
        (n_vec , p_vec) = self.__init_plane_deselect( n , p )
        cfunc.deselect_below_plane( self , n_vec , p_vec )

    @select_method
    def select_inside_polygon( self , points , intersect = False):
        """
        Will select all points inside polygon.

        Will select all points inside polygon specified by input
        variable @points. Points should be a list of two-element
        tuples (x,y). So to select all the points within the rectangle
        bounded by the lower left rectangle (0,0) and upper right
        (100,100) the @points list should be:

           points = [(0,0) , (0,100) , (100,100) ,  (100,0)]

        The elements in the points list should be (utm_x, utm_y)
        values. These values will be compared with the centerpoints of
        the cells in the grid. The selection is based the top k=0
        layer, and then extending this selection to all k values; this
        implies that the selection polygon will effectively be
        translated if the pillars are not vertical.
        """
        cfunc.select_inside_polygon( self , GeoPolygon( points ))

    @select_method
    def select_outside_polygon( self , points , intersect = False):
        """
        Will select all points outside polygon.

        See select_inside_polygon for more docuemntation.
        """
        cfunc.select_outside_polygon( self , GeoPolygon( points ))

    def deselect_outside_polygon( self , points ):
        """
        Will select all points outside polygon.

        See select_inside_polygon for more docuemntation.
        """
        cfunc.deselect_inside_polygon( self , GeoPolygon( points ))

    def deselect_outside_polygon( self , points ):
        """
        Will select all points outside polygon.

        See select_inside_polygon for more docuemntation.
        """
        cfunc.deselect_outside_polygon( self , GeoPolygon( points ))





    #################################################################

    def scalar_apply_kw( self , target_kw , scalar , func_dict , force_active = False):
        """
        Helper function to apply a function with one scalar arg on target_kw.
        """
        type = target_kw.type
        if func_dict.has_key( type ):
            func = func_dict[ type ]
            func( self , target_kw, scalar , force_active )
        else:
            raise Exception("scalar_apply_kw() only supported for INT/FLOAT/DOUBLE")
        
    def iadd_kw( self , target_kw , delta_kw , force_active = False):
        """
        The functions iadd_kw(), copy_kw(), set_kw(), scale_kw() and
        shift_kw() are not meant to be used as methods of the
        EclRegion class (altough that is of course perfectly OK) -
        rather a EclRegion instance is passed as an argument to an
        EclKW method, and then that method "flips things around" and
        calls one of these methods with the EclKW instance as
        argument. This applies to all the EclKW methods which take an
        optional "mask" argument.
        """
        if isinstance(delta_kw , EclKW):
            if target_kw.assert_binary( delta_kw ):
                cfunc.iadd_kw( self , target_kw , delta_kw , force_active )
            else:
                raise TypeError("Type mismatch")
        else:
            self.shift_kw( target_kw , delta_kw , force_active = force_active)


    def shift_kw( self , ecl_kw , shift , force_active = False):
        """
        See usage documentation on iadd_kw().
        """
        self.scalar_apply_kw( ecl_kw , shift , {ECL_INT_TYPE    : cfunc.shift_kw_int, 
                                                ECL_FLOAT_TYPE  : cfunc.shift_kw_float , 
                                                ECL_DOUBLE_TYPE : cfunc.shift_kw_double} , force_active)

    def isub_kw( self , target_kw , delta_kw , force_active = False):
        if isinstance(delta_kw , EclKW):
            if target_kw.assert_binary( delta_kw ):
                cfunc.isub_kw( self , target_kw , delta_kw , force_active )
            else:
                raise TypeError("Type mismatch")
        else:
            self.shift_kw( target_kw , -delta_kw , force_active = force_active)
        

    def scale_kw( self , ecl_kw , scale , force_active = False):
        """
        See usage documentation on iadd_kw().
        """
        self.scalar_apply_kw( ecl_kw , scale , {ECL_INT_TYPE    : cfunc.scale_kw_int, 
                                                ECL_FLOAT_TYPE  : cfunc.scale_kw_float , 
                                                ECL_DOUBLE_TYPE : cfunc.scale_kw_double} , force_active)

    def imul_kw( self , target_kw , other , force_active = False):
        if isinstance(other , EclKW):
            if target_kw.assert_binary( other):
                cfunc.imul_kw( self , target_kw , other )
            else:
                raise TypeError("Type mismatch")
        else:
            self.scale_kw( target_kw , other , force_active )


    def idiv_kw( self , target_kw , other , force_active = False):
        if isinstance(other , EclKW):
            if target_kw.assert_binary( other):
                cfunc.idiv_kw( self , target_kw , other )
            else:
                raise TypeError("Type mismatch")
        else:
            self.scale_kw( target_kw , 1/other , force_active )


    def copy_kw( self , target_kw , src_kw , force_active = False):
        """
        See usage documentation on iadd_kw().
        """
        if target_kw.assert_binary( src_kw ):
            cfunc.copy_kw( self , target_kw , src_kw , force_active )
        else:
            raise TypeError("Type mismatch")
        
    def set_kw( self , ecl_kw , value , force_active = False):
        """
        See usage documentation on iadd_kw().
        """
        self.scalar_apply_kw( ecl_kw , value , {ECL_INT_TYPE    : cfunc.set_kw_int, 
                                                ECL_FLOAT_TYPE  : cfunc.set_kw_float , 
                                                ECL_DOUBLE_TYPE : cfunc.set_kw_double} , force_active)

    


    #################################################################

    def ecl_region_instance( self ):
        """
        Helper function (attribute) to support run-time typechecking.
        """
        return True

    

    @property
    def active_list(self):
        """
        IntVector instance with active indices in the region.
        """
        c_ptr = cfunc.get_active_list( self )
        active_list = IntVector.ref( c_ptr , self )
        return active_list

    @property
    def global_list(self):
        """
        IntVector instance with global indices in the region.
        """
        c_ptr = cfunc.get_global_list( self )
        global_list = IntVector.ref( c_ptr , self )    
        return global_list

    @property
    def active_size( self ):
        """
        Number of active cells in region.
        """
        warnings.warn("The active_size property is deprecated - use \'active_list.size\' instead." , DeprecationWarning)
        return self.active_list.size

    @property
    def global_size( self ):
        """
        Number of global cells in region.
        """
        warnings.warn("The global_size property is deprecated - use \'global_list.size\' instead." , DeprecationWarning)
        return self.global_list.size    

    def contains_ijk( self , i,j,k):
        """
        Will check if the cell given by i,j,k is part of the region.
        """
        return cfunc.contains_ijk( self , i , j , k )


    def contains_global( self , global_index):
        """
        Will check if the cell given by @global_index is part of the region.
        """
        return cfunc.contains_global( self , global_index )


    def contains_active( self , active_index):
        """
        Will check if the cell given by @active_index is part of the region.
        """
        return cfunc.contains_active( self , active_index )

    
    def kw_index_list(self , ecl_kw , force_active):
        c_ptr = cfunc.get_kw_index_list( self , ecl_kw , force_active)
        index_list = IntVector.ref( c_ptr , self )
        return index_list


    def set_name( self , name ):
        cfunc.set_name( self , name )

    def get_name( self ):
        return cfunc.get_name( self )
        
    name = property( get_name , set_name )


# 2. Creating a wrapper object around the libecl library.
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_region" , EclRegion )

# 3. Installing the c-functions used to manipulate.
cfunc = CWrapperNameSpace("ecl_region")

cfunc.alloc                      = cwrapper.prototype("c_void_p ecl_region_alloc( ecl_grid , bool )")
cfunc.free                       = cwrapper.prototype("void ecl_region_free( ecl_region )")     
cfunc.reset                      = cwrapper.prototype("void ecl_region_reset( ecl_region )")

cfunc.select_all                 = cwrapper.prototype("void ecl_region_select_all( ecl_region )")
cfunc.deselect_all               = cwrapper.prototype("void ecl_region_deselect_all( ecl_region )")

cfunc.select_equal               = cwrapper.prototype("void ecl_region_select_equal( ecl_region , ecl_kw , int )")
cfunc.deselect_equal             = cwrapper.prototype("void ecl_region_deselect_equal( ecl_region , ecl_kw , int)")

cfunc.select_less                = cwrapper.prototype("void ecl_region_select_smaller( ecl_region , ecl_kw , float )")
cfunc.deselect_less              = cwrapper.prototype("void ecl_region_deselect_smaller( ecl_region , ecl_kw , float )")

cfunc.select_more                = cwrapper.prototype("void ecl_region_select_larger( ecl_region , ecl_kw , float )")
cfunc.deselect_more              = cwrapper.prototype("void ecl_region_deselect_larger( ecl_region , ecl_kw , float )")

cfunc.select_in_interval         = cwrapper.prototype("void ecl_region_select_in_interval( ecl_region, ecl_kw , float , float )")
cfunc.deselect_in_interval       = cwrapper.prototype("void ecl_region_deselect_in_interval( ecl_region, ecl_kw, float , float )")

cfunc.invert_selection           = cwrapper.prototype("void ecl_region_invert_selection( ecl_region )")

cfunc.set_kw_int                 = cwrapper.prototype("void ecl_region_set_kw_int( ecl_region , ecl_kw , int, bool) ")
cfunc.set_kw_float               = cwrapper.prototype("void ecl_region_set_kw_float( ecl_region , ecl_kw , float, bool ) ")
cfunc.set_kw_double              = cwrapper.prototype("void ecl_region_set_kw_double( ecl_region , ecl_kw , double , bool) ")

cfunc.shift_kw_int                 = cwrapper.prototype("void ecl_region_shift_kw_int( ecl_region , ecl_kw , int, bool) ")
cfunc.shift_kw_float               = cwrapper.prototype("void ecl_region_shift_kw_float( ecl_region , ecl_kw , float, bool ) ")
cfunc.shift_kw_double              = cwrapper.prototype("void ecl_region_shift_kw_double( ecl_region , ecl_kw , double , bool) ")

cfunc.scale_kw_int                 = cwrapper.prototype("void ecl_region_scale_kw_int( ecl_region , ecl_kw , int, bool) ")
cfunc.scale_kw_float               = cwrapper.prototype("void ecl_region_scale_kw_float( ecl_region , ecl_kw , float, bool ) ")
cfunc.scale_kw_double              = cwrapper.prototype("void ecl_region_scale_kw_double( ecl_region , ecl_kw , double , bool) ")

cfunc.select_box                 = cwrapper.prototype("void ecl_region_select_from_ijkbox(ecl_region , int , int , int , int , int , int)")     

cfunc.imul_kw                    = cwrapper.prototype("void  ecl_region_kw_imul( ecl_region , ecl_kw , ecl_kw , bool)")
cfunc.idiv_kw                    = cwrapper.prototype("void  ecl_region_kw_idiv( ecl_region , ecl_kw , ecl_kw , bool)")
cfunc.iadd_kw                    = cwrapper.prototype("void  ecl_region_kw_iadd( ecl_region , ecl_kw , ecl_kw , bool)")
cfunc.isub_kw                    = cwrapper.prototype("void  ecl_region_kw_isub( ecl_region , ecl_kw , ecl_kw , bool)")
cfunc.copy_kw                    = cwrapper.prototype("void  ecl_region_kw_copy( ecl_region , ecl_kw , ecl_kw , bool)")

cfunc.alloc_copy                 = cwrapper.prototype("c_void_p ecl_region_alloc_copy( ecl_region )")
cfunc.intersect                  = cwrapper.prototype("void ecl_region_intersection( ecl_region , ecl_region )")
cfunc.combine                    = cwrapper.prototype("void ecl_region_union( ecl_region , ecl_region )")
cfunc.subtract                   = cwrapper.prototype("void ecl_region_subtract( ecl_region , ecl_region )")
cfunc.xor                        = cwrapper.prototype("void ecl_region_xor( ecl_region , ecl_region )")

cfunc.get_kw_index_list          = cwrapper.prototype("c_void_p ecl_region_get_kw_index_list( ecl_region , ecl_kw , bool )")
cfunc.get_active_list            = cwrapper.prototype("c_void_p ecl_region_get_active_list( ecl_region )")
cfunc.get_global_list            = cwrapper.prototype("c_void_p ecl_region_get_global_list( ecl_region )")
cfunc.get_active_global          = cwrapper.prototype("c_void_p ecl_region_get_global_active_list( ecl_region )")

cfunc.select_cmp_less            = cwrapper.prototype("void ecl_region_cmp_select_less( ecl_region , ecl_kw , ecl_kw)")
cfunc.select_cmp_more            = cwrapper.prototype("void ecl_region_cmp_select_more( ecl_region , ecl_kw , ecl_kw)")
cfunc.deselect_cmp_less          = cwrapper.prototype("void ecl_region_cmp_deselect_less( ecl_region , ecl_kw , ecl_kw)")
cfunc.deselect_cmp_more          = cwrapper.prototype("void ecl_region_cmp_deselect_more( ecl_region , ecl_kw , ecl_kw)")

cfunc.select_islice              = cwrapper.prototype("void ecl_region_select_i1i2( ecl_region , int , int )")
cfunc.deselect_islice            = cwrapper.prototype("void ecl_region_deselect_i1i2( ecl_region , int , int )")
cfunc.select_jslice              = cwrapper.prototype("void ecl_region_select_j1j2( ecl_region , int , int )")
cfunc.deselect_jslice            = cwrapper.prototype("void ecl_region_deselect_j1j2( ecl_region , int , int )")
cfunc.select_kslice              = cwrapper.prototype("void ecl_region_select_k1k2( ecl_region , int , int )")
cfunc.deselect_kslice            = cwrapper.prototype("void ecl_region_deselect_k1k2( ecl_region , int , int )")

cfunc.select_deep_cells          = cwrapper.prototype("void ecl_region_select_deep_cells( ecl_region , double )")
cfunc.deselect_deep_cells        = cwrapper.prototype("void ecl_region_select_deep_cells( ecl_region , double )")
cfunc.select_shallow_cells       = cwrapper.prototype("void ecl_region_select_shallow_cells( ecl_region , double )")
cfunc.deselect_shallow_cells     = cwrapper.prototype("void ecl_region_select_shallow_cells( ecl_region , double )")

cfunc.select_small               = cwrapper.prototype("void ecl_region_select_small_cells( ecl_region , double )")
cfunc.deselect_small             = cwrapper.prototype("void ecl_region_deselect_small_cells( ecl_region , double )")
cfunc.select_large               = cwrapper.prototype("void ecl_region_select_large_cells( ecl_region , double )")
cfunc.deselect_large             = cwrapper.prototype("void ecl_region_deselect_large_cells( ecl_region , double )")

cfunc.select_thin                = cwrapper.prototype("void ecl_region_select_thin_cells( ecl_region , double )")
cfunc.deselect_thin              = cwrapper.prototype("void ecl_region_deselect_thin_cells( ecl_region , double )")
cfunc.select_thick               = cwrapper.prototype("void ecl_region_select_thick_cells( ecl_region , double )")
cfunc.deselect_thick             = cwrapper.prototype("void ecl_region_deselect_thick_cells( ecl_region , double )")

cfunc.select_active              = cwrapper.prototype("void ecl_region_select_active_cells( ecl_region )")
cfunc.select_inactive            = cwrapper.prototype("void ecl_region_select_inactive_cells( ecl_region )")
cfunc.deselect_active            = cwrapper.prototype("void ecl_region_deselect_active_cells( ecl_region )")
cfunc.deselect_inactive          = cwrapper.prototype("void ecl_region_deselect_inactive_cells( ecl_region )")

cfunc.select_above_plane        = cwrapper.prototype("void ecl_region_select_above_plane( ecl_region  , double* , double* )")
cfunc.select_below_plane        = cwrapper.prototype("void ecl_region_select_below_plane( ecl_region  , double* , double* )")
cfunc.deselect_above_plane      = cwrapper.prototype("void ecl_region_deselect_above_plane( ecl_region, double* , double* )")
cfunc.deselect_below_plane      = cwrapper.prototype("void ecl_region_deselect_below_plane( ecl_region, double* , double* )")

cfunc.select_inside_polygon     = cwrapper.prototype("void ecl_region_select_inside_polygon( ecl_region , double* , double* , int )")
cfunc.select_outside_polygon    = cwrapper.prototype("void ecl_region_select_outside_polygon( ecl_region , double* , double* , int )")
cfunc.deselect_inside_polygon   = cwrapper.prototype("void ecl_region_deselect_inside_polygon( ecl_region , double* , double* , int )")
cfunc.deselect_outside_polygon  = cwrapper.prototype("void ecl_region_deselect_outside_polygon( ecl_region , double* , double* , int )")

cfunc.set_name                  = cwrapper.prototype("void  ecl_region_set_name( ecl_region , char*)")
cfunc.get_name                  = cwrapper.prototype("char* ecl_region_get_name( ecl_region )")

cfunc.contains_ijk              = cwrapper.prototype("void ecl_region_contains_ijk( ecl_region , int , int , int)")
cfunc.contains_global           = cwrapper.prototype("void ecl_region_contains_global( ecl_region, int )")
cfunc.contains_active           = cwrapper.prototype("void ecl_region_contains_active( ecl_region , int )")

