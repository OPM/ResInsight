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
import datetime

try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from ecl.util import DoubleVector, IntVector, BoolVector, TimeVector, CTime, PermutationVector

class UtilTest(TestCase):
    def setUp(self):
        pass

    def dotest_slicing(self, vec):
        self.assertEqual(10, len(vec))
        self.assertEqual(vec[-1], vec[9])
        self.assertEqual(8, len(vec[:8]))
        self.assertEqual(9, len(vec[1:]))
        self.assertEqual(3, len(vec[1:8:3]))
        odds = vec[1::2]
        self.assertEqual(4, len(vec[1:8:2]))
        for i in range(4):
            self.assertEqual(vec[2*i + 1], odds[i])

    def test_slicing(self):
        dv = DoubleVector(initial_size=10)
        for i in range(10):
            dv[i] = 1.0 / (1+i)
        self.dotest_slicing(dv)
        iv = IntVector(initial_size=10)
        for i in range(10):
            iv[i] = i**3
        self.dotest_slicing(iv)
        bv = BoolVector(initial_size=10)
        for i in range(0,10,3):
            bv[i] = True
        self.dotest_slicing(bv)
        tv = TimeVector(initial_size=10)
        for i in range(10):
            tv[i] = CTime(datetime.datetime(2016, 12, i+3, 0, 0, 0))
        self.dotest_slicing(tv)



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
        v.setDefault(0.75)
        self.assertEqual(v.getDefault(), 0.75)
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

        # Size mismatch
        with self.assertRaises(ValueError):
            iv3 = iv1 + iv2

        # Size mismatch
        with self.assertRaises(ValueError):
            iv3 = iv1 * iv2

        # Type mismatch
        with self.assertRaises(TypeError):
            iv1 += dv1

        # Type mismatch
        with self.assertRaises(TypeError):
            iv1 *= dv1

    def test_setitem_getitem(self):
        primes = [2,3,5,7,11,13,17,19]
        primep = [i in primes for i in range(20)]
        b = BoolVector(initial_size = 20)
        b[2] = True
        b[3:8:2] = True
        b[11::2] = True
        self.assertTrue(b[15])
        self.assertTrue(b[-5])
        self.assertTrue(b[17])
        self.assertTrue(b[19])
        b[-5] = False
        self.assertEqual(list(b), primep)

    def test_repr(self):
        primes = [2,3,5,7,11,13,17,19]
        b = BoolVector()
        for i in primes:
            b[i] = True
        pfx = 'BoolVector(size = 20, content = "00110101000101000101")'
        print(repr(b))
        self.assertEqual(pfx, repr(b)[:len(pfx)])
        b[30] = True
        pfx = 'BoolVector(size = 31, content = "001101010...00000001")'
        print(repr(b))
        self.assertEqual(pfx, repr(b)[:len(pfx)])


    def test_bool_vector(self):
        b = BoolVector()
        b.setDefault(True)

        b[4] = False

        self.assertEqual(list(b), [True, True, True, True, False])

    def test_activeList(self):
        active_list = IntVector.active_list("1,10,100-105")
        self.assertTrue(len(active_list) == 8)
        self.assertTrue(active_list[0] == 1)
        self.assertTrue(active_list[2] == 100)
        self.assertTrue(active_list[7] == 105)
        self.assertEqual(active_list.count(100), 1)
        active_list.append(100)
        active_list.append(100)
        self.assertEqual(active_list.count(100), 3)

        active_list = IntVector.active_list("1,10,100-105X")
        self.assertFalse(active_list)
        
    def test_value_list(self):
        list2 = IntVector.valueList("3,10-12,0,1")
        self.assertTrue(len(list2) == 6)
        expected = [3, 10, 11, 12, 0, 1]
        for v1, v2 in zip(list2, expected):
            self.assertEqual(v1, v2)

    def test_contains_int(self):
        iv = IntVector()
        iv[0] = 1
        iv[1] = 10
        iv[2] = 100
        iv[3] = 1000

        self.assertTrue(1 in iv)
        self.assertTrue(10 in iv)
        self.assertTrue(88 not in iv)
        self.assertTrue(99 not in iv)

    def test_activeMask(self):
        active_list = BoolVector.createActiveMask("1 , 4 - 7 , 10")
        self.assertTrue(len(active_list) == 11)
        self.assertTrue(active_list[1])
        self.assertTrue(active_list[4])
        self.assertTrue(active_list[10])
        self.assertFalse(active_list[9])
        self.assertFalse(active_list[8])

        self.assertEqual(6, active_list.count(True))

        active_list = BoolVector.createActiveMask("1,4-7,10X")
        self.assertFalse(active_list)

    def test_update_active_mask(self):
        vec = BoolVector(False, 10)

        self.assertTrue(vec.updateActiveMask("1-2,5"))
        self.assertTrue(vec[1])
        self.assertTrue(vec[2])
        self.assertTrue(vec[5])
        self.assertFalse(vec[4])

        vec = BoolVector(False, 10)

        self.assertTrue(vec.updateActiveMask("1-5,2,3"))
        self.assertTrue(vec[1])
        self.assertTrue(vec[2])
        self.assertTrue(vec[3])
        self.assertTrue(vec[4])
        self.assertTrue(vec[5])
        self.assertFalse(vec[0])
        self.assertFalse(vec[6])

        vec = BoolVector(False, 10)

        self.assertTrue(vec.updateActiveMask("5,6,7,15"))
        self.assertTrue(vec[5])
        self.assertTrue(vec[6])
        self.assertTrue(vec[7])
        self.assertFalse(vec[4])
        self.assertFalse(vec[8])
        self.assertEqual(len(vec), 16)

    def test_pop(self):
        a = IntVector()
        a.append(1)
        a.append(2)

        self.assertEqual(a.pop(), 2)
        self.assertEqual(len(a), 1)
        self.assertEqual(a.pop(), 1)
        self.assertEqual(len(a), 0)
        with self.assertRaises(ValueError):
            a.pop()

#----
            
    def test_shift(self):
        a = IntVector()
        a.append(1)
        a.append(2)
        a.append(3)
        a.append(4)
        a.append(5)

        with self.assertRaises(ValueError):
            a >> -1

        with self.assertRaises(ValueError):
            a << -1

        with self.assertRaises(ValueError):
            a << -6

        b = a << 2
        self.assertEqual(list(b), [3, 4, 5])

        print(a)
        a <<= 2
        print(a)
        self.assertEqual(list(a), [3, 4, 5])

        b = a >> 2
        self.assertEqual(list(b), [0, 0, 3, 4, 5])

        a >>= 2
        self.assertEqual(list(a), [0, 0, 3, 4, 5])

    def test_int_vector(self):
        a = IntVector()
        a.append(1)
        a.append(2)
        a.append(3)
        a.append(4)
        a.append(5)

        self.assertEqual(list(a), [1, 2, 3, 4, 5])

        a.sort(reverse=True)
        self.assertEqual(list(a), [5, 4, 3, 2, 1])

        self.assertTrue(a.max(), 5)
        self.assertTrue(a.min(), 1)
        self.assertTrue(a.minIndex(), 4)
        
        self.assertEqual(a.maxIndex(reverse=True), 0)
        self.assertEqual(a.maxIndex(reverse=False), 0)
        
        a[4] = 5
        self.assertTrue(a[4] == 5)
        
        a_plus_one = a + 1
        self.assertEqual(list(a_plus_one), [6, 5, 4, 3, 6])

        sliced = a[0:3]
        self.assertEqual(list(sliced), [5, 4, 3])
        
        with self.assertRaises(IndexError):
            item = a[6]
        
        copy_of_a = a.copy()
        self.assertEqual(list(a), list(copy_of_a))
        
        another_copy_of_a = copy_of_a.copy( )
        self.assertEqual(list(a), list(another_copy_of_a))

#---

    def test_div(self):
        v = IntVector()
        v[0] = 100
        v[1] = 10
        v[2] = 1
        v /= 10

        self.assertEqual(list(v), [10, 1, 0])

    def test_true(self):
        iv = IntVector()
        self.assertFalse(iv)  # Will invoke the __len__ function; could override with __nonzero__
        iv[0] = 1
        self.assertTrue(iv)

    def test_time_vector(self):
        time_vector = TimeVector()

        time1 = CTime(datetime.datetime(2013, 8, 13, 0, 0, 0))
        time2 = CTime(datetime.datetime(2013, 8, 13, 1, 0, 0))

        time_vector.setDefault(time2)

        time_vector.append(time1)
        time_vector[2] = time2

        self.assertEqual(time_vector[0], time1)
        self.assertEqual(time_vector[1], time2)
        self.assertEqual(time_vector[2], time2)

        tv1 = TimeVector(default_value=datetime.date(2000, 1, 1), initial_size=2)
        self.assertEqual(tv1[0], datetime.date(2000, 1, 1))

        tv2 = TimeVector()
        tv2.append(time2)
        print(tv2)

    def test_permutation_vector(self):
        vector = DoubleVector()

        for i in range(5, 0, -1):
            vector.append(i)

        permutation_vector = vector.permutationSort()

        for index, value in enumerate(range(5, 0, -1)):
            self.assertEqual(vector[index], value)

        vector.permute(permutation_vector)

        for index, value in enumerate(range(1, 6)):
            self.assertEqual(vector[index], value)

    def test_contains_time(self):
        start = datetime.datetime(2010, 1, 1, 0, 0, 0)
        end = datetime.datetime(2010, 2, 1, 0, 0, 0)
        other = datetime.datetime(2010, 1, 15, 0, 0, 0)

        tv = TimeVector()
        tv.append(start)
        tv.append(end)

        self.assertTrue(start in tv)
        self.assertTrue(end in tv)
        self.assertTrue(other not in tv)

    def test_unique(self):
        iv = IntVector()
        iv.append(1)
        iv.append(1)
        iv.append(1)
        iv.append(0)
        iv.append(1)
        iv.append(2)
        iv.append(2)
        iv.append(0)
        iv.append(3)
        iv.selectUnique()
        self.assertEqual(len(iv), 4)
        self.assertEqual(iv[0], 0)
        self.assertEqual(iv[1], 1)
        self.assertEqual(iv[2], 2)
        self.assertEqual(iv[3], 3)

    def test_element_sum(self):
        dv = DoubleVector()
        iv = IntVector()
        for i in range(10):
            dv.append(i + 1)
            iv.append(i + 1)

        self.assertEqual(dv.elementSum(), 55)
        self.assertEqual(iv.elementSum(), 55)

    def test_time_vector_regular(self):
        start = datetime.datetime(2010, 1, 1, 0, 0, 0)
        end = datetime.datetime(2010, 2, 1, 0, 0, 0)

        with self.assertRaises(ValueError):
            trange = TimeVector.createRegular(end, start, "1X")

        with self.assertRaises(TypeError):
            trange = TimeVector.createRegular(start, end, "1X")

        with self.assertRaises(TypeError):
            trange = TimeVector.createRegular(start, end, "1")

        with self.assertRaises(TypeError):
            trange = TimeVector.createRegular(start, end, "X")

        with self.assertRaises(TypeError):
            trange = TimeVector.createRegular(start, end, "1.5Y")

        trange = TimeVector.createRegular(start, end, "d")
        trange = TimeVector.createRegular(start, end, "D")
        trange = TimeVector.createRegular(start, end, "1d")
        self.assertEqual(trange[0].datetime(), start)
        self.assertEqual(trange[-1].datetime(), end)
        date = start
        delta = datetime.timedelta(days=1)
        for t in trange:
            self.assertEqual(t, date)
            date += delta

        end = datetime.datetime(2010, 1, 10, 0, 0, 0)
        trange = TimeVector.createRegular(start, end, "2d")
        self.assertEqual(trange[-1].datetime(), datetime.datetime(2010, 1, 9, 0, 0, 0))
        self.assertEqual(5, len(trange))

        end = datetime.datetime(2012, 1, 10, 0, 0, 0)
        trange = TimeVector.createRegular(start, end, "3M")
        self.assertTrue(trange[-1] == datetime.datetime(2012, 1, 1, 0, 0, 0))
        self.assertTrue(trange[1] == datetime.datetime(2010, 4, 1, 0, 0, 0))
        self.assertTrue(trange[2] == datetime.datetime(2010, 7, 1, 0, 0, 0))
        self.assertTrue(trange[3] == datetime.datetime(2010, 10, 1, 0, 0, 0))
        self.assertTrue(trange[4] == datetime.datetime(2011, 1, 1, 0, 0, 0))

        start = datetime.datetime(1980, 1, 1, 0, 0, 0)
        end = datetime.datetime(2020, 1, 1, 0, 0, 0)
        trange = TimeVector.createRegular(start, end, "2Y")
        for (y, t) in zip(xrange(1980, 2022, 2), trange):
            self.assertTrue(t == datetime.datetime(y, 1, 1, 0, 0, 0))

        trange = TimeVector.createRegular(start, datetime.date(2050, 1, 1), "1Y")

    def test_asList(self):
        v = IntVector()
        v[0] = 100
        v[1] = 10
        v[2] = 1

        l = v.asList()
        self.assertListEqual(l, [100, 10, 1])

    def test_true_false(self):
        v = IntVector(default_value=77)
        self.assertFalse(v)
        v[10] = 77
        self.assertTrue(v)

        v = DoubleVector(default_value=77)
        self.assertFalse(v)
        v[10] = 77
        self.assertTrue(v)

    def test_count_equal(self):
        v = IntVector(default_value=77)
        v[0] = 1
        v[10] = 1
        v[20] = 1
        self.assertEqual(v.countEqual(1), 3)

        v = DoubleVector(default_value=77)
        v[0] = 1
        v[10] = 1
        v[20] = 1
        self.assertEqual(v.countEqual(1), 3)

    def range_test(self, v, a, b, d):
        v.initRange(a, b, d)
        r = range(a, b, d)

        self.assertEqual(len(v), len(r))
        for a, b in zip(v, r):
            self.assertEqual(a, b)

    def create_range_test(self, v, a, b, d):
        v = IntVector.createRange(a, b, d)
        r = range(a, b, d)

        self.assertEqual(len(v), len(r))
        for a, b in zip(v, r):
            self.assertEqual(a, b)

    def test_range(self):
        v = IntVector()
        v[10] = 99

        with self.assertRaises(ValueError):
            v.initRange(1, 2, 0)

        self.range_test(v, 0, 5, 1)
        self.range_test(v, 0, 100, 3)
        self.range_test(v, 0, 100, -3)

        self.create_range_test(v, 0, 5, 1)
        self.create_range_test(v, 0, 100, 3)
        self.create_range_test(v, 0, 100, -3)


        
    def test_perm_vector(self):
        v = IntVector.createRange( 11 , 0 , -1 )
        perm = v.permutationSort( )
        self.assertEqual( perm[0]  , 10 )
        self.assertEqual( perm[5]  ,  5 )
        self.assertEqual( perm[10] ,  0 )
