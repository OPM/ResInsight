#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_rft.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Module for loading ECLIPSE RFT files.
"""

import libecl
import ctypes
import types
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass
from   ert.util.ctime        import ctime

RFT = 1
PLT = 2
        

class EclRFTFile(CClass):
    def __new__( cls , case ):
        c_ptr = cfunc_file.load( case )
        if c_ptr:
            obj = object.__new__( cls )
            self.init_cobj( c_ptr , cfunc_file.free )
            return obj
        else:
            return None
        

    def __len__(self):
        return cfunc_file.get_size( self , None , -1)


    def __getitem__(self , index):
        if isinstance( index , types.IntType):
            length = self.__len__()
            if index < 0 or index >= length:
                raise IndexError
            else:
                return EclRFT( cfunc_file.iget( self , index ) , self )
        else:
            raise TypeError("Index should be integer type")


    
    @property
    def size( self , well = None , date = None):
        return cfunc_file.get_size( self , well , -1)

    @property
    def num_wells( self ):
        return cfunc_file.get_num_wells( self )

    @property
    def headers(self):
        header_list = []
        for i in (range(cfunc_file.get_size( self , None , -1))):
            rft = self.iget( i )
            print rft
            header_list.append( (rft.well , rft.date) )
        return header_list

    def iget(self , index):
        return self.__getitem__( index )

    def get(self , well_name , date ):
        c_ptr = cfunc_file.get_rft( self , well_name , ctime( date )) 
        if c_ptr:
            return EclRFT( c_ptr , self)
        else:
            return None


    


class EclRFT(CClass):
    def __init__(self , c_ptr , parent):
        self.init_cref( c_ptr , parent )


    def __len__(self):
        return cfunc_rft.get_size( self )



    @property
    def type(self):
        # Enum: ecl_rft_enum from ecl_rft_node.h
        # RFT     = 1
        # PLT     = 2
        # Segment = 3  -- Not properly implemented
        return cfunc_rft.get_type( self )

    @property
    def well(self):
        return cfunc_rft.get_well( self )

    @property
    def date(self):
        return cfunc_rft.get_date( self )

    @property
    def size(self):
        return self.__len__()


    def __make_node( self , index ):
        i = ctypes.c_int()
        j = ctypes.c_int()
        k = ctypes.c_int()
        pressure = cfunc_rft.iget_pressure( self, index )
        depth    = cfunc_rft.iget_depth( self, index )
        cfunc_rft.iget_ijk( self, index , ctypes.byref(i), ctypes.byref(j) , ctypes.byref(k))

        
        if self.type == RFT:
            swat = cfunc_rft.iget_swat( self, index )
            sgas = cfunc_rft.iget_sgas( self, index )
            return EclRFTCell.RFTCell( i,j,k,depth , pressure,swat,sgas)
        else:
            orat = cfunc_rft.iget_orat( self, index )
            wrat = cfunc_rft.iget_wrat( self, index )
            grat = cfunc_rft.iget_grat( self, index )
            return EclRFTCell.PLTCell( i,j,k,depth , pressure,orat,grat,wrat)

    def __getitem__(self , index):
        if isinstance( index , types.IntType):
            length = self.__len__()
            if index < 0 or index >= length:
                raise IndexError
            else:
                return self.__make_node( index )
        else:
            raise TypeError("Index should be integer type")

    def iget( self , index ):
        return self.__getitem__( index )
        

    # ijk are zero offset
    def ijkget( self , ijk ):
        index = cfunc_rft.lookup_ijk( self , ijk[0] , ijk[1] , ijk[2])
        if index >= 0:
            return self.iget( index )
        else:
            return None



class EclRFTCell:
    def __init__(self , type , i , j , k , depth , pressure):
        self.type = type
        self.__i = i
        self.__j = j
        self.__k = k
        self.__depth    = depth
        self.__pressure = pressure

        self.__sgas     = None 
        self.__swat     = None
        self.__orat     = None
        self.__wrat     = None
        self.__grat     = None

    @classmethod
    def RFTCell( cls , i,j,k,depth,pressure,swat,sgas):
        cell = cls(RFT , i,j,k,depth,pressure)
        cell.__swat = swat
        cell.__sgas = sgas
        return cell

    @classmethod
    def PLTCell(cls , i,j,k,depth,pressure,orat,grat,wrat):
        cell = cls(PLT , i,j,k,depth,pressure)
        cell.__orat = orat
        cell.__wrat = wrat
        cell.__grat = grat
        return cell
        
    @property
    def i(self):
        return self.__i.value

    @property
    def j(self):
        return self.__j.value

    @property
    def k(self):
        return self.__k.value

    @property
    def ijk(self):
        return (self.__i.value , self.__j.value , self.__k.value)

    @property
    def pressure(self):
        return self.__pressure
    
    @property
    def depth(self):
        return self.__depth

    @property
    def PLT(self):
        if self.type == PLT:
            return True
        else:
            return False

    @property
    def sgas(self):
        return self.__sgas

    @property
    def swat(self):
        return self.__swat

    @property
    def orat(self):
        return self.__orat

    @property
    def grat(self):
        return self.__grat

    @property
    def wrat(self):
        return self.__wrat




# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : ecl_kw <-> EclKW
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_rft_file" , EclRFTFile )
cwrapper.registerType( "ecl_rft"      , EclRFT )

cfunc_file = CWrapperNameSpace("ecl_rft_file")
cfunc_rft  = CWrapperNameSpace("ecl_rft")

cfunc_file.load                     = cwrapper.prototype("long ecl_rft_file_alloc_case( char* )")
cfunc_file.has_rft                  = cwrapper.prototype("bool ecl_rft_file_case_has_rft( char* )")
cfunc_file.free                     = cwrapper.prototype("void ecl_rft_file_free( ecl_rft_file )")
cfunc_file.get_size                 = cwrapper.prototype("int ecl_rft_file_get_size__( ecl_rft_file , char* , time_t)")
cfunc_file.iget                     = cwrapper.prototype("long ecl_rft_file_iget_node( ecl_rft_file , int )")
cfunc_file.get_num_wells            = cwrapper.prototype("int  ecl_rft_file_get_num_wells( ecl_rft_file )")
cfunc_file.get_rft                  = cwrapper.prototype("long ecl_rft_file_get_well_time_rft( ecl_rft_file , char* , time_t)")

cfunc_rft.get_type                      = cwrapper.prototype("int    ecl_rft_node_get_type( ecl_rft )")
cfunc_rft.get_well                      = cwrapper.prototype("char*  ecl_rft_node_get_well_name( ecl_rft )")
cfunc_rft.get_date                      = cwrapper.prototype("time_t ecl_rft_node_get_date( ecl_rft )")
cfunc_rft.get_size                      = cwrapper.prototype("int ecl_rft_node_get_size( ecl_rft )")
cfunc_rft.iget_depth                    = cwrapper.prototype("double ecl_rft_node_iget_depth( ecl_rft )")
cfunc_rft.iget_pressure                 = cwrapper.prototype("double ecl_rft_node_iget_pressure(ecl_rft)")
cfunc_rft.iget_ijk                      = cwrapper.prototype("void ecl_rft_node_iget_ijk( ecl_rft , int , int*, int*, int*)") 
cfunc_rft.iget_swat                     = cwrapper.prototype("double ecl_rft_node_iget_swat(ecl_rft)")
cfunc_rft.iget_sgas                     = cwrapper.prototype("double ecl_rft_node_iget_sgas(ecl_rft)")
cfunc_rft.iget_orat                     = cwrapper.prototype("double ecl_rft_node_iget_orat(ecl_rft)")
cfunc_rft.iget_wrat                     = cwrapper.prototype("double ecl_rft_node_iget_wrat(ecl_rft)")
cfunc_rft.iget_grat                     = cwrapper.prototype("double ecl_rft_node_iget_grat(ecl_rft)")
cfunc_rft.lookup_ijk                    = cwrapper.prototype("int    ecl_rft_node_lookup_ijk( ecl_rft , int , int , int)")

