#  Copyright (C) 2014.  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
#  A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

from __future__ import print_function

class FaultSegment(object):
    def __init__(self , C1 , C2 ):
        self.__C1 = C1
        self.__C2 = C2
        self.__next_segment = None


    def __eq__(self , other):
        if self.__C1 == other.__C1 and self.__C2 == other.__C2:
            return True
        elif self.__C1 == other.__C2 and self.__C2 == other.__C1:
            return True
        else:
            return False

    def __hash__(self):
        return hash(hash(self.__C1) + hash(self.__C2) + hash(self.__next_segment))

    def getCorners(self):
        return (self.__C1 , self.__C2)

    def joins(self , other):
        if self.__C1 == other.__C1:
            return True
        if self.__C1 == other.__C2:
            return True 
        if self.__C2 == other.__C1:
            return True
        if self.__C2 == other.__C2:
            return True
        
        return False

    def getC1(self):
        return self.__C1

    def getC2(self):
        return self.__C2

    def swap(self):
        C1 = self.__C1
        self.__C1 = self.__C2
        self.__C2 = C1


    def __str__(self):
        return "%d -> %d" % (self.__C1 , self.__C2)



class SegmentMap(object):
    def __init__(self):
        self.__segment_map = {}
        self.__count_map = {}

    def __len__(self):
        return len(self.__segment_map)

    def __str__(self):
        return self.__segment_map.__str__()

    def verify(self):
        for (C, count) in self.__count_map.iteritems():
            if count > 0:
                d = self.__segment_map[C]
                if len(d) != count:
                    print('CornerPoint:%d  count:%d  len(d):%d map:%s' % (C , count , len(d) , d))
                assert len(d) == count
            else:
                assert self.__segment_map.get(C) is None


    def addSegment(self , segment):
        (C1,C2) = segment.getCorners()
        if not self.__segment_map.has_key(C1):
            self.__segment_map[C1] = {}
            self.__count_map[C1] = 0
        if not self.__segment_map.has_key(C2):
            self.__segment_map[C2] = {}
            self.__count_map[C2] = 0

        if not self.__segment_map[C1].has_key(C2):
            self.__segment_map[C1][C2] = segment
            self.__count_map[C1] += 1

        if not self.__segment_map[C2].has_key(C1):
            self.__segment_map[C2][C1] = segment
            self.__count_map[C2] += 1



    def delSegment(self , segment):
        (C1,C2) = segment.getCorners()
        self.__count_map[C1] -= 1
        self.__count_map[C2] -= 1
        del self.__segment_map[C1][C2]
        del self.__segment_map[C2][C1]

        if len(self.__segment_map[C1]) == 0:
            del self.__segment_map[C1]

        if len(self.__segment_map[C2]) == 0:
            del self.__segment_map[C2]


    def popStart(self):
        end_segments = []
        for (C, count) in self.__count_map.iteritems():
            if count == 1:
                end_segments.append(self.__segment_map[C].values()[0])

        start_segment = end_segments[0]
        self.delSegment( start_segment )
        return start_segment

    def popNext(self , segment):
        (C1,C2) = segment.getCorners()
        if self.__count_map[C1] >= 1:
            next_segment = self.__segment_map[C1].values()[0]
        elif self.__count_map[C2] >= 1:
            next_segment = self.__segment_map[C2].values()[0]
        else:
            next_segment = None

        if next_segment:
            self.delSegment( next_segment )
        return next_segment


    def printContent(self):
        for d in self.__segment_map.values():
            for (C,S) in d.iteritems():
                print(S)
