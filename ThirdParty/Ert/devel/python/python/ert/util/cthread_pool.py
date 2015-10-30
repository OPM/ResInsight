#  Copyright (C) 2015  Statoil ASA, Norway. 
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
from ert.util import UTIL_LIB
from ert.cwrap import CWrapper, BaseCClass

class CThreadPool(BaseCClass):
    def __init__(self , pool_size , start = True):
        c_ptr = CThreadPool.cNamespace().alloc( pool_size , start )
        super(CThreadPool, self).__init__(c_ptr)
        self.arg_list = []
        
    def addTask(self , cfunc , arg):
        """ 
        The function should come from CThreadPool.lookupCFunction().
        """
        if isinstance(arg, BaseCClass):
            arg_ptr = BaseCClass.from_param( arg )
        else:
            arg_ptr = arg

        self.arg_list.append( arg )
        CThreadPool.cNamespace().add_job( self , cfunc , arg_ptr )


    def join(self):
        CThreadPool.cNamespace().join( self )

        
    def free(self):
        self.join( )
        CThreadPool.cNamespace().free( self )
        

    @staticmethod
    def lookupCFunction(lib , name):
        if isinstance(lib , ctypes.CDLL):
            func = getattr(lib , name)
            return func
        else:
            raise TypeError("The lib argument must of type ctypes.CDLL")

        

class CThreadPoolContextManager(object):

    def __init__(self , tp):
        self.__tp = tp
    
    def __enter__(self):
        return self.__tp

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.__tp.join()
        return False          


def startCThreadPool( size ):
    return CThreadPoolContextManager( CThreadPool( size , start = True ))


        
CWrapper.registerObjectType("thread_pool", CThreadPool)

cwrapper = CWrapper(UTIL_LIB)
CThreadPool.cNamespace().alloc = cwrapper.prototype("c_void_p thread_pool_alloc( int , bool )")
CThreadPool.cNamespace().free = cwrapper.prototype("void thread_pool_free( thread_pool )")
CThreadPool.cNamespace().add_job = cwrapper.prototype("void thread_pool_add_job( thread_pool , c_void_p , c_void_p)")
CThreadPool.cNamespace().join  = cwrapper.prototype("void thread_pool_join( thread_pool )")    
