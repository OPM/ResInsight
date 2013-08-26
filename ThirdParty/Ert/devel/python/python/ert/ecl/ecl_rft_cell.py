#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_rft_cell.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import warnings
import ctypes
import types

import libecl
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass

class RFTCell(CClass):
    """The RFTCell is a base class for the cells which are part of an RFT/PLT.
    
    The RFTCell class contains the elements which are common to both
    RFT and PLT. The list of common elements include the corrdinates
    (i,j,k) the pressure and the depth of the cell. Actual user access
    should be based on the derived classes EclRFTCell and EclPLTCell.

    Observe that from june 2013 the properties i,j and k which return
    offset 1 coordinate values are deprecated, and you should rather
    use the methods get_i(), get_j() and get_k() which return offset 0
    coordinate values.
    """


    def warn(self , old , new):
        msg = """ 

The cell property:%s has been deprecated, and the method:%s() should
be used instead. Observe that the new method %s() returns coordinate
values starting at 0, whereas the old property %s returned values
starting at 1; hence you must adapt the calling code when you change
from %s -> %s() 
""" % (old , new , new , old , old , new)
        warnings.warn( msg , DeprecationWarning )

    @property
    def i(self):
        self.warn("i" , "get_i")
        return self.get_i() + 1

    @property
    def j(self):
        self.warn("j" , "get_j")
        return self.get_j() + 1

    @property
    def k(self):
        self.warn("k" , "get_k")
        return self.get_k() + 1

    @property
    def ijk(self):
        self.warn("ijk" , "get_ijk")
        return (self.get_i() + 1 , self.get_j() + 1 , self.get_k() + 1)
    
    def get_i(self):
        return cfunc.get_i( self )

    def get_j(self):
        return cfunc.get_j( self )

    def get_k(self):
        return cfunc.get_k( self )

    def get_ijk(self):
        return (cfunc.get_i( self ) , cfunc.get_j( self ) , cfunc.get_k( self ))

    @property
    def pressure(self):
        return cfunc.get_pressure( self )

    @property
    def depth(self):
        return cfunc.get_depth( self )


#################################################################


class EclRFTCell(RFTCell):
    
    @classmethod
    def new(cls , i , j , k , depth , pressure , swat , sgas ):
        cell = EclRFTCell()
        c_ptr = cfunc.alloc_RFT( i,j,k,depth,pressure,swat,sgas)
        cell.init_cobj( c_ptr , cfunc.free )
        return cell

    @classmethod
    def ref(cls, c_ptr , parent):
        cell = EclRFTCell()
        cell.init_cref( c_ptr , parent )
        return cell

    @property
    def swat(self):
        return cfunc.get_swat( self )

    @property
    def sgas(self):
        return cfunc.get_sgas( self )

    @property
    def soil(self):
        return 1 - (cfunc.get_sgas( self ) + cfunc.get_swat( self ))
    
    
#################################################################


class EclPLTCell(RFTCell):

    @classmethod
    def new(self , i , j , k , depth , pressure , orat , grat , wrat , conn_start , flowrate , oil_flowrate , gas_flowrate , water_flowrate ):
        cell = EclPLTCell()
        c_ptr = cfunc.alloc_PLT( i,j,k,depth,pressure,orat , grat , wrat , conn_start , flowrate , oil_flowrate , gas_flowrate , water_flowrate)
        cell.init_cobj( c_ptr , cfunc.free )
        return cell

    @classmethod
    def ref(cls, c_ptr , parent):
        cell = EclPLTCell()
        cell.init_cref( c_ptr , parent )
        return cell

    @property
    def orat(self):
        return cfunc.get_orat( self )

    @property
    def grat(self):
        return cfunc.get_grat( self )

    @property
    def wrat(self):
        return cfunc.get_wrat( self )

    @property
    def conn_start(self):
        """Will return the length from wellhead(?) to connection.

        For MSW wells this property will return the distance from a
        fixed point (wellhead) to the current connection. This value
        will be used to sort the completed cells along the well
        path. In the case of non MSW wells this will just return a
        fixed default value.
        """
        return cfunc.get_conn_start( self )

    @property
    def flowrate(self):
        return cfunc.get_flowrate( self )

    @property
    def oil_flowrate(self):
        return cfunc.get_oil_flowrate( self )

    @property
    def gas_flowrate(self):
        return cfunc.get_gas_flowrate( self )

    @property
    def water_flowrate(self):
        return cfunc.get_water_flowrate( self )


#################################################################


cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "rft_cell"     , RFTCell)
cwrapper.registerType( "ecl_rft_cell" , EclRFTCell )
cwrapper.registerType( "ecl_plt_cell" , EclPLTCell )

cfunc = CWrapperNameSpace("ecl_rft_cell")

cfunc.alloc_RFT    = cwrapper.prototype("c_void_p ecl_rft_cell_alloc_RFT( int, int , int , double , double , double , double)")
cfunc.alloc_PLT    = cwrapper.prototype("c_void_p ecl_rft_cell_alloc_PLT( int, int , int , double , double , double , double, double , double , double , double , double , double )")
cfunc.free         = cwrapper.prototype("void ecl_rft_cell_free( rft_cell )")

cfunc.get_pressure = cwrapper.prototype("double ecl_rft_cell_get_pressure( rft_cell )")
cfunc.get_depth    = cwrapper.prototype("double ecl_rft_cell_get_depth( rft_cell )")
cfunc.get_i        = cwrapper.prototype("int ecl_rft_cell_get_i( rft_cell )")
cfunc.get_j        = cwrapper.prototype("int ecl_rft_cell_get_j( rft_cell )")
cfunc.get_k        = cwrapper.prototype("int ecl_rft_cell_get_k( rft_cell )")

cfunc.get_swat = cwrapper.prototype("double ecl_rft_cell_get_swat( ecl_rft_cell )")
cfunc.get_soil = cwrapper.prototype("double ecl_rft_cell_get_soil( ecl_rft_cell )")
cfunc.get_sgas = cwrapper.prototype("double ecl_rft_cell_get_sgas( ecl_rft_cell )")

cfunc.get_orat = cwrapper.prototype("double ecl_rft_cell_get_orat( ecl_plt_cell )")
cfunc.get_grat = cwrapper.prototype("double ecl_rft_cell_get_grat( ecl_plt_cell )")
cfunc.get_wrat = cwrapper.prototype("double ecl_rft_cell_get_wrat( ecl_plt_cell )")

cfunc.get_conn_start = cwrapper.prototype("double ecl_rft_cell_get_connection_start( ecl_plt_cell )")

cfunc.get_flowrate       = cwrapper.prototype("double ecl_rft_cell_get_flowrate( ecl_plt_cell )")
cfunc.get_oil_flowrate   = cwrapper.prototype("double ecl_rft_cell_get_oil_flowrate( ecl_plt_cell )")
cfunc.get_gas_flowrate   = cwrapper.prototype("double ecl_rft_cell_get_gas_flowrate( ecl_plt_cell )")
cfunc.get_water_flowrate = cwrapper.prototype("double ecl_rft_cell_get_water_flowrate( ecl_plt_cell )")


