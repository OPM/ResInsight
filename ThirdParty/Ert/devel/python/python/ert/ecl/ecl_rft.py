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

import ctypes
import types
import warnings

import libecl
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass
from   ert.util.ctime        import ctime
from   ecl_rft_cell          import EclRFTCell, EclPLTCell

        

class EclRFTFile(CClass):
    """
    The EclRFTFile class is used to load an ECLIPSE RFT file.

    The EclRFTFile serves as a container which can load and hold the
    content of an ECLIPSE RFT file. The RFT files will in general
    contain data for several wells and several times in one large
    container. The EclRFTClass class contains methods get the the RFT
    results for a specific time and/or well. 

    The EclRFTFile class can in general contain a mix of RFT and PLT
    measurements. The class does not really differentiate between
    these.
    """
    
    def __new__( cls , case ):
        c_ptr = cfunc_file.load( case )
        if c_ptr:
            obj = object.__new__( cls )
            obj.init_cobj( c_ptr , cfunc_file.free )
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

    
    def size( self , well = None , date = None):
        """
        The number of elements in EclRFTFile container. 

        By default the size() method will return the total number of
        RFTs/PLTs in the container, but by specifying the optional
        arguments date and/or well the function will only count the
        number of well measurements matching that time or well
        name. The well argument can contain wildcards.

           rftFile = ecl.EclRFTFile( "ECLIPSE.RFT" )
           print "Total number of RFTs : %d" % rftFile.size( )
           print "RFTs matching OP*    : %d" % rftFile.size( well = "OP*" )
           print "RFTs at 01/01/2010   : %d" % rftFile.size( date = datetime.date( 2010 , 1 , 1 ))

        """
        if date:
            cdate = ctime( date )
        else:
            cdate = -1

        return cfunc_file.get_size( self , well , cdate)


    @property
    def num_wells( self ):
        """
        Returns the total number of distinct wells in the RFT file.
        """
        return cfunc_file.get_num_wells( self )

    @property
    def headers(self):
        """
        Returns a list of two tuples (well_name , date) for the whole file.
        """
        header_list = []
        for i in (range(cfunc_file.get_size( self , None , -1))):
            rft = self.iget( i )
            header_list.append( (rft.well , rft.date) )
        return header_list


    def iget(self , index):
        """
        Will lookup RFT @index - equivalent to [@index].
        """
        return self.__getitem__( index )


    def get(self , well_name , date ):
        """
        Will look up the RFT object corresponding to @well and @date.

        Returns None if no matching RFT can be found.
        """
        c_ptr = cfunc_file.get_rft( self , well_name , ctime( date )) 
        if c_ptr:
            return EclRFT( c_ptr , self)
        else:
            return None


    


class EclRFT(CClass):
    """The EclRFT class contains the information for *one* RFT.

    The ECLIPSE RFT file can contain three different types of RFT like
    objects which are lumped together; the EclRFTClass is a container
    for such objects. The three different object types which can be
    found in an RFT file are:
   
       RFT: This is old-fashioned RFT which contains measurements of
            saturations for each of the completed cells.
       
       PLT: This contains production and flow rates for each phase in
            each cell.

       SEGMENT: Not implemented.

    In addition to the measurements specific for RFT and PLT each cell
    has coordinates, pressure and depth.
    """
    def __init__(self , c_ptr , parent):
        self.init_cref( c_ptr , parent )

    def __len__(self):
        """
        The number of completed cells in this RFT.
        """
        return cfunc_rft.get_size( self )

    def is_RFT(self):
        """
        Is instance an RFT; in that case all the cells will be EclRFTCell instances.
        """
        return cfunc_rft.is_RFT( self )

    def is_PLT(self):
        """
        Is instance a PLT; in that case all the cells will be EclPLTCell instances.
        """
        return cfunc_rft.is_PLT( self )

    def is_SEGMENT(self):
        """
        Is this a SEGMENT - not implemented.
        """
        return cfunc_rft.is_SEGMENT( self )

    def is_MSW(self):
        """
        Is this well a MSW well. Observe that the test ONLY applies to PLTs.
        """
        return cfunc_rft.is_MSW( self )


    @property
    def type(self):
        # Enum: ecl_rft_enum from ecl_rft_node.h
        # RFT     = 1
        # PLT     = 2
        # Segment = 3  -- Not properly implemented
        """
        Deprecated - use query methods: is_RFT(), is_PLT() and is_SEGMENT() instead.
        """
        warnings.warn("The property type is deprecated, use the query methods is_RFT(), is_PLT() and is_SEGMENT() instead.")
        return cfunc_rft.get_type( self )

    @property
    def well(self):
        """
        The name of the well we are considering.
        """
        return cfunc_rft.get_well( self )

    @property
    def date(self):
        """
        The date when this RFT/PLT/... was recorded.
        """
        return cfunc_rft.get_date( self )

    @property
    def size(self):
        """
        The number of completed cells.
        """
        return self.__len__()


    def __cell_ref( self , cell_ptr ):
        if self.is_RFT():
            return EclRFTCell.ref( cell_ptr , self )
        elif self.is_PLT():
            return EclPLTCell.ref( cell_ptr , self )
        else:
            raise NotImplementedError("Only RFT and PLT cells are implemented")


    def assert_cell_index( self , index ):
        if isinstance( index , types.IntType):
            length = self.__len__()
            if index < 0 or index >= length:
                raise IndexError
        else:
            raise TypeError("Index should be integer type")


    def __getitem__(self , index):
        """Implements the [] operator to return the cells.

        To get the object related to cell nr 5:

           cell = rft[4]

        The return value from the __getitem__() method is either an
        EclRFTCell instance or a EclPLTCell instance, depending on the
        type of this particular RFT object. 
        """
        self.assert_cell_index( index )
        cell_ptr = cfunc_rft.iget_cell( self , index )
        return self.__cell_ref( cell_ptr )
        

    def iget( self , index ):
        return self.__getitem__( index )

        
    def iget_sorted( self , index ):
        """
        Will return the cell nr @index in the list of sorted cells. 

        See method sort() for further documentation.
        """
        self.assert_cell_index( index )
        cell_ptr = cfunc_rft.iget_cell_sorted( self , index )
        return self.__cell_ref( cell_ptr )
        

    def sort(self):
        """
        Will sort cells in RFT; currently only applies to MSW wells.

        By default the cells in the RFT come in the order they are
        specified in the ECLIPSE input file; that is not necessarily
        in a suitable order. In the case of MSW wells it is possible
        to sort the connections after distance along the wellpath. To
        access the cells in sort order you have two options:

           1. Sort the cells using the sort() method, and then
              subsequently access them sequentially:

                rft.sort()
                for cell in rft:
                    print cell

           2. Let the rft object stay unsorted, but access the cells
              using the iget_sorted() method:

                 for i in range(len(rft)):
                     cell = rft.iget_sorted( i )
        
        Currently only MSW/PLTs are sorted, based on the CONLENST
        keyword; for other wells the sort() method does nothing.  
        """
        cfunc_rft.sort_cells( self )


    # ijk are zero offset
    def ijkget( self , ijk ):
        """
        Will look up the cell based on (i,j,k).

        If the cell (i,j,k) is not part of this RFT/PLT None will be
        returned. The (i,j,k) input values should be zero offset,
        i.e. you must subtract 1 from the (i,j,k) values given in the ECLIPSE input.
        """
        cell_ptr = cfunc_rft.lookup_ijk( self , ijk[0] , ijk[1] , ijk[2])
        if cell_ptr:
            return self.__cell_ref( cell_ptr )
        else:
            return None





# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : ecl_kw <-> EclKW
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_rft_file" , EclRFTFile )
cwrapper.registerType( "ecl_rft"      , EclRFT )

cfunc_file = CWrapperNameSpace("ecl_rft_file")
cfunc_rft  = CWrapperNameSpace("ecl_rft")

cfunc_file.load                     = cwrapper.prototype("c_void_p ecl_rft_file_alloc_case( char* )")
cfunc_file.has_rft                  = cwrapper.prototype("bool ecl_rft_file_case_has_rft( char* )")
cfunc_file.free                     = cwrapper.prototype("void ecl_rft_file_free( ecl_rft_file )")
cfunc_file.get_size                 = cwrapper.prototype("int ecl_rft_file_get_size__( ecl_rft_file , char* , time_t)")
cfunc_file.iget                     = cwrapper.prototype("c_void_p ecl_rft_file_iget_node( ecl_rft_file , int )")
cfunc_file.get_num_wells            = cwrapper.prototype("int  ecl_rft_file_get_num_wells( ecl_rft_file )")
cfunc_file.get_rft                  = cwrapper.prototype("c_void_p ecl_rft_file_get_well_time_rft( ecl_rft_file , char* , time_t)")

cfunc_rft.get_type                  = cwrapper.prototype("int    ecl_rft_node_get_type( ecl_rft )")
cfunc_rft.get_well                  = cwrapper.prototype("char*  ecl_rft_node_get_well_name( ecl_rft )")
cfunc_rft.get_date                  = cwrapper.prototype("time_t ecl_rft_node_get_date( ecl_rft )")
cfunc_rft.get_size                  = cwrapper.prototype("int ecl_rft_node_get_size( ecl_rft )")
cfunc_rft.iget_cell                 = cwrapper.prototype("c_void_p ecl_rft_node_iget_cell( ecl_rft )")
cfunc_rft.iget_cell_sorted          = cwrapper.prototype("c_void_p ecl_rft_node_iget_cell_sorted( ecl_rft )")
cfunc_rft.sort_cells                = cwrapper.prototype("c_void_p ecl_rft_node_inplace_sort_cells( ecl_rft )")
cfunc_rft.iget_depth                = cwrapper.prototype("double ecl_rft_node_iget_depth( ecl_rft )")
cfunc_rft.iget_pressure             = cwrapper.prototype("double ecl_rft_node_iget_pressure(ecl_rft)")
cfunc_rft.iget_ijk                  = cwrapper.prototype("void ecl_rft_node_iget_ijk( ecl_rft , int , int*, int*, int*)") 
cfunc_rft.iget_swat                 = cwrapper.prototype("double ecl_rft_node_iget_swat(ecl_rft)")
cfunc_rft.iget_sgas                 = cwrapper.prototype("double ecl_rft_node_iget_sgas(ecl_rft)")
cfunc_rft.iget_orat                 = cwrapper.prototype("double ecl_rft_node_iget_orat(ecl_rft)")
cfunc_rft.iget_wrat                 = cwrapper.prototype("double ecl_rft_node_iget_wrat(ecl_rft)")
cfunc_rft.iget_grat                 = cwrapper.prototype("double ecl_rft_node_iget_grat(ecl_rft)")
cfunc_rft.lookup_ijk                = cwrapper.prototype("c_void_p ecl_rft_node_lookup_ijk( ecl_rft , int , int , int)")
cfunc_rft.is_RFT                    = cwrapper.prototype("bool   ecl_rft_node_is_RFT( ecl_rft )")
cfunc_rft.is_PLT                    = cwrapper.prototype("bool   ecl_rft_node_is_PLT( ecl_rft )")
cfunc_rft.is_SEGMENT                = cwrapper.prototype("bool   ecl_rft_node_is_SEGMENT( ecl_rft )")
cfunc_rft.is_MSW                    = cwrapper.prototype("bool   ecl_rft_node_is_MSW( ecl_rft )")
