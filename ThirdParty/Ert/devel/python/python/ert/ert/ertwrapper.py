#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ertwrapper.py' is part of ERT - Ensemble based Reservoir Tool. 
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



from ctypes import *
import ctypes.util
import atexit
import re
import sys
import os
import erttypes
import ert
#import ert.enkf.enkf as enkf
import ert.enkf.enkf_main as enkf

def RH_version():
    RH  = open('/etc/redhat-release' , 'r').read().split()[6]
    return float( RH )



class ErtWrapper:
    """Wraps the functionality of ERT using ctypes"""

    def __init__( self ):
        self.__loadLibraries( )

        self.pattern = re.compile("(?P<return>[a-zA-Z][a-zA-Z0-9_*]*) +(?P<function>[a-zA-Z]\w*) *[(](?P<arguments>[a-zA-Z0-9_*, ]*)[)]")
        self.__registerDefaultTypes()

        
    def bootstrap(self, enkf_config , site_config , strict = True):
        self.main = enkf.EnKFMain.bootstrap( enkf_config, site_config, strict )
        print "\nBootstrap complete!"

        self.initializeTypes()

        atexit.register(self.cleanup)


    def __loadLibrary(self, name):
        lib = "%s.so" % name
        try:
            lib_handle = CDLL( lib, RTLD_GLOBAL )
            return lib_handle
        except: 
            raise AssertionError("Can not find library: %s" % (name))
            


    def __loadLibraries(self ):
        """Load libraries that are required by ERT and ERT itself"""
        CDLL("libblas.so"   , RTLD_GLOBAL)
        CDLL("liblapack.so" , RTLD_GLOBAL)
        CDLL("libz.so"      , RTLD_GLOBAL)
        CDLL("libnsl.so"    , RTLD_GLOBAL)
        
        LSF_HOME = os.getenv("LSF_HOME")
        if LSF_HOME:
            CDLL("%s/lib/liblsf.so" % LSF_HOME   , RTLD_GLOBAL)
            CDLL("%s/lib/libbat.so" % LSF_HOME   , RTLD_GLOBAL)
        else:
            sys.exit("Need a value for environment variable LSF_HOME")
        
        self.util = self.__loadLibrary( "libert_util" )
        self.__loadLibrary( "libert_geometry" )
        self.ecl  = self.__loadLibrary( "libecl" )
        self.__loadLibrary( "libsched" )
        self.__loadLibrary( "librms"    )
        self.__loadLibrary( "libconfig" )
        self.__loadLibrary( "libanalysis" )
        self.job_queue = self.__loadLibrary( "libjob_queue" )
        self.enkf      = self.__loadLibrary( "libenkf" )

        self.enkf.enkf_main_install_SIGNALS()

        
    def __registerDefaultTypes(self):
        """Registers the default available types for prototyping."""
        self.registered_types = {}
        self.registerType("void", None)
        self.registerType("int", ctypes.c_int)
        self.registerType("int*", ctypes.POINTER(ctypes.c_int))
        self.registerType("bool", ctypes.c_int)
        self.registerType("bool*", ctypes.POINTER(ctypes.c_int))
        self.registerType("long", ctypes.c_long)
        self.registerType("long*", ctypes.POINTER(ctypes.c_long))
        self.registerType("char", ctypes.c_char)
        self.registerType("char*", ctypes.c_char_p)
        self.registerType("float", ctypes.c_float)
        self.registerType("float*", ctypes.POINTER(ctypes.c_float))
        self.registerType("double", ctypes.c_double)
        self.registerType("double*", ctypes.POINTER(ctypes.c_double))

    def registerType(self, type, value):
        """Register a type against a legal ctypes type"""
        self.registered_types[type] = value

    def __parseType(self, type):
        """Convert a prototype definition type from string to a ctypes legal type."""
        type = type.strip()

        if self.registered_types.has_key(type):
            return self.registered_types[type]
        else:
            return getattr(ctypes, type)

    def prototype(self, prototype, lib=None):
        """
        Defines the return type and arguments for a C-function

        prototype expects a string formatted like this:

            #type functionName(type, ... ,type)#

        where type is a type available to ctypes
        Some type are automatically converted:
            int -> c_int
            long -> c_long
            char -> c_char_p
            bool -> c_int
            void -> None
            double -> c_double
            float -> c_float

        There are also pointer versions of these:
            long* -> POINTER(c_long)
            bool* -> POINTER(c_int)
            double* -> POINTER(c_double)
            char* -> c_char_p
            ...

        if lib is None lib defaults to the enkf library
        """
        if lib is None:
            lib = self.enkf

        match = re.match(self.pattern, prototype)
        if not match:
            sys.stderr.write("Illegal prototype definition: %s\n" % (prototype))
            return None
        else:
            restype = match.groupdict()["return"]
            functioname = match.groupdict()["function"]
            arguments = match.groupdict()["arguments"].split(",")

            func = getattr(lib , functioname)
            func.restype = self.__parseType(restype)

            if len(arguments) == 1 and arguments[0].strip() == "":
                func.argtypes = []
            else:
                argtypes = [self.__parseType(arg) for arg in arguments]
                if len(argtypes) == 1 and argtypes[0] is None:
                    argtypes = []
                func.argtypes = argtypes

            #print func, func.restype, func.argtyp
            return func

    def initializeTypes(self):
        self.prototype("long hash_iter_alloc(long)", lib=self.util)
        self.prototype("char* hash_iter_get_next_key(long)", lib=self.util)
        self.prototype("char* hash_get(long, char*)", lib=self.util)
        self.prototype("int hash_get_int(long, char*)", lib=self.util)
        self.prototype("void hash_iter_free(long)", lib=self.util)
        self.prototype("bool hash_iter_is_complete(long)", lib=self.util)

        self.prototype("int subst_list_get_size(long)", lib=self.util)
        self.prototype("char* subst_list_iget_key(long, int)", lib=self.util)
        self.prototype("char* subst_list_iget_value(long, int)", lib=self.util)

        self.prototype("void enkf_main_fprintf_config(long)")
        self.prototype("void enkf_main_create_new_config(long , char*, char* , char* , int)")
        

        self.registerType("time_t", erttypes.time_t)
        erttypes.time_vector.initialize(self)
        erttypes.double_vector.initialize(self)

        
    def getHash(self, hashpointer, intValue = False, return_type="char*"):
        """Retrieves a hash as a list of 2 element lists"""
        if hashpointer == 0:
            return []

        hash_iterator = self.util.hash_iter_alloc(hashpointer)
        self.prototype("%s hash_get(long)" % (return_type), lib = self.util)

        result = []
        while not self.util.hash_iter_is_complete(hash_iterator):
            key   = self.util.hash_iter_get_next_key(hash_iterator)

            if not intValue:
                value = self.util.hash_get(hashpointer, key)
            else:
                value = self.util.hash_get_int(hashpointer, key)
                #print "%s -> %d" % (key , value)

            result.append([key, str(value)])

        self.util.hash_iter_free(hash_iterator)
        #print result
        return result

    def getSubstitutionList(self, substlistpointer):
        """Retrieves a substitution list as a list of 2 element lists"""
        size = self.util.subst_list_get_size(substlistpointer)

        result = []
        for index in range(size):
            key = self.util.subst_list_iget_key(substlistpointer, index)
            value = self.util.subst_list_iget_value(substlistpointer, index)
            result.append([key, value])

        return result

    def __getErtPointer(self, function):
        """Returns a pointer from ERT as a c_long (64-bit support)"""
        func = getattr( self.enkf, function )
        func.restype = c_long   # Should be c_size_t - if that exists.
        return func( self.main )

    
    def cleanup(self):
        """Called at atexit to clean up before shutdown"""
        print "Calling enkf_main_free()"
        self.main.__del__

    def nonify(self, s):
        """Convert an empty string to None."""
        return s or None

    def save(self):
        """Save the state of ert to a configuration file."""
        self.main.fprintf_config


        


