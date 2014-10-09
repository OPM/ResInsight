#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'fault.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from .fault_line import FaultLine
from .fault_segments import FaultSegment , SegmentMap
from  ert.ecl import EclTypeEnum

class Layer(object):
    def __init__(self, grid , K):
        self.__grid = grid
        self.__K = K
        self.__fault_lines = []
        self.__segment_map = SegmentMap()
        self.__processed = False

        
    def addSegment(self , segment):
        self.__segment_map.addSegment( segment )
        self.__processed = False

    def __len__(self):
        self.processSegments()
        return len(self.__fault_lines)

    def __iter__(self):
        self.processSegments()
        return iter(self.__fault_lines)

    def __getitem__(self , index):
        self.processSegments()
        return self.__fault_lines[index]

    def getK(self):
        return self.__K


    def getNeighborCells(self):
        neighbor_cells = []
        for fl in self:
            neighbor_cells += fl.getNeighborCells()
        return neighbor_cells


    def processSegments(self):
        if self.__processed:
            return

        while self.__segment_map:
            fault_line = FaultLine(self.__grid , self.__K)
            self.__fault_lines.append( fault_line )

            current_segment = self.__segment_map.popStart()
            while current_segment:
                append = fault_line.tryAppend(current_segment)
                if not append:
                    fault_line = FaultLine(self.__grid , self.__K)
                    self.__fault_lines.append( fault_line )
                    fault_line.tryAppend(current_segment)

                current_segment.next_segment = self.__segment_map.popNext( current_segment )
                current_segment = current_segment.next_segment
                
        self.__processed = True


#################################################################


class Fault(object):
    allowed_faces = ["X","Y","Z","I","J","K","X-","Y-","Z-","I-","J-","K-"]

    def __init__(self, grid , name):
        self.__grid = grid
        self.__name = name
        self.__layer_map  = {}
        self.__layer_list = []
        (self.nx , self.ny , self.nz , nactive) = grid.dims
        

    def __str__(self):
        return "Fault:%s" % self.__name

    def __getitem__(self , K):
        layer = self.__layer_map[K]
        return layer

    def __len__(self):
        return len(self.__layer_map)


    def __iter__(self):
        for layer in self.__layer_list:
            yield layer


    def hasLayer(self , K):
        return self.__layer_map.has_key( K )


    def addLayer(self , K):
        layer = Layer(self.__grid , K)
        self.__layer_map[K] = layer
        self.__layer_list.append( layer )


    def createSegment(self , I1 , I2 , J1 , J2 , face):
        if face in ["X" , "I"]:
            C1 = I1 + 1 + J1*(self.nx + 1)
            C2 = C1 + (1 + J2 - J1) * (self.nx + 1)
        elif face in ["X-" , "I-"]:
            C1 = I1 + J1*(self.nx + 1)
            C2 = C1 + (1 + J2 - J1) * (self.nx + 1)
        elif face in ["Y" , "J"]:
            C1 = I1 + (J1 + 1) * (self.nx + 1)
            C2 = C1 + (1 + I2 - I1)
        elif face in ["Y-" , "J-"]:
            C1 = I1 + J1 * (self.nx + 1)
            C2 = C1 + (1 + I2 - I1)
        else:
            raise Exception("Can only handle X,Y faces")

        return FaultSegment(C1,C2)
         

        
    def addRecord(self , I1 , I2 , J1 , J2 , K1 , K2 , face):
        if not face in Fault.allowed_faces:
            raise ValueError("Invalid face:%s" % face)
            
        if I1 > I2:
            raise ValueError("Invalid I1 I2 indices")

        if J1 > J2:
            raise ValueError("Invalid J1 J2 indices")

        if K1 > K2:
            raise ValueError("Invalid K1 K2 indices")
            
        if I1 < 0 or I1 >= self.nx:
            raise ValueError("Invalid I1:%d" % I1)
        if I2 < 0 or I2 >= self.nx:
            raise ValueError("Invalid I2:%d" % I2)

        if J1 < 0 or J1 >= self.ny:
            raise ValueError("Invalid J1:%d" % J1)
        if J2 < 0 or J2 >= self.ny:
            raise ValueError("Invalid J2:%d" % J2)

        if K1 < 0 or K1 >= self.nz:
            raise ValueError("Invalid K1:%d" % K1)
        if K2 < 0 or K2 >= self.nz:
            raise ValueError("Invalid K2:%d" % K2)

        if face in ["X","I"]:
            if I1 != I2:
                raise ValueError("For face:%s we must have I1 == I2" % face)

        if face in ["Y","J"]:
            if J1 != J2:
                raise ValueError("For face:%s we must have J1 == J2" % face)

        if face in ["Z","K"]:
            if K1 != K2:
                raise ValueError("For face:%s we must have K1 == K2" % face)
        
        #-----------------------------------------------------------------
        
        for K in range(K1,K2+1):
            if not self.hasLayer(K):
                self.addLayer(K)
            layer = self.__layer_map[K]
            segment = self.createSegment(I1,I2,J1,J2,face)
            layer.addSegment( segment )
            

    def getName(self):
        return self.__name


    def getNeighborCells(self):
        neighbor_cells = []
        for layer in self:
            neighbor_cells += layer.getNeighborCells()
        return neighbor_cells


