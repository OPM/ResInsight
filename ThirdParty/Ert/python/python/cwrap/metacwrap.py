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

from __future__ import (absolute_import, division,
                        print_function, unicode_literals)
import six

import re
from types import MethodType

from .prototype import Prototype


def snakeCase(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


class MetaCWrap(type):
    def __init__(cls, name, bases, attrs):
        super(MetaCWrap, cls).__init__(name, bases, attrs)

        is_return_type = False
        storage_type = None

        if "TYPE_NAME" in attrs:
            type_name = attrs["TYPE_NAME"]
        else:
            type_name = snakeCase(name)

        if hasattr(cls, "DATA_TYPE") or hasattr(cls, "enums"):
            is_return_type = True

        if hasattr(cls, "storageType"):
            storage_type = cls.storageType()

        Prototype.registerType(type_name, cls, is_return_type=is_return_type, storage_type=storage_type)

        if hasattr(cls, "createCReference"):
            Prototype.registerType("%s_ref" % type_name, cls.createCReference, is_return_type=True, storage_type=storage_type)

        if hasattr(cls, "createPythonObject"):
            Prototype.registerType("%s_obj" % type_name, cls.createPythonObject, is_return_type=True, storage_type=storage_type)


        for key, attr in attrs.items():
            if isinstance(attr, Prototype):
                attr.resolve()
                attr.__name__ = key

                if attr.shouldBeBound():
                    method = MethodType(attr, None, cls)
                    #method = six.create_bound_method(attr, cls)
                    setattr(cls, key, method)
