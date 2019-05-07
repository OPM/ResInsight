#  Copyright (C) 2015  Equinor ASA, Norway.
#
#  The file 'ecl_3dkw.py' is part of ERT - Ensemble based Reservoir Tool.
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

from __future__ import (absolute_import, division,
                        print_function, unicode_literals)

from ecl.util.util import monkey_the_camel
from .ecl_kw import EclKW

class Ecl3DKW(EclKW):
    """
    Class for working with Eclipse keywords defined over a grid

    The Ecl3DKW class is derived from the EclKW class, and most of the
    methods are implemented in the EclKW base class. The purpose of
    the Ecl3DKW class is to simplify working with 3D properties like
    PORO or SATNUM.

    The Ecl3DKW class has an attached EclGrid which is used to support
    [i,j,k] indexing, and a defined default value which is used when
    reading an inactive value. The Ecl3DKW keyword instances are
    returned from the EclInitFile and EclRestartFile classes, in
    addition you can excplicitly \"cast\" a EclKW keyword to Ecl3DKW
    with the Ecl3DKW.castFromKW() classmethod.

    Usage example:

       from ecl.ecl import EclInitFile,EclGrid

       grid = EclGrid("ECLIPSE.EGRID")
       file = EclInitFile(grid , "ECLIPSE.INIT")

       permx_kw = file["PORO"][0]
       porv_kw = file["PORV"][0]

       permx_kw.setDefault( -1 )
       for k in range(grid.getNZ()):
           for j in range(grid.getNY()):
               for i in range(grid.getNX()):
                   print('"(%d,%d,%d)  Permx:%g  Porv:%g"' % (i,j,k,permx_kw[i,j,k] , porv_kw[i,j,k]))

    In the example we open an ECLIPSE INIT file and extract the PERMX
    and PORV properties, and then iterate over all the cells in the
    grid.

    In the INIT file the PORV keyword is stored with all cells,
    whereas the PERMX keyword typically only has the active cells
    stored, this active/inactive gymnastics is handled
    transparently. With the call:

        permx_kw.setDefault( -1 )

    we say that we want the value -1 for all inactive cells in the
    PERMX property.

    """
    def __init__(self, kw , grid , value_type , default_value = 0 , global_active = False):
        if global_active:
            size = grid.getGlobalSize()
        else:
            size = grid.getNumActive( )
        super(Ecl3DKW , self).__init__( kw , size , value_type)
        self.grid = grid
        self.global_active = global_active
        self.setDefault( default_value )


    @classmethod
    def create(cls , kw , grid , value_type , default_value = 0 , global_active = False):
        new_kw = Ecl3DKW(kw , grid , value_type , default_value , global_active)
        return new_kw

    @classmethod
    def read_grdecl( cls , grid , fileH , kw , strict = True , ecl_type = None):
        """
        Will load an Ecl3DKW instance from a grdecl formatted filehandle.

        See the base class EclKW.read_grdecl() for more documentation.
        """
        kw = super(Ecl3DKW , cls).read_grdecl( fileH , kw , strict , ecl_type)
        Ecl3DKW.castFromKW(kw , grid)
        return kw



    def __getitem__(self , index):
        """Will return item [g] or [i,j,k].

        The __getitem__() methods supports both scalar indexing like
        [g] and tuples [i,j,k]. If the input argument is given as a
        [i,j,k] tuple it is converted to an active index before the
        final lookup.

        If the [i,j,k] input corresponds to an inactive cell in a
        keyword with only nactive elements the default value will be
        returned. By default the default value will be 0, but another
        value can be assigned with the setDefault() method.
        """
        if isinstance(index , tuple):
            global_index = self.grid.get_global_index( ijk = index )
            if self.global_active:
                index = global_index
            else:
                if not self.grid.active( global_index = global_index):
                    return self.getDefault()
                else:
                    index = self.grid.get_active_index( ijk = index )


        return super(Ecl3DKW , self).__getitem__( index )




    def __setitem__(self , index , value):
        """Set the value of at index [g] or [i,j,k].

        The __setitem__() methods supports both scalar indexing like
        [g] and tuples [i,j,k]. If the input argument is given as a
        [i,j,k] tuple it is converted to an active index before the
        final assignment.

        If you try to assign an inactive cell in a keyword with only
        nactive elements a ValueError() exception will be raised.
        """
        if isinstance(index , tuple):
            global_index = self.grid.get_global_index( ijk = index )
            if self.global_active:
                index = global_index
            else:
                if not self.grid.active( global_index = global_index):
                    raise ValueError("Tried to assign value to inactive cell: (%d,%d,%d)" % index)
                else:
                    index = self.grid.get_active_index( ijk = index )


        return super(Ecl3DKW , self).__setitem__( index , value )


    @classmethod
    def cast_from_kw(cls, kw, grid, default_value=0):
        """Will convert a normal EclKW to a Ecl3DKW.

        The method will convert a normal EclKW instance to Ecl3DKw
        instance with an attached grid and a default value.

        The method will check that size of the keyword is compatible
        with the grid dimensions, i.e. the keyword must have either
        nactive or nx*ny*nz elements. If the size of the keyword is
        not compatible with the grid dimensions a ValueError exception
        is raised.

        Example:

          1. Load the poro keyword from a grdecl formatted file.
          2. Convert the keyword to a 3D keyword.


        from ecl.ecl import EclGrid,EclKW,Ecl3DKW

        grid = EclGrid("ECLIPSE.EGRID")
        poro = EclKW.read_grdecl(open("poro.grdecl") , "PORO")
        Ecl3DKW.castFromKW( poro , grid )

        print('Porosity in cell (10,11,12):%g' % poro[10,11,12])
        """
        if len(kw) == grid.getGlobalSize():
            kw.global_active = True
        elif len(kw) == grid.getNumActive():
            kw.global_active = False
        else:
            raise ValueError("Size mismatch - must have size matching global/active size of grid")


        kw.__class__ = cls
        kw.default_value = default_value
        kw.grid = grid
        if len(kw) == grid.getGlobalSize():
            kw.global_active = True
        else:
            kw.global_active = False

        kw.setDefault( default_value )
        return kw


    def compressed_copy(self):
        """Will return a EclKW copy with nactive elements.

        The returned copy will be of type EclKW; i.e. no default
        interpolation and only linear access in the [] operator. The
        main purpose of this is to facilitate iteration over the
        active index, and for writing binary files.
        """
        return self.grid.compressedKWCopy( self )


    def global_copy(self):
        """Will return a EclKW copy with nx*ny*nz elements.

        The returned copy will be of type EclKW; i.e. no default
        interpolation and only linear access in the [] operator. The
        main purpose of this is to facilitate iteration over the
        global index, and for writing binary files.
        """
        return self.grid.globalKWCopy( self , self.getDefault() )



    def dims(self):
        return (self.grid.getNX() , self.grid.getNY() , self.grid.getNZ())


    def set_default(self, default_value):
        self.default_value = default_value


    def get_default(self):
        return self.default_value


monkey_the_camel(Ecl3DKW, 'castFromKW', Ecl3DKW.cast_from_kw, classmethod)
monkey_the_camel(Ecl3DKW, 'compressedCopy', Ecl3DKW.compressed_copy)
monkey_the_camel(Ecl3DKW, 'globalCopy', Ecl3DKW.global_copy)
monkey_the_camel(Ecl3DKW, 'setDefault', Ecl3DKW.set_default)
monkey_the_camel(Ecl3DKW, 'getDefault', Ecl3DKW.get_default)
