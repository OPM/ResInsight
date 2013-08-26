#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'cclass.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class CClass(object):
    """
    Base class used by all the classes wrapping a C 'class'.

    All Python classes which wrap a C 'class', like e.g. the
    ecl_kw_type structure, need a from_param() classmethod which is
    used to 'translate' between the python object and the pointer to
    corresponding underlying C structure. The CClass class contains an
    implementation of such a from_param() method; all Python classes
    which wrap a C 'class' should inherit from this class.

    Python objects are managed with garbage collection, whereas the C
    structures which are wrapped must be managed manually. Users of
    the ert Python wrappers should assume 100% normal Python memory
    management, and /NOT/ take any special care due to the fact that
    the underlying C datastructures must be managed manually. This is
    achieved, but in the wrapping layer some care must be taken.
    
      - Each of the Python classes is centered around an opaque handle
        to the underlying C object; the c_ptr. This handle is passed
        as a 'self' argument when calling the C functions.

      - When a Python object goes out of scope the __del__() method is
        called, and this method can be used to call the xxx_free()
        method of the underlying C object.

    There are essentially two categories of C objects; either objects
    which are created with a xxxx_alloc() call - these objects should
    be destroyed when they go out scope. The other class of objects
    are references which are managed by another (container) class,
    e.g. an ecl_kw instance might be managed by an ecl_file
    container. In the case of of references the C object should not
    bee freed when the Python object goes out of scope, on the other
    hand we must ascertain that the container does /NOT/ go out of
    scope as long the child reference exists, otherwise the object
    will be destroyed under the reference.

    The handling of references and objects is performed with the
    init_cref() and init_cobj() methods below:

       - init_cref(): This method will store a 'pointer' to the Python
         container managing the reference. This will ensure that the
         container is not recycle prematurely. Consider the example:

            def load_poro( base ):
                file = ecl.EclFile( "%s.INIT" % base )
                poro = file["PORO"][0]
                return poro

            poro = load_poro( "CASE" )
            
         In this example the scope of the 'file' object is limited to
         the 'load_poro' function, whereas the poro keyword object is
         returned to the calling scope. If we did not take care to
         store a pointer to the file container that would be destroyed
         when returning from the load_poro function, and the poro
         object would be invalid.

         When a reference, like the poro variable in the example is
         destroyed no cleanup is performed on the underlying C
         storage, and the init_cref() method sets the cfree function
         pointer to None.

       - init_cobj(): This method is used for 'self-managed' objects,
         when the Python object goes out of scope the underlying C
         object should be destroyed as well. This is achieved by
         storing a function pointer to the xxxx_free() function of the
         C object.
    
    """
    c_ptr  = None
    cfree  = None
    parent = None
    
    @classmethod
    def from_param( cls , obj ):
        if obj is None:
            return ctypes.c_void_p()
        else:
            return ctypes.c_void_p( obj.c_ptr )

    @classmethod
    def asPythonObject( cls , c_ptr, cfree):
        assert cfree is not None
        obj = cls( )
        obj.init_cobj( c_ptr , cfree)
        return obj

    @classmethod
    def asPythonReference( cls , c_ptr , parent ):
        obj = cls( )
        obj.init_cref( c_ptr , parent )
        return obj


    def init_cref(self , c_ptr , parent):
        self.c_ptr = c_ptr
        self.parent = parent
        self.cfree = None


    def init_cobj( self , c_ptr , cfree):
        self.c_ptr = c_ptr
        self.parent = None
        self.cfree = cfree


    def __del__(self):
        if self.cfree:
            # Important to check the c_ptr; in the case of failed object creation
            # we can have a Python object with c_ptr == None.
            if self.c_ptr:
                self.cfree( self )
                
