#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'stringlist.py' is part of ERT - Ensemble based Reservoir Tool. 
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
"""
Simple wrapping of stringlist 'class' from C library.

The stringlist type from the libutil C library is a simple structure
consisting of a vector of \0 terminated char pointers - slightly
higher level than the (char ** string , int size) convention.

For a pure Python application you should just stick with a normal
Python list of string objects; but when interfacing with the C
libraries there are situations where you might need to instantiate a
StringList object. 

The StringList constructor can take an optional argument which should
be an iterable consisting of strings, and the strings property will
return a normal python list of string objects, used in this way you
hardly need to notice that the StringList class is at play.
"""
import sys

from ert.util import UTIL_LIB
from types import StringType, IntType
from ert.cwrap import CWrapper, BaseCClass


class StringList(BaseCClass):

    def __init__(self, initial=None):
        """
        Creates a new stringlist instance.
        
        Creates a new stringlist instance. The optional argument
        @initial should be an iterable of strings which will be the
        initial content of the StringList; the content will be copied
        from the initial list:

            S = StringList( initial = ["My" , "name" , "is", "John" , "Doe"] )

        If an element in the @initial argument is not a string the
        TypeError exception will be raised.

        If c_ptr argument is different from None, that should refer to
        an already created stringlist instance; this Python will take
        ownership of the underlying object.
        """

        c_ptr = StringList.cNamespace().alloc()
        super(StringList, self).__init__(c_ptr)

        if initial:
            for s in initial:
                if isinstance(s, StringType):
                    self.append(s)
                else:
                    raise TypeError("Item: %s not a string" % s)


    def __eq__(self , other):
        if len(self) == len(other):
            if isinstance( other , StringList):
                return StringList.cNamespace().equal(self, other)
            else:
                equal = True
                for index,s2 in enumerate(other):
                    if self[index] != s2:
                        equal = False
                        break
                return equal
        else:
            return False


    def __setitem__(self, index, value):
        if isinstance(index, IntType):
            length = self.__len__()
            if index < 0:
                # Will only wrap backwards once
                index = len(self) + index

            if index < 0 or index >= length:
                raise IndexError("index must be in range %d <= %d < %d" % (0, index, len(self)))
            if isinstance(value, StringType):
                StringList.cNamespace().iset(self, index, value)
            else:
                raise TypeError("Item: %s not string type" % value)


    def __getitem__(self, index):
        """
        Implements [] read operator on the stringlist.
        
        The __getitem__ method supports negative, i.e. from the right,
        indexing; but not slices.
        """
        if isinstance(index, IntType):
            length = self.__len__()
            if index < 0:
                index += length
            if index < 0 or index >= length:
                raise IndexError("index must be in range %d <= %d < %d" % (0, index, len(self)))
            else:
                return StringList.cNamespace().iget(self, index)
        else:
            raise TypeError("Index should be integer type")

    def __contains__(self, s):
        """
        Implements the 'in' operator.

        The 'in' check is based on string equality.
        """
        return StringList.cNamespace().contains(self, s)

    def contains(self, s):
        """
        Checks if the list contains @s.

        Functionality also available through the 'in' builtin in
        Python.
        """
        return self.__contains__(s)


    def __len__(self):
        """
        The length of the list - used to support builtin len().
        """
        return StringList.cNamespace().get_size(self)


    def __str__(self):
        """
        String representation of list; used when calling print."
        """
        buffer = "["
        length = len(self)
        for i in range(length):
            if i == length - 1:
                buffer += "\'%s\'" % self[i]
            else:
                buffer += "\'%s\'," % self[i]
        buffer += "]"
        return buffer


    def pop(self):
        """
        Will remove the last element from the list and return it. 
        
        Will raise IndexError if list is empty.
        """
        if len(self):
            return StringList.cNamespace().pop(self)
        else:
            raise IndexError("pop() failed - the list is empty")


    def append(self, s):
        """
        Appends a new string @s to list. If the input argument is not a
        string the string representation will be appended.
        """
        if isinstance(s, StringType):
            StringList.cNamespace().append(self, s)
        else:
            StringList.cNamespace().append(self, str(s))
            

    @property
    def strings(self):
        """
        The strings in as a normal Python list of strings.

        The content is copied, so the StringList() instance can safely go
        out of scope after the call has completed. Hmmmm - is that true?
        """
        slist = []
        for s in self:
            slist.append(s)
        return slist

    @property
    def last(self):
        """
        Will return the last element in list. Raise IndexError if empty.
        """
        if len(self) > 0:
            return StringList.cNamespace().last(self)
        else:
            raise IndexError("The list is empty")


    def sort(self, cmp_flag=0):
        """
        Will sort the list inplace.

        The string comparison can be altered with the value of the
        optional cmp_flag parameter:
        
             0 : Normal strcmp() string comparison
             1 : util_strcmp_int() string comparison
             2 : util_strcmp_float() string comparison

        """
        StringList.cNamespace().sort(self, cmp_flag)

    def index(self, value):
        """ @rtype: int """
        assert isinstance(value, str)
        return StringList.cNamespace().find_first(self, value)

    def free(self):
        StringList.cNamespace().free(self)


    def front(self):
        if len(self) > 0:
            return StringList.cNamespace().front(self)
        else:
            raise IndexError

    def back(self):
        if len(self) > 0:
            return StringList.cNamespace().back(self)
        else:
            raise IndexError



CWrapper.registerObjectType("stringlist", StringList)

cwrapper = CWrapper(UTIL_LIB)

StringList.cNamespace().alloc      = cwrapper.prototype("c_void_p stringlist_alloc_new( )")
StringList.cNamespace().free       = cwrapper.prototype("void stringlist_free(stringlist )")
StringList.cNamespace().append     = cwrapper.prototype("void stringlist_append_copy(stringlist , char* )")
StringList.cNamespace().iget       = cwrapper.prototype("char* stringlist_iget(stringlist , int )")
StringList.cNamespace().front      = cwrapper.prototype("char* stringlist_front( stringlist )")
StringList.cNamespace().back       = cwrapper.prototype("char* stringlist_back( stringlist )")
StringList.cNamespace().iget_copy  = cwrapper.prototype("char* stringlist_iget_copy(stringlist, int)")
StringList.cNamespace().iset       = cwrapper.prototype("void  stringlist_iset_copy( stringlist , int , char* )")
StringList.cNamespace().get_size   = cwrapper.prototype("int  stringlist_get_size( stringlist )")
StringList.cNamespace().contains   = cwrapper.prototype("bool stringlist_contains(stringlist , char*)")
StringList.cNamespace().equal      = cwrapper.prototype("bool stringlist_equal(stringlist , stringlist)")
StringList.cNamespace().sort       = cwrapper.prototype("void stringlist_python_sort( stringlist , int)")
StringList.cNamespace().pop        = cwrapper.prototype("char* stringlist_pop(stringlist)")
StringList.cNamespace().last       = cwrapper.prototype("char* stringlist_get_last(stringlist)")
StringList.cNamespace().find_first = cwrapper.prototype("int stringlist_find_first(stringlist, char*)")
