#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'util_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import random
import copy

from   ert.util.tvector import DoubleVector
from   ert.util.tvector import IntVector
from   ert.util.tvector import BoolVector
from   ert.util.stringlist import StringList
import ert.util.stat    as stat
from   ert.util.util_func import *

v = DoubleVector( 0 )
print v
v[0]  =  77.25
v[1]  = 123.25
v[2]  =  66.25
v[3]  =  56.25
v[4]  = 111.25
v[5]  =  99.25
v[12] =  12
v.printf( )
v.clear()

for i in range( 100000 ):
    r = random.random()
    v.append( r )


print stat.quantile( v , 0.10 )
print stat.quantile_sorted( v , 0.20 )
print stat.quantile_sorted( v , 0.30 )
print stat.quantile_sorted( v , 0.40 )
print stat.quantile_sorted( v , 0.50 )

a = v[0:10]
print a
print a[6]
print a


b = copy.deepcopy( a )
c = a.copy(  )
print c
print b
print v
c = IntVector( 0 )
c.append( 1 )
c.append( 2 )
c.append( 3 )
c.append( 4 )
c.append( 5 )

c.rsort()
c.printf()

print c.max
print c.min
print c.min_index()
c[4] = 5
print c.max_index( reverse = True )
print c.max_index( reverse = False )

d = c + 1
d.printf()

v2  = v.copy()
v2  = v2 * 2
v2 += v

print v2.assign
v2.assign( 1.0 )
print v2[77]

v2.default = 100
print v2.default

B = BoolVector( True )
B[4] = False
B.printf()

l = ["CASE-9","CASE-10"]
print l.sort()
print l.sort( strcmp_int )

v.default = 13
v[1000] = 99
print v.size
np = v.numpy_copy()
v[0] = 77
v[5] = 99
print v.str( max_lines = None )

S = StringList(["A" , "list" ,"of" , "strings"])

print S

if "A" in S:
    print "S contains A"

if not "Bjarne" in S:
    print "S does not contain BJARNE"


