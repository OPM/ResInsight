#  Copyright (C) 2014  Equinor ASA, Norway.
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

import numpy as np

from ecl.util.util import monkey_the_camel
from ecl.util.geometry import Polyline, CPolyline, GeometryTools

from .fault_line import FaultLine
from .fault_segments import FaultSegment, SegmentMap


class FaultLayer(object):
    def __init__(self, grid, K):
        assert(isinstance(K, int))
        self.__grid = grid
        self.__K = K
        self.__fault_lines = []
        self.__segment_map = SegmentMap()
        self.__processed = False


    def add_segment(self, segment):
        self.__segment_map.addSegment(segment)
        self.__processed = False

    def __len__(self):
        self.processSegments()
        return len(self.__fault_lines)

    def __iter__(self):
        self.processSegments()
        return iter(self.__fault_lines)

    def __getitem__(self, index):
        self.processSegments()
        return self.__fault_lines[index]

    def get_k(self):
        return self.__K

    @property
    def k(self):
        return self.__K


    def get_neighbor_cells(self):
        neighbor_cells = []
        for fl in self:
            neighbor_cells += fl.getNeighborCells()
        return neighbor_cells

    def get_polyline(self, name=None):
        polyline = CPolyline(name=name)
        for fl in self:
            polyline += fl.getPolyline()
        return polyline


    def get_ij_polyline(self):
        """
        Will return a python list of (int,int) tuple.
        """
        polyline = []
        for fl in self:
            polyline += fl.getIJPolyline()
        return polyline


    def num_lines(self):
        return len(self)

    def __sort_fault_lines(self):
        """A fault can typically consist of several non connected fault
           segments; right after reading the fault input these can be in
           a complete mess:

           1. The different part of the fault can be in random order.

           2. Within each fault line the micro segments can be ordered in
           reverse.

           This method goes through some desparate heuristics trying to sort
           things out.

        """

        N = len(self.__fault_lines)
        x = np.zeros(N)
        y = np.zeros(N)
        for index,line in enumerate(self.__fault_lines):
            xc,yc = line.center()
            x[index] = xc
            y[index] = yc





        # y = beta[1] + beta[0] * x
        #   = a       + b * x
        beta = np.polyfit(x, y, 1)
        a = beta[1]
        b = beta[0]

        perm_list = []
        for index,line in enumerate(self.__fault_lines):
            x0, y0 = line.center()
            d = x0 + b*(y0 - a)
            perm_list.append((index, d))
        perm_list.sort(key=lambda x: x[1])

        fault_lines = []
        for (index,d) in perm_list:
            fault_lines.append(self.__fault_lines[ index  ])
        self.__fault_lines = fault_lines


        for line in self.__fault_lines:
            x1,y1 = line.startPoint()
            x2,y2 = line.endPoint()
            d1 = x1 + b*(y1 - a)
            d2 = x2 + b*(y2 - a)

            if d1 > d2:
                line.reverse()




    def process_segments(self):
        if self.__processed:
            return

        while self.__segment_map:
            fault_line = FaultLine(self.__grid, self.__K)
            self.__fault_lines.append(fault_line)

            current_segment = self.__segment_map.popStart()
            while current_segment:
                append = fault_line.tryAppend(current_segment)
                if not append:
                    fault_line = FaultLine(self.__grid, self.__K)
                    self.__fault_lines.append(fault_line)
                    fault_line.tryAppend(current_segment)

                current_segment.next_segment = self.__segment_map.popNext(current_segment)
                current_segment = current_segment.next_segment

        if len(self.__fault_lines) > 1:
            self.__sort_fault_lines()

        self.__processed = True


#################################################################


class Fault(object):
    allowed_faces = ["X","Y","Z","I","J","K","X-","Y-","Z-","I-","J-","K-"]

    def __init__(self, grid, name):
        self.__grid = grid
        self.__name = name
        self.__layer_map  = {}
        self.__layer_list = []
        (self.nx, self.ny, self.nz, nactive) = grid.getDims()


    def __str__(self):
        return "Fault:%s" % self.__name

    def __getitem__(self, K):
        if not self.hasLayer(K):
            self.addLayer(K)
        layer = self.__layer_map[K]
        return layer

    def __len__(self):
        return len(self.__layer_map)


    def __iter__(self):
        for layer in self.__layer_list:
            yield layer


    def has_layer(self, K):
        return K in self.__layer_map


    def add_layer(self, K):
        layer = FaultLayer(self.__grid, K)
        self.__layer_map[K] = layer
        self.__layer_list.append(layer)


    def create_segment(self, I1, I2, J1, J2, face):
        if face in ["X", "I"]:
            C1 = I1 + 1 + J1*(self.nx + 1)
            C2 = C1 + (1 + J2 - J1) * (self.nx + 1)
        elif face in ["X-", "I-"]:
            C1 = I1 + J1*(self.nx + 1)
            C2 = C1 + (1 + J2 - J1) * (self.nx + 1)
        elif face in ["Y", "J"]:
            C1 = I1 + (J1 + 1) * (self.nx + 1)
            C2 = C1 + (1 + I2 - I1)
        elif face in ["Y-", "J-"]:
            C1 = I1 + J1 * (self.nx + 1)
            C2 = C1 + (1 + I2 - I1)
        else:
            return None

        return FaultSegment(C1,C2)



    def add_record(self, I1, I2, J1, J2, K1, K2, face):
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
            if segment:
                layer.addSegment(segment)



    @property
    def name(self):
        return self.__name

    def get_name(self):
        return self.__name


    def get_neighbor_cells(self):
        neighbor_cells = []
        for layer in self:
            neighbor_cells += layer.getNeighborCells()
        return neighbor_cells


    def get_polyline(self, k):
        layer = self[k]
        return layer.getPolyline(name="Polyline[%s]" % self.getName())


    def get_ij_polyline(self, k):
        layer = self[k]
        return layer.getIJPolyline()


    def num_lines(self, k):
        layer = self[k]
        return layer.numLines()


    @staticmethod
    def __ray_intersect(p0, p1, polyline):
        ray_dir = GeometryTools.lineToRay(p0, p1)
        intersections = GeometryTools.rayPolygonIntersections(p1, ray_dir, polyline)
        if intersections:
            if len(intersections) > 1:
                d_list = [ GeometryTools.distance(p1, p[1]) for p in intersections ]
                index = d_list.index(min(d_list))
            else:
                index = 0
            p2 = intersections[index][1]
            return [p1, p2]
        else:
            return None


    def connect_with_polyline(self, polyline, k):
        """
        """
        if self.intersectsPolyline(polyline, k):
            return None
        else:
            self_polyline = self.getPolyline(k)
            if len(self_polyline) > 0:
                return self_polyline.connect(polyline)
            else:
                return None


    def connect(self, target, k):
        if isinstance(target, Fault):
            polyline = target.getPolyline(k)
        else:
            polyline = target
        return self.connectWithPolyline(polyline, k)



    def extend_to_polyline(self, polyline, k):
        """Extends the fault until it intersects @polyline in layer @k.

        The return value is a list [(x1,y1), (x2,y2)] where (x1,y1)
        is on the tip of the fault, and (x2,y2) is on the polyline. If
        the fault already intersects polyline None is returned, if no
        intersection is found a ValueError exception is raised.

        The method will try four different strategies for finding an
        intersection between the extension of the fault and the
        polyline. Assume the fault and the polyline looks like:


        Polyline: ----------------------------------------------

                        +------------+       D
                        |            |       |
                        |            +-------C
                  B-----+
                  |
                  A

        The algorithm will then try to intersect the following rays
        with the polyline, the first match will return:

           1. (Pc, Pd)
           2. (Pb, Pa)
           3. (Pa, Pd)
           4. (Pd, Pa)

        The fault object is not directed in any way; i.e. in the case
        both (Pc,Pd) and (Pb,Pa) intersects the polyline it is
        impossible to know which intersection is returned, without
        actually consulting the construction of the fault object.
        """
        if self.intersectsPolyline(polyline, k):
            return None

        fault_polyline = self.getPolyline(k)
        p0 = fault_polyline[-2]
        p1 = fault_polyline[-1]
        extension = self.__ray_intersect(p0, p1, polyline)
        if extension:
            return extension

        p0 = fault_polyline[1]
        p1 = fault_polyline[0]
        extension = self.__ray_intersect(p0, p1, polyline)
        if extension:
            return extension

        p0 = fault_polyline[0]
        p1 = fault_polyline[-1]
        extension = self.__ray_intersect(p0, p1, polyline)
        if extension:
            return extension

        p0 = fault_polyline[-1]
        p1 = fault_polyline[0]
        extension = self.__ray_intersect(p0, p1, polyline)
        if extension:
            return extension

        raise ValueError("The fault %s can not be extended to intersect with polyline:%s in layer:%d" % (self.getName(), polyline.getName(), k+1))



    def intersects_polyline(self, polyline, k):
        fault_line = self.getPolyline(k)
        return fault_line.intersects(polyline)


    def intersects_fault(self, other_fault, k):
        fault_line = other_fault.getPolyline(k)
        return self.intersectsPolyline(fault_line, k)

    def extend_to_fault(self, fault, k):
        fault_line = fault.getPolyline(k)
        return self.extendToPolyline(fault_line, k)

    def extend_to_edge(self, edge, k):
        if isinstance(edge, Fault):
            return self.extendToFault(edge, k)
        else:
            return self.extendToPolyline(edge, k)


    def extend_to_b_box(self, bbox, k, start=True):
        fault_polyline = self.getPolyline(k)
        if start:
            p0 = fault_polyline[1]
            p1 = fault_polyline[0]
        else:
            p0 = fault_polyline[-2]
            p1 = fault_polyline[-1]

        ray_dir = GeometryTools.lineToRay(p0,p1)
        intersections = GeometryTools.rayPolygonIntersections(p1, ray_dir, bbox)
        if intersections:
            p2 = intersections[0][1]
            if self.getName():
                name = "Extend:%s" % self.getName()
            else:
                name = None

            return CPolyline(name=name, init_points=[(p1[0], p1[1]), p2])
        else:
            raise Exception("Logical error - must intersect with bounding box")


    def end_join(self, other, k):
        fault_polyline = self.getPolyline(k)

        if isinstance(other, Fault):
            other_polyline = other.getPolyline(k)
        else:
            other_polyline = other

        return GeometryTools.joinPolylines(fault_polyline, other_polyline)



    def connect_polyline_onto(self, polyline, k):
        if self.intersectsPolyline(polyline, k):
            return None

        self_polyline = self.getPolyline(k)
        return polyline.connect(self_polyline)



    def extend_polyline_onto(self, polyline, k):
        if self.intersectsPolyline(polyline, k):
            return None

        if len(polyline) > 1:
            fault_polyline = self.getPolyline(k)
            ext1 = self.__ray_intersect(polyline[-2], polyline[-1], fault_polyline)
            ext2 = self.__ray_intersect(polyline[0] , polyline[1] , fault_polyline)

            if ext1 and ext2:
                d1 = GeometryTools.distance(ext1[0], ext1[1])
                d2 = GeometryTools.distance(ext2[0], ext2[1])

                if d1 < d2:
                    return ext1
                else:
                    return ext2

            if ext1:
                return ext1
            else:
                return ext2
        else:
            raise ValueError("Polyline must have length >= 2")



    @staticmethod
    def intersect_fault_rays(ray1, ray2):
        p1,dir1 = ray1
        p2,dir2 = ray2
        if p1 == p2:
            return []

        dx = p2[0] - p1[0]
        dy = p2[1] - p1[1]
        if dx != 0:
            if dir1[0] * dx <= 0 and dir2[0] * dx >= 0:
                raise ValueError("Rays will never intersect")

        if dy != 0:
            if dir1[1] * dy <= 0 and dir2[1] * dy >= 0:
                raise ValueError("Rays will never intersect")

        if dx*dy != 0:
            if dir1[0] != 0:
                xc = p2[0]
                yc = p1[1]
            else:
                xc = p1[0]
                yc = p2[1]

            coord_list = [p1, (xc,yc), p2]
        else:
            coord_list = [p1,p2]

        return coord_list


    @staticmethod
    def int_ray(p1,p2):
        if p1 == p2:
            raise Exception("Can not form ray from coincident points")

        if p1[0] == p2[0]:
            # Vertical line
            dx = 0
            if p2[1] > p1[1]:
                dy = 1
            elif p2[1] < p1[1]:
                dy = -1
        else:
            # Horizontal line
            if p2[1] != p1[1]:
                raise Exception("Invalid direction")

            dy = 0
            if p2[0] > p1[0]:
                dx = 1
            else:
                dx = -1

        return [p2, (dx,dy)]



    def get_end_rays(self, k):
        polyline = self.getIJPolyline(k)

        p0 = polyline[0]
        p1 = polyline[1]
        p2 = polyline[-2]
        p3 = polyline[-1]

        return (Fault.intRay(p1,p0), Fault.intRay(p2,p3))




    @staticmethod
    def join_faults(fault1, fault2, k):
        fault1_rays = fault1.getEndRays(k)
        fault2_rays = fault2.getEndRays(k)

        if fault1.intersectsFault(fault2, k):
            return None

        count = 0
        join = None
        try:
            join = Fault.intersectFaultRays(fault1_rays[0], fault2_rays[0])
            count += 1
        except ValueError:
            pass

        try:
            join = Fault.intersectFaultRays(fault1_rays[0], fault2_rays[1])
            count += 1
        except ValueError:
            pass

        try:
            join = Fault.intersectFaultRays(fault1_rays[1], fault2_rays[0])
            count += 1
        except ValueError:
            pass

        try:
            join = Fault.intersectFaultRays(fault1_rays[1], fault2_rays[1])
            count += 1
        except ValueError:
            pass

        if count == 1:
            xy_list = []
            for ij in join:
                xyz = fault1.__grid.getNodeXYZ(ij[0], ij[1], k)
                xy_list.append((xyz[0], xyz[1]))

            return xy_list
        else:
            return fault1.endJoin(fault2, k)


monkey_the_camel(FaultLayer, 'addSegment', FaultLayer.add_segment)
monkey_the_camel(FaultLayer, 'getK', FaultLayer.get_k)
monkey_the_camel(FaultLayer, 'getNeighborCells', FaultLayer.get_neighbor_cells)
monkey_the_camel(FaultLayer, 'getPolyline', FaultLayer.get_polyline)
monkey_the_camel(FaultLayer, 'getIJPolyline', FaultLayer.get_ij_polyline)
monkey_the_camel(FaultLayer, 'numLines', FaultLayer.num_lines)
monkey_the_camel(FaultLayer, 'processSegments', FaultLayer.process_segments)

monkey_the_camel(Fault, 'hasLayer', Fault.has_layer)
monkey_the_camel(Fault, 'addLayer', Fault.add_layer)
monkey_the_camel(Fault, 'createSegment', Fault.create_segment)
monkey_the_camel(Fault, 'addRecord', Fault.add_record)
monkey_the_camel(Fault, 'getName', Fault.get_name)
monkey_the_camel(Fault, 'getNeighborCells', Fault.get_neighbor_cells)
monkey_the_camel(Fault, 'getPolyline', Fault.get_polyline)
monkey_the_camel(Fault, 'getIJPolyline', Fault.get_ij_polyline)
monkey_the_camel(Fault, 'numLines', Fault.num_lines)
monkey_the_camel(Fault, 'connectWithPolyline', Fault.connect_with_polyline)
monkey_the_camel(Fault, 'extendToPolyline', Fault.extend_to_polyline)
monkey_the_camel(Fault, 'intersectsPolyline', Fault.intersects_polyline)
monkey_the_camel(Fault, 'intersectsFault', Fault.intersects_fault)
monkey_the_camel(Fault, 'extendToFault', Fault.extend_to_fault)
monkey_the_camel(Fault, 'extendToEdge', Fault.extend_to_edge)
monkey_the_camel(Fault, 'extendToBBox', Fault.extend_to_b_box)
monkey_the_camel(Fault, 'endJoin', Fault.end_join)
monkey_the_camel(Fault, 'connectPolylineOnto', Fault.connect_polyline_onto)
monkey_the_camel(Fault, 'extendPolylineOnto', Fault.extend_polyline_onto)
monkey_the_camel(Fault, 'intersectFaultRays', Fault.intersect_fault_rays, staticmethod)
monkey_the_camel(Fault, 'intRay', Fault.int_ray, staticmethod)
monkey_the_camel(Fault, 'getEndRays', Fault.get_end_rays)
monkey_the_camel(Fault, 'joinFaults', Fault.join_faults, staticmethod)
