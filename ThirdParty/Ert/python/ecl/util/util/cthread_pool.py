#  Copyright (C) 2015  Equinor ASA, Norway.
#
#  The file 'cthread_pool.py' is part of ERT - Ensemble based Reservoir Tool.
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

import ctypes

from cwrap import BaseCClass
from ecl import EclPrototype

import weakref


class CThreadPool(BaseCClass):
    TYPE_NAME = "thread_pool"

    _alloc   = EclPrototype("void* thread_pool_alloc(int, bool)", bind = False)
    _free    = EclPrototype("void thread_pool_free(thread_pool)")
    _add_job = EclPrototype("void thread_pool_add_job(thread_pool, void*, void*)")
    _join    = EclPrototype("void thread_pool_join(thread_pool)")

    def __init__(self, pool_size, start=True):
        c_ptr = self._alloc(pool_size, start)
        super(CThreadPool, self).__init__(c_ptr)
        self.arg_list = []

    def addTaskFunction(self, name, lib, c_function_name):
        function = CThreadPool.lookupCFunction(lib, c_function_name)
        self_ref = weakref.ref(self)  # avoid circular dependencies

        def wrappedFunction(arg):
            return self_ref().addTask(function, arg)

        setattr(self, name, wrappedFunction)

    def addTask(self, cfunc, arg):
        """
        The function should come from CThreadPool.lookupCFunction().
        """
        if isinstance(arg, BaseCClass):
            arg_ptr = BaseCClass.from_param(arg)
        else:
            arg_ptr = arg

        self.arg_list.append(arg)
        self._add_job(cfunc, arg_ptr)

    def join(self):
        self._join()

    def free(self):
        self.join()
        self._free()

    @staticmethod
    def lookupCFunction(lib, name):
        if isinstance(lib, ctypes.CDLL):
            func = getattr(lib, name)
            return func
        else:
            raise TypeError("The lib argument must be of type ctypes.CDLL")


class CThreadPoolContextManager(object):
    def __init__(self, tp):
        self.__tp = tp

    def __enter__(self):
        return self.__tp

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.__tp.join()
        return False


def startCThreadPool(size):
    return CThreadPoolContextManager(CThreadPool(size, start=True))
