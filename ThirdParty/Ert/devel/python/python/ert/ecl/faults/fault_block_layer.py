#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'fault_block_layer.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from ert.cwrap import BaseCClass, CWrapper
from ert.ecl import ECL_LIB
from ert.ecl import EclTypeEnum
from ert.ecl.faults import Fault

class FaultBlockLayer(BaseCClass):

    def __init__(self , grid , k):
        c_pointer = self.cNamespace().alloc( grid , k)
        if c_pointer:
            super(FaultBlockLayer, self).__init__(c_pointer)
        else:
            raise ValueError("Invalid input - failed to create FaultBlockLayer")

        # The underlying C implementation uses lazy evaluation and
        # needs to hold on to the grid reference. We therefor take
        # references to it here, to protect against premature garbage
        # collection.
        self.grid_ref = grid


    def __len__(self):
        return self.cNamespace().size(self)
        

    def __getitem__(self , index):
        """
        @rtype: FaultBlock
        """
        if isinstance(index, int):
            if index < 0:
                index += len(self)
                
            if 0 <= index < len(self):
                return self.cNamespace().iget_block( self , index ).setParent(self)
            else:
                raise IndexError("Index:%d out of range: [0,%d)" % (index , len(self)))
        elif isinstance(index,tuple):
            i,j = index
            if 0 <= i < self.grid_ref.getNX() and 0 <= j < self.grid_ref.getNY():
                geo_layer = self.getGeoLayer()
                block_id = geo_layer[i,j]
                if block_id == 0:
                    raise ValueError("No fault block defined for location (%d,%d)" % (i,j))
                else:
                    return self.getBlock( block_id )
            else:
                raise IndexError("Invalid i,j : (%d,%d)" % (i,j))
        else:
            raise TypeError("Index should be integer type")

    def __contains__(self , block_id):
        return self.cNamespace().has_block( self , block_id)


    def scanKeyword(self , fault_block_kw):
        """
        Will reorder the block ids, and ensure single connectedness. Assign block_id to zero blocks.
        """
        ok = self.cNamespace().scan_keyword( self , fault_block_kw )
        if not ok:
            raise ValueError("The fault block keyword had wrong type/size:  type:%s  size:%d  grid_size:%d" % (fault_block_kw.typeName() , len(fault_block_kw) , self.grid_ref.getGlobalSize()))


    def loadKeyword(self , fault_block_kw):
        """
        Will load directly from keyword - without reorder; ignoring zero.
        """
        ok = self.cNamespace().load_keyword( self , fault_block_kw )
        if not ok:
            raise ValueError("The fault block keyword had wrong type/size:  type:%s  size:%d  grid_size:%d" % (fault_block_kw.typeName() , len(fault_block_kw) , self.grid_ref.getGlobalSize()))
            

    def getBlock(self , block_id):
        """
        @rtype: FaultBlock
        """
        if block_id in self:
            return self.cNamespace().get_block( self , block_id).setParent(self)
        else:
            raise KeyError("No blocks with ID:%d in this layer" % block_id)


    def deleteBlock(self , block_id):
        if block_id in self:
            self.cNamespace().del_block( self , block_id)
        else:
            raise KeyError("No blocks with ID:%d in this layer" % block_id)

    def addBlock(self , block_id = None):
        if block_id is None:
            block_id = self.getNextID()

        if block_id in self:
            raise KeyError("Layer already contains block with ID:%s" % block_id)
        else:
            return self.cNamespace().add_block( self , block_id).setParent(self)
    
    def getNextID(self):
        return self.cNamespace().get_next_id( self )


    def getK(self):
        return self.cNamespace().getK( self )
            

    def free(self):
        self.cNamespace().free(self)


    def scanLayer( self , layer):
        self.cNamespace().scan_layer(self , layer)
    

    def insertBlockContent(self , block):
        self.cNamespace().insert_block_content(self , block)

    def exportKeyword(self , kw):
        if len(kw) != self.grid_ref.getGlobalSize():
            raise ValueError("The size of the target keyword must be equal to the size of the grid. Got:%d Expected:%d" % (len(kw) , self.grid_ref.getGlobalSize()))

        if kw.getEclType() != EclTypeEnum.ECL_INT_TYPE:
            raise TypeError("The target kewyord must be of integer type")
            
        self.cNamespace().export_kw( self , kw )


    def addFaultBarrier(self , fault , link_segments = False):
        layer = self.getGeoLayer( )
        layer.addFaultBarrier( fault , self.getK() , link_segments )


    def addFaultLink(self , fault1 , fault2 ):
        if not fault1.intersectsFault( fault2 , self.getK()):
            layer = self.getGeoLayer()
            layer.addIJBarrier( fault1.extendToFault( fault2 , self.getK() ) )


    def joinFaults(self , fault1 , fault2):
        if not fault1.intersectsFault( fault2 , self.getK()):
            layer = self.getGeoLayer()
            try:
                layer.addIJBarrier( Fault.joinFaults( fault1 , fault2 , self.getK()) )
            except ValueError:
                print "Failed to join faults %s and %s" % (fault1.getName() , fault2.getName())
                raise ValueError("")


    def addPolylineBarrier(self , polyline):
        layer = self.getGeoLayer()
        p0 = polyline[0]
        c0 = self.grid_ref.findCellCornerXY( p0[0] ,  p0[1] , self.getK() )
        i,j = self.grid_ref.findCellXY( p0[0] ,  p0[1] , self.getK() )
        print "%g,%g -> %d,%d   %d" % (p0[0] , p0[1] , i,j,c0)
        for index in range(1,len(polyline)):
            p1 = polyline[index]
            c1 = self.grid_ref.findCellCornerXY( p1[0] ,  p1[1] , self.getK() )
            i,j = self.grid_ref.findCellXY( p1[0] ,  p1[1] , self.getK() )
            layer.addInterpBarrier( c0 , c1 )
            print "%g,%g -> %d,%d   %d" % (p1[0] , p1[1] , i,j,c1)
            print "Adding barrier %d -> %d" % (c0 , c1)
            c0 = c1
            
        

    def getGeoLayer(self):
        """Returns the underlying geometric layer."""
        return self.cNamespace().get_layer( self )


    def cellContact(self , p1 , p2):
        layer = self.getGeoLayer()
        return layer.cellContact(p1,p2)
        


cwrapper = CWrapper(ECL_LIB)
CWrapper.registerObjectType("fault_block_layer", FaultBlockLayer)


FaultBlockLayer.cNamespace().alloc      = cwrapper.prototype("c_void_p         fault_block_layer_alloc(ecl_grid ,  int)")
FaultBlockLayer.cNamespace().free       = cwrapper.prototype("void             fault_block_layer_free(fault_block_layer)")
FaultBlockLayer.cNamespace().size       = cwrapper.prototype("int              fault_block_layer_get_size(fault_block_layer)")
FaultBlockLayer.cNamespace().iget_block = cwrapper.prototype("fault_block_ref  fault_block_layer_iget_block(fault_block_layer, int)")
FaultBlockLayer.cNamespace().add_block  = cwrapper.prototype("fault_block_ref  fault_block_layer_add_block(fault_block_layer, int)")
FaultBlockLayer.cNamespace().get_block  = cwrapper.prototype("fault_block_ref  fault_block_layer_get_block(fault_block_layer, int)")
FaultBlockLayer.cNamespace().del_block  = cwrapper.prototype("void  fault_block_layer_del_block(fault_block_layer, int)")
FaultBlockLayer.cNamespace().has_block  = cwrapper.prototype("bool  fault_block_layer_has_block(fault_block_layer, int)")
FaultBlockLayer.cNamespace().scan_keyword  = cwrapper.prototype("bool  fault_block_layer_scan_kw(fault_block_layer, ecl_kw)")
FaultBlockLayer.cNamespace().load_keyword  = cwrapper.prototype("bool  fault_block_layer_load_kw(fault_block_layer, ecl_kw)")
FaultBlockLayer.cNamespace().getK          = cwrapper.prototype("int   fault_block_layer_get_k(fault_block_layer)")
FaultBlockLayer.cNamespace().get_next_id   = cwrapper.prototype("int   fault_block_layer_get_next_id(fault_block_layer)")
FaultBlockLayer.cNamespace().scan_layer    = cwrapper.prototype("void  fault_block_layer_scan_layer( fault_block_layer , layer)")
FaultBlockLayer.cNamespace().insert_block_content = cwrapper.prototype("void  fault_block_layer_insert_block_content( fault_block_layer , fault_block)")
FaultBlockLayer.cNamespace().export_kw            = cwrapper.prototype("bool  fault_block_layer_export( fault_block_layer , ecl_kw )")
FaultBlockLayer.cNamespace().get_layer            = cwrapper.prototype("layer_ref fault_block_layer_get_layer( fault_block_layer )")
