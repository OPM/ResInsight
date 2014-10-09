#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'fault_collection.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import re
from .fault import Fault

comment_regexp = re.compile("--.*")

def dequote(s):
    if s[0] in ["'" , '"']:
        if s[0] == s[-1]:
            return s[1:-1]
        else:
            raise ValueError("Quote fuckup")
    else:
        return s


class FaultCollection(object):
    def __init__(self , grid , *file_list):
        self.__fault_list = []
        self.__fault_map = {}
        self.__grid = grid

        for file in file_list:
            self.load(file)


    def __len__(self):
        return len(self.__fault_list)


    def __getitem__(self , index):
        if isinstance(index , str):
            return self.__fault_map[index]
        elif isinstance(index , int):
            return self.__fault_list[index]
        else:
            raise TypeError("Argument must be fault name or number")

    def __iter__(self):
        return iter(self.__fault_list)
        

    def getGrid(self):
        return self.__grid

            
    def getFault(self , name):
        return self.__getitem__(name)


    def hasFault(self , fault_name):
        return self.__fault_map.has_key( fault_name )
        

    def addFault(self, fault_name):
        new_fault = Fault(self.__grid , fault_name)
        self.__fault_map[fault_name] = new_fault
        self.__fault_list.append( new_fault )


    def splitLine(self , line):
        tmp = line.split()
        if not tmp[-1] == "/":
            raise ValueError("Line:%s does not end with /" % line)

        if len(tmp) != 9:
            raise ValueError("Line:%s not correct number of items" % line)

        fault_name = dequote(tmp[0])
        I1 = int(tmp[1]) - 1
        I2 = int(tmp[2]) - 1
        J1 = int(tmp[3]) - 1
        J2 = int(tmp[4]) - 1
        K1 = int(tmp[5]) - 1
        K2 = int(tmp[6]) - 1
        face = dequote(tmp[7])

        return (fault_name , I1,I2,J1,J2,K1,K2 , face)


        
    def loadFaults(self , fileH):
        for line in fileH:
            line = comment_regexp.sub("" , line)
            line = line.strip()
            if line == "/":
                break

            if line:
                (name , I1 , I2 , J1 , J2 , K1 , K2 , face) = self.splitLine( line )
                if not self.hasFault(name):
                    self.addFault(name)
                fault = self.getFault(name)
                fault.addRecord(I1 , I2 , J1 , J2 , K1 , K2 , face)


    def load(self , file_name):
        with open(file_name) as fileH:
            for line in fileH:
                if line.startswith("FAULTS"):
                    self.loadFaults(fileH)
            
