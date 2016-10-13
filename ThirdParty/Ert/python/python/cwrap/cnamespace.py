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

class CNamespace(object):
    def __init__(self, name):
        object.__setattr__(self, "_name", name)
        object.__setattr__(self, "_functions", {})

    def __str__(self):
        return "%s wrapper" % self._name

    def __setitem__(self, key, value):
        self.__setValue(key, value)


    def __getitem__(self, item):
        return self._functions[item]

    def __setattr__(self, key, value):
        self.__setValue(key, value)

    def __setValue(self, key, value):
        assert not hasattr(self, key), "The namespace %s already contains a function named %s!" % (self._name, key)

        self._functions[key] = value
        object.__setattr__(self, key, value)
