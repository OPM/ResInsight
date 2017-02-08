#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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
from cwrap import BaseCClass
from ert.enkf import NodeId, FieldConfig
from ert.enkf import EnkfPrototype
import ctypes

class BlockDataConfig(BaseCClass):
    TYPE_NAME = "block_data_config"

    def __init__(self):
        raise NotImplementedError('Cannot instantiate BlockDataConfig!')

    @classmethod
    def from_param(cls , instance):
        if instance is None:
            return ctypes.c_void_p()
        elif isinstance(instance , FieldConfig):
            return FieldConfig.from_param( instance )

        # The Container class which is used to support summary based
        # source in the BLOCK_OBS configuration is not yet supported
        # in Python.

        #elif isinstance(instance , ContainerConfig):
        #    return ContainerConfig.from_param( instance )
        else:
            raise ValueError("Currently ONLY field data is supported")
