#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_vectors.py' is part of ERT - Ensemble based Reservoir Tool.
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


import copy
from datetime import datetime
from unittest2 import TestCase

from ert.util import DoubleVector, IntVector, BoolVector, TimeVector, ctime


class UtilTest(TestCase):
    def test_double_vector(self):
        v = DoubleVector()

        v[0] = 77.25
        v[1] = 123.25
        v[2] = 66.25
        v[3] = 56.25
        v[4] = 111.25
        v[5] = 99.25
        v[12] = 12

        self.assertEqual(len(v), 13)
        self.assertEqual(list(v), [v[0], v[1], v[2], v[3], v[4], v[5], 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, v[12]])

        v.clear()
        self.assertEqual(len(v), 0)

        v.clear()

        v[0] = 0.1
        v[1] = 0.2
        v[2] = 0.4
        v[3] = 0.8

        v2 = v * 2

        self.assertEqual(list(v2), [v[0] * 2, v[1] * 2, v[2] * 2, v[3] * 2])

        v2 += v
        self.assertEqual(list(v2), [v[0] * 3, v[1] * 3, v[2] * 3, v[3] * 3])

        v2.assign(0.66)
        self.assertEqual(list(v2), [0.66, 0.66, 0.66, 0.66])

        v.assign(v2)
        self.assertEqual(list(v), [0.66, 0.66, 0.66, 0.66])

        v.clear()
        v.default = 0.75
        self.assertEqual(v.default, 0.75)
        v[2] = 0.0
        self.assertEqual(v[1], 0.75)



    def test_vector_operations_with_exceptions(self):
        iv1 = IntVector()
        iv1.append(1)
        iv1.append(2)
        iv1.append(3)

        iv2 = IntVector()
        iv2.append(4)
        iv2.append(5)

        dv1 = DoubleVector()
        dv1.append(0.5)
        dv1.append(0.75)
        dv1.append(0.25)

        #Size mismatch
        with self.assertRaises(ValueError):
            iv3 = iv1 + iv2

        #Size mismatch
        with self.assertRaises(ValueError):
            iv3 = iv1 * iv2

        #Type mismatch
        with self.assertRaises(TypeError):
            iv1 += dv1

        #Type mismatch
        with self.assertRaises(TypeError):
            iv1 *= dv1


    def test_bool_vector(self):
        b = BoolVector()
        b.default = True

        b[4] = False

        self.assertEqual(list(b), [True, True, True, True, False])


    def test_activeList(self):
        active_list = IntVector.active_list("1,10,100-105")
        self.assertTrue(len(active_list) == 8)
        self.assertTrue(active_list[0] == 1)
        self.assertTrue(active_list[2] == 100)
        self.assertTrue(active_list[7] == 105)

        active_list = IntVector.active_list("1,10,100-105X")
        self.assertFalse(active_list)


    def test_activeMask(self):
        active_list = BoolVector.active_mask("1 , 4 - 7 , 10")
        self.assertTrue(len(active_list) == 11)
        self.assertTrue(active_list[1])
        self.assertTrue(active_list[4])
        self.assertTrue(active_list[10])
        self.assertFalse(active_list[9])
        self.assertFalse(active_list[8])

        active_list = BoolVector.active_mask("1,4-7,10X")
        self.assertFalse(active_list)




    def test_int_vector(self):
        a = IntVector()
        a.append(1)
        a.append(2)
        a.append(3)
        a.append(4)
        a.append(5)

        self.assertEqual(list(a), [1, 2, 3, 4, 5])

        a.rsort()
        self.assertEqual(list(a), [5, 4, 3, 2, 1])

        self.assertTrue(a.max, 5)
        self.assertTrue(a.min, 1)
        self.assertTrue(a.min_index(), 4)

        self.assertEqual(a.max_index(reverse=True), 0)
        self.assertEqual(a.max_index(reverse=False), 0)

        a[4] = 5
        self.assertTrue(a[4] == 5)

        a_plus_one = a + 1
        self.assertEqual(list(a_plus_one), [6, 5, 4, 3, 6])

        sliced = a[0:3]
        self.assertEqual(list(sliced), [5, 4, 3])

        with self.assertRaises(IndexError):
            item = a[6]

        copy_of_a = copy.deepcopy(a)
        self.assertEqual(list(a), list(copy_of_a))

        another_copy_of_a = copy.copy(a)
        self.assertEqual(list(a), list(another_copy_of_a))


    def test_div(self):
        v = IntVector()
        v[0] = 100
        v[1] = 10
        v[2] = 1
        v /= 10

        self.assertEqual(list(v), [10, 1, 0])


    def test_true(self):
        iv = IntVector()
        self.assertFalse(iv)    # Will invoke the __len__ function; could override with __nonzero__
        iv[0] = 1
        self.assertTrue(iv)



    def test_time_vector(self):
        time_vector = TimeVector()

        time1 = ctime(datetime(2013, 8, 13, 0, 0, 0))
        time2 = ctime(datetime(2013, 8, 13, 1, 0, 0))

        time_vector.default = time2

        time_vector.append(time1)
        time_vector[2] = time2

        self.assertEqual(time_vector[0], time1)
        self.assertEqual(time_vector[1], time2)
        self.assertEqual(time_vector[2], time2)

