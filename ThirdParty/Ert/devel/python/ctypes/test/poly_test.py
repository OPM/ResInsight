#!/usr/bin/env python
import ert.ecl.ecl as ecl

g = ecl.EclGrid( "data/eclipse/case/ECLIPSE" )
r = ecl.EclRegion( g , False )

ll = g.get_xyz( ijk = (0,0,0) )
ur = g.get_xyz( ijk = (39,63,0) )

dx = 0.5*(ur[0] - ll[0])
dy = 0.5*(ur[1] - ll[1])

x0 = ll[0] + dx*0.5
y0 = ll[1] + dy*0.5
x1 = x0 + dx
y1 = y0 + dy


points = [(x0,y0) , (x1,y0) , (x1,y1) , (x0,y1)]


print "            X ----------------------------------- (%10.3f , %10.3f)" % (x1-x0,y1-y0)
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "            |                                                 |"
print "(%10.3f , %10.3f) ---------------------------------- X\n\n" % (x0-x0,y0-y0)

r.select_inside_polygon( points )
for gi in r.global_list:
    (x,y,z) = g.get_xyz( global_index = gi )
    (i,j,k) = g.get_ijk( global_index = gi )
    print "(ijk) = (%d,%d,%d)  x,y = (%10.3f,%10.3f)" % (i,j,k,x-x0,y-y0)

print "-------------------------"

j = 15
k = 0
for i in (10,11,12):
    (x,y,z) = g.get_xyz( ijk = (i,j,k) )
    print "(ijk) = (%d,%d,%d)  x,y = (%10.3f,%10.3f)" % (i,j,k,x-x0,y-y0)
    
