#!/usr/bin/env python
#  Copyright (C) 2018  Statoil ASA, Norway.
#
#  The file 'test_grid.pandas' is part of ERT - Ensemble based Reservoir Tool.
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
import pandas as pd

from ecl import  EclTypeEnum

from ecl.eclfile import EclKW

from ecl.grid import EclGrid

from tests import EclTest

class GridPandasTest(EclTest):

  def test_dataframe_actnum(self):
    grid = EclGrid.create_rectangular( (2,3,1), (1,1,1) , actnum=[1, 1, 0, 0, 1, 1])
    df = grid.export_index(True)
    index_matrix = np.array([ [0, 0, 0, 0],
                              [1, 0, 0, 1], 
                              [0, 2, 0, 2],
                              [1, 2, 0, 3] ])
    assert( np.array_equal(df.values, index_matrix) )

    kw_int_active = EclKW('int_act', 4, EclTypeEnum.ECL_INT_TYPE)
    kw_int_active[0] = 9
    kw_int_active[1] = 8
    kw_int_active[2] = 7
    kw_int_active[3] = 6
    data = grid.export_data(df, kw_int_active)
    assert( len(data) == 4 )
    assert( np.array_equal(data, np.array([9, 8, 7, 6]))  )

    kw_float_active = EclKW('float_at', 4, EclTypeEnum.ECL_FLOAT_TYPE)
    kw_float_active[0] = 10.5
    kw_float_active[1] = 9.25
    kw_float_active[2] = 2.0
    kw_float_active[3] = 1.625
    data = grid.export_data(df, kw_float_active)
    assert( len(data) == 4 )
    assert( np.array_equal(data, np.array([10.5, 9.25, 2.0, 1.625])) )

    kw_int_global = EclKW('int_glob', 6, EclTypeEnum.ECL_INT_TYPE)
    kw_int_global[0] = 0
    kw_int_global[1] = 2
    kw_int_global[2] = 4
    kw_int_global[3] = 6
    kw_int_global[4] = 8
    kw_int_global[5] = 9
    data = grid.export_data(df, kw_int_global)
    assert( len(data) == 4)
    assert( np.array_equal(data, np.array([0, 2, 8, 9]))  )

    kw_double_global = EclKW('double_g', 6, EclTypeEnum.ECL_DOUBLE_TYPE)
    kw_double_global[0] = 1.1
    kw_double_global[1] = 2.2
    kw_double_global[2] = 3.3
    kw_double_global[3] = 4.4
    kw_double_global[4] = 5.5
    kw_double_global[5] = 6.6
    data = grid.export_data(df, kw_double_global)
    assert( np.array_equal(data, np.array([1.1, 2.2, 5.5, 6.6])) )

    df = grid.export_index()      #DataFrame has now 6 rows
    global_index = df.index;
    assert( np.array_equal(global_index, np.array([0, 1, 2, 3, 4, 5])) )
    
    data = grid.export_data(df, kw_int_active, 9999)
    assert( np.array_equal(data, np.array([9, 8, 9999, 9999, 7, 6])) )

    data = grid.export_data(df, kw_float_active, 2222.0)
    assert( np.array_equal(data, np.array([10.5, 9.25, 2222.0, 2222.0, 2.0, 1.625])) )

    
  def test_dataframe_grid_data(self):
    grid = EclGrid.create_rectangular( (2,3,1), (1,1,1) , actnum=[1, 1, 0, 0, 1, 1])
    index_frame = grid.export_index()
    volume_data = grid.export_volume(index_frame)
    assert( len(volume_data) == 6 )
    assert( np.array_equal(volume_data, np.array([1.0, 1.0, 1.0, 1.0, 1.0, 1.0]))  )
    
    position_data = grid.export_position(index_frame)
    x_pos = position_data[:, 0]
    y_pos = position_data[:, 1]
    z_pos = position_data[:, 2]
    assert( np.array_equal(x_pos, np.array([0.5, 1.5, 0.5, 1.5, 0.5, 1.5])) )
    assert( np.array_equal(y_pos, np.array([0.5, 0.5, 1.5, 1.5, 2.5, 2.5])) )
    assert( np.array_equal(z_pos, np.array([0.5, 0.5, 0.5, 0.5, 0.5, 0.5])) )
    
    corner_data = grid.export_corners(index_frame)
    compare = np.array([[0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0], 
                        [1.0, 0.0, 0.0, 2.0, 0.0, 0.0, 1.0, 1.0, 0.0, 2.0, 1.0, 0.0, 1.0, 0.0, 1.0, 2.0, 0.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0], 
                        [0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 2.0, 0.0, 1.0, 2.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 2.0, 1.0, 1.0, 2.0, 1.0], 
                        [1.0, 1.0, 0.0, 2.0, 1.0, 0.0, 1.0, 2.0, 0.0, 2.0, 2.0, 0.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 2.0, 1.0, 2.0, 2.0, 1.0], 
                        [0.0, 2.0, 0.0, 1.0, 2.0, 0.0, 0.0, 3.0, 0.0, 1.0, 3.0, 0.0, 0.0, 2.0, 1.0, 1.0, 2.0, 1.0, 0.0, 3.0, 1.0, 1.0, 3.0, 1.0], 
                        [1.0, 2.0, 0.0, 2.0, 2.0, 0.0, 1.0, 3.0, 0.0, 2.0, 3.0, 0.0, 1.0, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 3.0, 1.0, 2.0, 3.0, 1.0]])
    assert( np.array_equal(corner_data, compare) ) 
   
    
