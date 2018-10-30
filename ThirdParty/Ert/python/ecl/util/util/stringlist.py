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
from __future__ import absolute_import, division, print_function, unicode_literals
from six import string_types
from ecl import EclPrototype
from cwrap import BaseCClass


class StringList(BaseCClass):
    TYPE_NAME = "stringlist"

    _alloc      = EclPrototype("void* stringlist_alloc_new( )", bind = False)
    _free       = EclPrototype("void stringlist_free(stringlist )")
    _append     = EclPrototype("void stringlist_append_copy(stringlist , char* )")
    _iget       = EclPrototype("char* stringlist_iget(stringlist , int )")
    _front      = EclPrototype("char* stringlist_front( stringlist )")
    _back       = EclPrototype("char* stringlist_back( stringlist )")
    _iget_copy  = EclPrototype("char* stringlist_iget_copy(stringlist, int)")
    _iset       = EclPrototype("void  stringlist_iset_copy( stringlist , int , char* )")
    _get_size   = EclPrototype("int  stringlist_get_size( stringlist )")
    _contains   = EclPrototype("bool stringlist_contains(stringlist , char*)")
    _equal      = EclPrototype("bool stringlist_equal(stringlist , stringlist)")
    _sort       = EclPrototype("void stringlist_python_sort( stringlist , int)")
    _pop        = EclPrototype("char* stringlist_pop(stringlist)")
    _last       = EclPrototype("char* stringlist_get_last(stringlist)")
    _find_first = EclPrototype("int stringlist_find_first(stringlist, char*)")

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

        c_ptr = self._alloc()
        super(StringList, self).__init__(c_ptr)
        if initial:
            self._append_all(initial)

    def _append_all(self, lst):
        for s in lst:
            if isinstance(s, bytes):
                s.decode('ascii')
            if isinstance(s, string_types):
                self.append(s)
            else:
                raise TypeError('Item is not a string: "%s".' % s)


    def __eq__(self , other):
        if len(self) == len(other):
            if isinstance( other , StringList):
                return self._equal(other)
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
        if isinstance(index, int):
            length = len(self)
            if index < 0:
                # Will only wrap backwards once
                index = len(self) + index

            if index < 0 or index >= length:
                raise IndexError("index must be in range %d <= %d < %d" % (0, index, len(self)))
            if isinstance(value, bytes):
                value = value.decode('ascii')
            if isinstance(value, string_types):
                self._iset(index, value)
            else:
                raise TypeError("Item: %s not string type" % value)


    def __getitem__(self, index):
        """
        Implements [] read operator on the stringlist.

        The __getitem__ method supports negative, i.e. from the right,
        indexing; but not slices.
        """
        if isinstance(index, int):
            length = len(self)
            if index < 0:
                index += length
            if index < 0 or index >= length:
                raise IndexError("index must be in range %d <= %d < %d" % (0, index, len(self)))
            else:
                return self._iget(index)
        else:
            raise TypeError("Index should be integer type")

    def __contains__(self, s):
        """
        Implements the 'in' operator.

        The 'in' check is based on string equality.
        """
        return self._contains(s)


    def __iadd__(self , other):
        if isinstance(other, bytes):
            other.decode('ascii')
        if isinstance(other , string_types):
            raise TypeError("Can not add strings with + - use append()")
        for s in other:
            self.append( s )
        return self


    def __add__(self , other):
        copy = StringList( initial = self )
        copy += other
        return copy


    def __ior__(self , other):
        if isinstance(other, bytes):
            other.decode('ascii')
        if isinstance(other , string_types):
            raise TypeError("Can not | with string.")
        for s in other:
            if not s in self:
                self.append( s )
        return self


    def __or__(self , other):
        copy = StringList( initial = self )
        copy |= other
        return copy



    def contains(self, s):
        """
        Checks if the list contains @s.

        Functionality also available through the 'in' builtin in
        Python.
        """
        return s in self


    def __len__(self):
        """
        The length of the list - used to support builtin len().
        """
        return self._get_size( )


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

    def __repr__(self):
        return 'StringList(size = %d) %s' % (len(self), self._ad_str())

    def empty(self):
        """Returns true if and only if list is empty."""
        return len(self) == 0

    def pop(self):
        """
        Will remove the last element from the list and return it.

        Will raise IndexError if list is empty.
        """
        if not self.empty():
            return self._pop()
        else:
            raise IndexError("List empty.  Cannot call pop().")


    def append(self, s):
        """
        Appends a new string @s to list. If the input argument is not a
        string the string representation will be appended.
        """
        if isinstance(s, bytes):
            s.decode('ascii')
        if isinstance(s, string_types):
            self._append(s)
        else:
            self._append(str(s))


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
        if not self.empty():
            return self._last()
        else:
            raise IndexError("List empty.  No such element last().")


    def sort(self, cmp_flag=0):
        """
        Will sort the list inplace.

        The string comparison can be altered with the value of the
        optional cmp_flag parameter:

             0 : Normal strcmp() string comparison
             1 : util_strcmp_int() string comparison
             2 : util_strcmp_float() string comparison

        """
        self._sort(cmp_flag)

    def index(self, value):
        """ @rtype: int """
        if isinstance(value, bytes):
            value.decode('ascii')
        if isinstance(value, string_types):
            return self._find_first(value)
        raise KeyError('Cannot index by "%s", lst.index() needs a string.' % str(type(value)))

    def free(self):
        self._free()


    def front(self):
        if not self.empty():
            return self._front()
        else:
            raise IndexError('List empty.  No such element front().')

    def back(self):
        if not self.empty():
            return self._back()
        else:
            raise IndexError('List empty.  No such element back().')
