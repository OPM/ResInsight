#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'test_util.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import filecmp

def is_number(x):
    try:
        y = x + 1
        return True
    except TypeError:
        return False
    
def approx_equal(a,b):
    if is_number(a):
        tol = 1e-6
        d   = abs(a - b)
        s   = max(1 , abs(a) + abs(b))
        
        return d < tol * s
    else:
        return a == b

def approx_equalv(a,b):
    equal = False
    if len(a) == len(b):
        equal = True
        for (ai,bi) in zip(a,b):
            if not approx_equal(ai,bi):
                equal = False
                break
    return equal
            

def file_equal(f1,f2):
    buffer1 = open(f1).read()
    buffer2 = open(f2).read()
    if buffer1 == buffer2:
        return True
    else:
        return False


