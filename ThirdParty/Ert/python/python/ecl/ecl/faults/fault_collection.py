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
from ecl.ecl import EclGrid

comment_regexp = re.compile("--.*")

def dequote(s):
    if s[0] in ["'", '"']:
        if s[0] == s[-1]:
            return s[1:-1]
        else:
            raise ValueError("Quote fuckup")
    else:
        return s


class FaultCollection(object):
    def __init__(self, grid=None, *file_list):
        self.__fault_list = []
        self.__fault_map = {}
        self.__grid = grid

        if self.__grid is not None:
            if not isinstance(self.__grid, EclGrid):
                raise ValueError("When supplying a list of files to load - you must have a grid")
            for file in file_list:
                self.load(self.__grid, file)


    def __contains__(self, fault_name):
        return self.__fault_map.has_key(fault_name)


    def __len__(self):
        return len(self.__fault_list)


    def __getitem__(self, index):
        if isinstance(index, str):
            return self.__fault_map[index]
        elif isinstance(index, int):
            return self.__fault_list[index]
        else:
            raise TypeError("Argument must be fault name or number")

    def __iter__(self):
        return iter(self.__fault_list)


    def getGrid(self):
        return self.__grid


    def getFault(self, name):
        return self[name]


    def hasFault(self, fault_name):
        return fault_name in self


    def addFault(self, fault):
        self.__fault_map[fault.getName()] = fault
        self.__fault_list.append(fault)


    def splitLine(self, line):
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

        return (fault_name, I1,I2,J1,J2,K1,K2, face)



    def loadFaults(self, grid, fileH):
        for line in fileH:
            line = comment_regexp.sub("", line)
            line = line.strip()
            if line == "/":
                break

            if line:
                (name, I1, I2, J1, J2, K1, K2, face) = self.splitLine(line)
                if not self.hasFault(name):
                    fault = Fault(grid, name)
                    self.addFault(fault)
                else:
                    fault = self.getFault(name)

                fault.addRecord(I1, I2, J1, J2, K1, K2, face)


    def load(self, grid, file_name):
        with open(file_name) as fileH:
            for line in fileH:
                if line.startswith("FAULTS"):
                    self.loadFaults(grid, fileH)
