#  Copyright (C) 2011  Equinor ASA, Norway.
#
#  The file 'ecl_kw.py' is part of ERT - Ensemble based Reservoir Tool.
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
Support for working with one keyword from ECLIPSE file.

ECLIPSE files in "restart format" are organized in keywords consisting
of a simple header and blocks of data. A keyword typically looks like:

  'SWAT    '  10000  'REAL'
  0.05  0.08  0.08  0.10
  0.11  0.11  0.10  0.09
  ....

I.e. it starts with of header consisting of a 8 characters name, a
length and a datatype, immediately followed by the actual
data.

Altough the term "restart format" is used to describe the format, this
particular format is not limited to restart files; it is (at least)
used in INIT, EGRID, GRID, Snnn, UNSMRY, SMSPEC, UNRST, Xnnnn and RFT
files. This module also has (some) support for working with GRDECL
'formatted' files.

The ecl_kw.py implementation wraps the ecl_kw.c implementation from
the libecl library.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import ctypes
import warnings
import numpy

from cwrap import CFILE, BaseCClass

from ecl import EclPrototype
from ecl.util.util import monkey_the_camel
from ecl import EclDataType
from ecl import EclTypeEnum, EclUtil

from .fortio import FortIO

def dump_type_deprecation_warning():
    warnings.warn("EclTypeEnum is deprecated. " +
        "You should instead provide an EclDataType",
        DeprecationWarning)

def constant_size_data_type(ecl_type):
    return (ecl_type in [
                        EclTypeEnum.ECL_CHAR_TYPE,
                        EclTypeEnum.ECL_FLOAT_TYPE,
                        EclTypeEnum.ECL_DOUBLE_TYPE,
                        EclTypeEnum.ECL_INT_TYPE,
                        EclTypeEnum.ECL_BOOL_TYPE,
                        EclTypeEnum.ECL_MESS_TYPE
                        ])

def warn_and_cast_data_type(data_type):
    if isinstance(data_type, EclDataType):
        return data_type
    if isinstance(data_type, EclTypeEnum):
        if not constant_size_data_type(data_type):
            raise ValueError("Cannot cast EclTypeEnum (%d) to EclDataType due "
                    "to non-constant size. Please provide an EclDataType instead.")

        dump_type_deprecation_warning()
        return EclDataType(data_type)

class EclKW(BaseCClass):
    """
    The EclKW class contains the information from one ECLIPSE keyword.

    The ecl_kw type is the lowest level type in the libecl C library,
    and all the other datatypes like e.g. ecl_grid and ecl_sum are
    based on collections of ecl_kw instances, and interpreting the
    content of the ecl_kw keywords.

    Many of the special __xxx___() functions have been implemented, so
    that the EclKW class supports both numerical operations and also
    [] based lookup. Many of the methods accept an optional @mask
    argument; this should be a EclRegion instance which can be used to
    limit the operation to a part of the EclKW.
    """

    int_kw_set = set(["PVTNUM", "FIPNUM", "EQLNUM", "FLUXNUM", "MULTNUM", "ACTNUM", "SPECGRID", "REGIONS"])

    TYPE_NAME          = "ecl_kw"
    _alloc_new         = EclPrototype("void* ecl_kw_alloc_python(char*, int, ecl_data_type)", bind = False)
    _fread_alloc       = EclPrototype("ecl_kw_obj ecl_kw_fread_alloc(fortio)", bind = False)
    _load_grdecl       = EclPrototype("ecl_kw_obj ecl_kw_fscanf_alloc_grdecl_dynamic_python(FILE, char*, bool, ecl_data_type)", bind = False)
    _fseek_grdecl      = EclPrototype("bool     ecl_kw_grdecl_fseek_kw(char*, bool, FILE)", bind = False)

    _sub_copy          = EclPrototype("ecl_kw_obj ecl_kw_alloc_sub_copy(ecl_kw, char*, int, int)")
    _copyc             = EclPrototype("ecl_kw_obj ecl_kw_alloc_copy(ecl_kw)")
    _slice_copyc       = EclPrototype("ecl_kw_obj ecl_kw_alloc_slice_copy(ecl_kw, int, int, int)")
    _global_copy       = EclPrototype("ecl_kw_obj ecl_kw_alloc_global_copy(ecl_kw, ecl_kw)")
    _fprintf_grdecl    = EclPrototype("void     ecl_kw_fprintf_grdecl(ecl_kw, FILE)")
    _fprintf_data      = EclPrototype("void     ecl_kw_fprintf_data(ecl_kw, char*, FILE)")

    _get_size          = EclPrototype("int      ecl_kw_get_size(ecl_kw)")
    _get_fortio_size   = EclPrototype("size_t   ecl_kw_fortio_size(ecl_kw)")
    _get_type          = EclPrototype("ecl_type_enum ecl_kw_get_type(ecl_kw)")
    _iget_char_ptr     = EclPrototype("char*    ecl_kw_iget_char_ptr(ecl_kw, int)")
    _iset_char_ptr     = EclPrototype("void     ecl_kw_iset_char_ptr(ecl_kw, int, char*)")
    _iget_string_ptr   = EclPrototype("char*    ecl_kw_iget_string_ptr(ecl_kw, int)")
    _iset_string_ptr   = EclPrototype("void     ecl_kw_iset_string_ptr(ecl_kw, int, char*)")
    _iget_bool         = EclPrototype("bool     ecl_kw_iget_bool(ecl_kw, int)")
    _iset_bool         = EclPrototype("bool     ecl_kw_iset_bool(ecl_kw, int, bool)")
    _iget_int          = EclPrototype("int      ecl_kw_iget_int(ecl_kw, int)")
    _iget_double       = EclPrototype("double   ecl_kw_iget_double(ecl_kw, int)")
    _iget_float        = EclPrototype("float    ecl_kw_iget_float(ecl_kw, int)")
    _float_ptr         = EclPrototype("float*   ecl_kw_get_float_ptr(ecl_kw)")
    _int_ptr           = EclPrototype("int*     ecl_kw_get_int_ptr(ecl_kw)")
    _double_ptr        = EclPrototype("double*  ecl_kw_get_double_ptr(ecl_kw)")
    _free              = EclPrototype("void     ecl_kw_free(ecl_kw)")
    _fwrite            = EclPrototype("void     ecl_kw_fwrite(ecl_kw, fortio)")
    _get_header        = EclPrototype("char*    ecl_kw_get_header (ecl_kw)")
    _set_header        = EclPrototype("void     ecl_kw_set_header_name (ecl_kw, char*)")
    _get_data_type     = EclPrototype("ecl_data_type_obj ecl_kw_get_data_type_python(ecl_kw)");

    _int_sum           = EclPrototype("int      ecl_kw_element_sum_int(ecl_kw)")
    _float_sum         = EclPrototype("double   ecl_kw_element_sum_float(ecl_kw)")
    _iadd_squared      = EclPrototype("void     ecl_kw_inplace_add_squared(ecl_kw, ecl_kw)")
    _isqrt             = EclPrototype("void     ecl_kw_inplace_sqrt(ecl_kw)")
    _iadd              = EclPrototype("void     ecl_kw_inplace_add(ecl_kw, ecl_kw)")
    _imul              = EclPrototype("void     ecl_kw_inplace_mul(ecl_kw, ecl_kw)")
    _idiv              = EclPrototype("void     ecl_kw_inplace_div(ecl_kw, ecl_kw)")
    _isub              = EclPrototype("void     ecl_kw_inplace_sub(ecl_kw, ecl_kw)")
    _iabs              = EclPrototype("void     ecl_kw_inplace_abs(ecl_kw)")
    _equal             = EclPrototype("bool     ecl_kw_equal(ecl_kw, ecl_kw)")
    _equal_numeric     = EclPrototype("bool     ecl_kw_numeric_equal(ecl_kw, ecl_kw, double, double)")

    _assert_binary     = EclPrototype("bool     ecl_kw_size_and_numeric_type_equal(ecl_kw, ecl_kw)")
    _scale_int         = EclPrototype("void     ecl_kw_scale_int(ecl_kw, int)")
    _scale_float       = EclPrototype("void     ecl_kw_scale_float_or_double(ecl_kw, double)")
    _shift_int         = EclPrototype("void     ecl_kw_shift_int(ecl_kw, int)")
    _shift_float       = EclPrototype("void     ecl_kw_shift_float_or_double(ecl_kw, double)")
    _copy_data         = EclPrototype("void     ecl_kw_memcpy_data(ecl_kw, ecl_kw)")
    _set_int           = EclPrototype("void     ecl_kw_scalar_set_int(ecl_kw, int)")
    _set_float         = EclPrototype("void     ecl_kw_scalar_set_float_or_double(ecl_kw, double)")

    _max_min_int       = EclPrototype("void     ecl_kw_max_min_int(ecl_kw, int*, int*)")
    _max_min_float     = EclPrototype("void     ecl_kw_max_min_float(ecl_kw, float*, float*)")
    _max_min_double    = EclPrototype("void     ecl_kw_max_min_double(ecl_kw, double*, double*)")
    _fix_uninitialized = EclPrototype("void     ecl_kw_fix_uninitialized(ecl_kw,int, int, int, int*)")
    _create_actnum     = EclPrototype("ecl_kw_obj ecl_kw_alloc_actnum(ecl_kw, float)")
    _first_different   = EclPrototype("int      ecl_kw_first_different(ecl_kw, ecl_kw, int, double, double)")
    _resize            = EclPrototype("void     ecl_kw_resize(ecl_kw, int)")
    _safe_div          = EclPrototype("bool     ecl_kw_inplace_safe_div(ecl_kw,ecl_kw)")


    @classmethod
    def createCReference(cls, c_ptr, parent=None):
        ecl_kw = super(EclKW, cls).createCReference(c_ptr, parent=parent)
        if ecl_kw is None:
            raise ValueError("Failed to create EclKW instance")

        ecl_kw.__private_init()
        return ecl_kw


    @classmethod
    def createPythonObject(cls, c_ptr):
        ecl_kw = super(EclKW, cls).createPythonObject(c_ptr)
        if ecl_kw is None:
            raise ValueError("Failed to create EclKW instance")

        ecl_kw.__private_init()
        return ecl_kw



    @classmethod
    def add_int_kw(cls, kw):
        """Will add keyword @kw to the standard set of integer keywords."""
        cls.int_kw_set.add(kw)

    @classmethod
    def del_int_kw(cls, kw):
        """Will remove keyword @kw from the standard set of integer keywords."""
        cls.int_kw_set.discard(kw)

    @classmethod
    def int_keywords(cls):
        """Will return the current set of integer keywords."""
        return cls.int_kw_set


    def slice_copy(self, slice_range):
        (start, stop, step) = slice_range.indices(len(self))
        if stop > start:
            return self._slice_copyc(start, stop, step)
        else:
            return None


    def copy(self):
        """
        Will create a deep copy of the current kw instance.
        """
        return self._copyc()




    @classmethod
    def read_grdecl(cls, fileH, kw, strict=True, ecl_type=None):
        """
        Function to load an EclKW instance from a grdecl formatted filehandle.

        This constructor can be used to load an EclKW instance from a
        grdecl formatted file; the input files for petrophysical
        properties are typically given as grdecl files.

        The @file argument should be a Python filehandle to an open
        file. The @kw argument should be the keyword header you are
        searching for, e.g. "PORO" or "PVTNUM"[1], the method will
        then search forward through the file to look for this @kw. If
        the keyword can not be found the method will return None. The
        searching will start from the current position in the file; so
        if you want to reposition the file pointer you should use the
        seek() method of the file object first.

        Observe that there is a strict 8 character limit on @kw -
        altough you could in principle use an arbitrary external
        program to create grdecl files with more than 8 character
        length headers, this implementation will refuse to even try
        loading them. In that case you will have to rename the
        keywords in your file - sorry. A TypeError exception
        will be raised if @kw has more than 8 characters.

        The implementation in ert can read integer and float type
        keywords from grdecl files; however the grdecl files have no
        datatype header, and it is impossible to determine the type
        reliably by inspection. Hence the type must be known when
        reading the file. The algorithm for specifying type, in order
        of presedence, is as follows:

        1. The optional argument @ecl_type can be used to specify
           the type:

           special_int_kw = EclKW.read_grdecl(fileH, 'INTKW', ecl_type=ECL_INT)

           If ecl_type is different from ECL_INT or
           ECL_FLOAT a TypeError exception will be raised.

           If ecl_type == None (the default), the method will continue
           to point 2. or 3. to determine the correct type.


        2. If the keyword is included in the set built in set
           'int_kw_set' the type will be ECL_INT_TYPE.

           pvtnum_kw = EclKW.read_grdecl(fileH, 'PVTNUM')

           Observe that (currently) no case conversions take place
           when checking the 'int_kw_set'. The current built in set is
           accesible through the int_kw property.


        3. Otherwise the default is float, i.e. ECL_FLOAT.

           EclKw reads grdecl with EclDataType
           poro_kw = EclKW.read_grdecl(fileH, 'PORO')


        Observe that since the grdecl files are quite weakly
        structured it is difficult to verify the integrity of the
        files, malformed input might therefore pass unnoticed before
        things blow up at a later stage.

        [1]: It is possible, but not recommended, to pass in None for
        @kw, in which case the method will load the first keyword
        it finds in the file.
        """

        cfile  = CFILE(fileH)
        if kw:
            if len(kw) > 8:
                raise TypeError("Sorry keyword:%s is too long, must be eight characters or less." % kw)

        if ecl_type is None:
            if cls.int_kw_set.__contains__(kw):
                ecl_type = EclDataType.ECL_INT
            else:
                ecl_type = EclDataType.ECL_FLOAT

        ecl_type = warn_and_cast_data_type(ecl_type)

        if not isinstance(ecl_type, EclDataType):
            raise TypeError("Expected EclDataType, was: %s" % type(ecl_type))

        if not ecl_type in [EclDataType.ECL_FLOAT, EclDataType.ECL_INT]:
            raise ValueError("The type:%s is invalid when loading keyword:%s" % (ecl_type.type_name, kw))

        return cls._load_grdecl(cfile, kw, strict, ecl_type)


    @classmethod
    def fseek_grdecl(cls, fileH, kw, rewind=False):
        """
        Will search through the open file and look for string @kw.

        If the search succeeds the function will return and the file
        pointer will be positioned at the start of the kw, if the
        search fails the function will return false and the file
        pointer will be repositioned at the position it had prior to
        the call.

        Only @kw instances which are found at the beginning of a line
        (with optional leading space characters) are considered,
        i.e. searching for the string PERMX in the cases below will
        fail:

           -- PERMX
           EQUIL   PERMX /


        The function will start searching from the current position in
        the file and forwards, if the optional argument @rewind is
        true the function rewind to the beginning of the file and
        search from there after the initial search.
        """
        cfile = CFILE(fileH)
        return cls._fseek_grdecl(kw, rewind, cfile)


    @classmethod
    def fread(cls, fortio):
        """
        Will read a new EclKW instance from the open FortIO file.
        """
        return cls._fread_alloc(fortio)


    def free(self):
        self._free()

    def __repr__(self):
        si = len(self)
        nm = self.getName()
        mm = 'type=%s' % str(self.data_type)
        if self.isNumeric():
            mi, ma = self.getMinMax()
            mm = 'min=%.2f, max=%.2f' % (mi,ma)
        ad = self._ad_str()
        fmt = 'EclKW(size=%d, name="%s", %s) %s'
        return fmt % (si,nm,mm,ad)

    def __init__(self, name, size, data_type):
        """Creates a brand new EclKW instance.

        This method will create a grand spanking new EclKW
        instance. The instance will get name @name @size elements and
        datatype @data_type. Using this method you could create a SOIL
        keyword with:

           soil_kw = EclKW("SOIL", 10000, ECL_FLOAT_TYPE)

        """
        if len(name) > 8:
            raise ValueError("Sorry - maximum eight characters in keyword name")

        data_type = warn_and_cast_data_type(data_type)

        if not isinstance(data_type, EclDataType):
            raise TypeError("Expected an EclDataType, received: %s" %
                    type(data_type))

        c_ptr = self._alloc_new(name, size, data_type)
        super(EclKW, self).__init__(c_ptr)
        self.__private_init()



    def __private_init(self):
        self.data_ptr   = None

        if self.data_type.is_int():
            self.data_ptr = self._int_ptr()
            self.dtype    = numpy.int32
            self.str_fmt  = "%8d"
        elif self.data_type.is_float():
            self.data_ptr = self._float_ptr()
            self.dtype    = numpy.float32
            self.str_fmt  = "%13.4f"
        elif self.data_type.is_double():
            self.data_ptr = self._double_ptr()
            self.dtype    = numpy.float64
            self.str_fmt  = "%13.4f"
        else:
            # Iteration not supported for CHAR / BOOL
            self.data_ptr = None
            self.dtype    = None
            if self.data_type.is_char():
                self.str_fmt  = "%8s"
            elif self.data_type.is_bool():
                self.str_fmt  = "%d"
            elif self.data_type.is_mess():
                self.str_fmt = "%s"  #"Message type"
            elif self.data_type.is_string():
                self.str_fmt = "%" + str(self.data_type.element_size) + "s"
            else:
                raise ValueError("Unknown EclDataType (%s)!" % self.data_type.type_name)

    def sub_copy(self, offset, count, new_header=None):
        """
        Will create a new block copy of the src keyword.

        If @new_header == None the copy will get the same 'name' as
        the src, otherwise the keyword will get the @new_header as
        header.

        The copy will start at @block of the src keyword and copy
        @count elements; a negative value of @count is interpreted as
        'the rest of the elements'

           new1 = src.sub_copy(0, 10, new_header="NEW1")
           new2 = src.sub_copy(10, -1, new_header="NEW2")

        If the count or index arguments are in some way invalid the
        method will raise IndexError.
        """
        if offset < 0 or offset >= len(self):
            raise IndexError("Offset:%d invalid - valid range:[0,%d)" % (offset, len(self)))

        if offset + count > len(self):
            raise IndexError("Invalid value of (offset + count):%d" % (offset + count))

        return self._sub_copy(new_header, offset, count)


    def is_numeric(self):
        """
        Will check if the keyword contains numeric data, i.e int, float or double.
        """
        return self.data_type.is_numeric()

    def ecl_kw_instance(self):
        return True



    def __len__(self):
        """
        Returns the number of elements. Implements len()
        """
        return self._get_size()


    def __deep_copy__(self, memo):
        """
        Python special routine used to perform deep copy.
        """
        ecl_kw = self.copy()
        return ecl_kw


    def __getitem__(self, index):
        """
        Function to support index based lookup: y = kw[index]
        """
        if isinstance(index,int):
            length = len(self)
            if index < 0:
                # We allow one level of negative indexing
                index += len(self)

            if index < 0 or index >= length:
                raise IndexError
            else:
                if self.data_ptr:
                    return self.data_ptr[ index ]
                else:
                    if self.data_type.is_bool():
                        return self._iget_bool(index)
                    elif self.data_type.is_char():
                        return self._iget_char_ptr(index)
                    elif self.data_type.is_string():
                        return self._iget_string_ptr(index)
                    else:
                        raise TypeError("Internal implementation error ...")
        elif isinstance(index, slice):
            return self.slice_copy(index)
        else:
            raise TypeError("Index should be integer type")


    def __setitem__(self, index,value):
        """
        Function to support index based assignment: kw[index] = value
        """
        if isinstance(index, int):
            length = len(self)
            if index < 0:
                # Will only wrap backwards once
                index = len(self) + index

            if index < 0 or index >= length:
                raise IndexError
            else:
                if self.data_ptr:
                    self.data_ptr[ index ] = value
                else:
                    if self.data_type.is_bool():
                        self._iset_bool(index, value)
                    elif self.data_type.is_char():
                        return self._iset_char_ptr(index, value)
                    elif self.data_type.is_string():
                        return self._iset_string_ptr(index, value)
                    else:
                        raise SystemError("Internal implementation error ...")
        elif isinstance(index, slice):
            (start, stop, step) = index.indices(len(self))
            index = start
            while index < stop:
                self[index] = value
                index += step
        else:
            raise TypeError("Index should be integer type")


    #################################################################


    def __IMUL__(self, factor, mul=True):
        if self.isNumeric():
            if hasattr(factor, "ecl_kw_instance"):
                if self.assert_binary(factor):
                    if mul:
                        self._imul(factor)
                    else:
                        self._idiv(factor)
                else:
                    raise TypeError("Type mismatch")
            else:
                if not mul:
                    factor = 1.0 / factor

                if self.data_type.is_int():
                    if isinstance(factor, int):
                        self._scale_int(factor)
                    else:
                        raise TypeError("Type mismatch")
                else:
                    if isinstance(factor, int) or isinstance(factor, float):
                        self._scale_float(factor)
                    else:
                        raise TypeError("Only muliplication with scalar supported")
        else:
            raise TypeError("Not numeric type")

        return self


    def __IADD__(self, delta, add=True):
        if self.isNumeric():
            if type(self) == type(delta):
                if self.assert_binary(delta):
                    if add:
                        self._iadd(delta)
                    else:
                        self._isub(delta)
                else:
                    raise TypeError("Type / size mismatch")
            else:
                if add:
                    sign = 1
                else:
                    sign = -1

                if self.data_type.is_int():
                    if isinstance(delta, int):
                        self._shift_int(delta * sign)
                    else:
                        raise TypeError("Type mismatch")
                else:
                    if isinstance(delta, int) or isinstance(delta, float):
                        self._shift_float(delta * sign) # Will call the _float() or _double() function in the C layer.
                    else:
                        raise TypeError("Type mismatch")
        else:
            raise TypeError("Type / size mismatch")

        return self

    def __iadd__(self, delta):
        return self.__IADD__(delta, True)

    def __isub__(self, delta):
        return self.__IADD__(delta, False)

    def __imul__(self, delta):
        return self.__IMUL__(delta, True)

    def __idiv__(self, delta):
        return self.__IMUL__(delta, False)


    #################################################################

    def __abs__(self):
        if self.isNumeric():
            copy = self.copy()
            copy._iabs()
            return copy
        else:
            raise TypeError("The __abs__() function is only implemented for numeric types")



    def __add__(self, delta):
        copy = self.copy()
        copy += delta
        return copy

    def __radd__(self, delta):
        return self.__add__(delta)

    def __sub__(self, delta):
        copy  = self.copy()
        copy -= delta
        return copy

    def __rsub__(self, delta):
        return self.__sub__(delta) * -1

    def __mul__(self, factor):
        copy  = self.copy()
        copy *= factor
        return copy

    def __rmul__(self, factor):
        return self.__mul__(factor)

    def __div__(self, factor):
        copy = self.copy()
        copy /= factor
        return copy

    # No __rdiv__()

    def add_squared(self, other):
       if not self.is_numeric():
           raise TypeError("Can only be called on numeric types")

       if not self.assert_binary(other):
           raise ValueError("Invalid argument to method add_squared")

       self._iadd_squared(other)

    def isqrt(self):
        if not self.is_numeric():
            raise TypeError("Can only be called on numeric types")

        self._isqrt()


    def sum(self, mask = None, force_active = False):
        """
        Will calculate the sum of all the elements in the keyword.

        String: Raise ValueError exception.
        Bool:   The number of true values
        """
        if mask is None:
            if self.data_type.is_int():
                return self._int_sum()
            elif self.data_type.is_float():
                return self._float_sum()
            elif self.data_type.is_double():
                return self._float_sum()
            elif self.data_type.is_bool():
                sum = 0
                for elm in self:
                    if elm:
                        sum += 1
                return sum
            else:
                raise ValueError('The keyword "%s" is of string type - sum is not implemented' % self.getName())

        return mask.sum_kw(self, force_active)



    def assert_binary(self, other):
        """
        Utility function to assert that keywords @self and @other can
        be combined.
        """
        return self._assert_binary(other)

    #################################################################

    def assign(self, value, mask=None, force_active=False):
        """
        Assign a value to current kw instance.

        This method is used to assign value(s) to the current EclKW
        instance. The @value parameter can either be a scalar, or
        another EclKW instance. To set all elements of a keyword to
        1.0:

            kw.assign(1.0)

        The numerical type of @value must be compatible with the
        current keyword. The optional @mask argument should be an
        EclRegion instance which can be used to limit the assignment
        to only parts of the EclKW. In the example below we select all
        the elements with PORO below 0.10, and then assign EQLNUM
        value 88 to those cells:

            grid = ecl.EclGrid("ECLIPSE.EGRID")
            reg  = ecl.EclRegion(grid, false)
            init = ecl.EclFile("ECLIPSE.INIT")

            poro = init["PORO"][0]
            eqlnum = init["EQLNUM"][0]
            reg.select_below(poro, 0.10)

            eqlnum.assign(88, mask = reg)

        The EclRegion instance has two equivalent sets of selected
        indices; one consisting of active indices and one consisting
        of global indices. By default the assign() method will select
        the global indices if the keyword has nx*ny*nz elements and
        the active indices if the kw has nactive elements. By setting
        the optional argument @force_active to true, you can force the
        method to only modify the active indices, even though the
        keyword has nx*ny*nz elements; if the keyword has nactive
        elements the @force_active flag is not considered.
        """
        if self.isNumeric():
            if type(value) == type(self):
                if mask is not None:
                    mask.copy_kw(self, value, force_active)
                else:
                    if self.assert_binary(value):
                        self._copy_data(value)
                    else:
                        raise TypeError("Type / size mismatch")
            else:
                if mask is not None:
                    mask.set_kw(self, value, force_active)
                else:
                    if self.data_type.is_int():
                        if isinstance(value, int):
                            self._set_int(value)
                        else:
                            raise TypeError("Type mismatch")
                    else:
                        if isinstance(value, int) or isinstance(value, float):
                            self._set_float(value)
                        else:
                            raise TypeError("Only muliplication with scalar supported")


    def add(self, other, mask=None, force_active=False):
        """
        See method assign() for documentation of optional arguments
        @mask and @force_active.
        """
        if mask is not None:
            mask.iadd_kw(self, other, force_active)
        else:
            return self.__iadd__(other)

    def sub(self, other, mask=None, force_active=False):
        """
        See method assign() for documentation of optional arguments
        @mask and @force_active.
        """
        if mask is not None:
            mask.isub_kw(self, other, force_active)
        else:
            return self.__isub__(other)

    def mul(self, other, mask=None, force_active=False):
        """
        See method assign() for documentation of optional arguments
        @mask and @force_active.
        """
        if mask is not None:
            mask.imul_kw(self, other, force_active)
        else:
            return self.__imul__(other)

    def div(self, other, mask=None, force_active=False):
        """
        See method assign() for documentation of optional arguments
        @mask and @force_active.
        """
        if mask is not None:
            mask.idiv_kw(self, other, force_active)
        else:
            return self.__idiv__(other)

    def apply(self, func, arg=None, mask=None, force_active=False):
        """
        Will apply the function @func on the keyword - inplace.

        The function @func should take a scalar value from the ecl_kw
        vector as input, and return a scalar value of the same type;
        optionally you can supply a second argument with the @arg
        attribute:

          def cutoff(x, limit):
              if x > limit:
                 return x
              else:
                 return 0


          kw.apply(math.sin)
          kw.apply(cutoff, arg=0.10)


        See method assign() for documentation of optional arguments
        @mask and @force_active.
        """
        if mask is not None:
            active_list = mask.kw_index_list(self, force_active)
            if arg:
                for index in active_list:
                    self.data_ptr[index] = func(self.data_ptr[index], arg)
            else:
                for index in active_list:
                    self.data_ptr[index] = func(self.data_ptr[index])
        else:
            if arg:
                for i in range(len(self)):
                    self.data_ptr[i] = func(self.data_ptr[i], arg)
            else:
                for i in range(len(self)):
                    self.data_ptr[i] = func(self.data_ptr[i])


    def equal(self, other):
        """
        Will check if the two keywords are (exactly) equal.

        The check is based on the content of the keywords, and not
        pointer comparison.
        """
        if isinstance(other, EclKW):
            return self._equal(other)
        else:
            raise TypeError("Can only compare with another EclKW")


    def __eq__(self, other):
        return self.equal(other)

    def __hash__(self):
        return hash(self._get_header())


    def equal_numeric(self, other, epsilon=1e-6, abs_epsilon=None, rel_epsilon=None):
        """Will check if two numerical keywords are ~nearly equal.


        If the keywords are of type integer, the comparison is
        absolute.

        If you pass in xxx_epsilon <= 0 the xxx_epsilon will be
        ignored in the test.

        """
        if isinstance(other, EclKW):
            if abs_epsilon is None:
                abs_epsilon = epsilon

            if rel_epsilon is None:
                rel_epsilon = epsilon

            return self._equal_numeric(other, abs_epsilon, rel_epsilon)
        else:
            raise TypeError("Can only compare with another EclKW")


    #################################################################

    def deep_copy(self):
        ecl_kw = self.__deep_copy__({})
        return ecl_kw

    def fort_io_size(self):
        """
        The number of bytes this keyword would occupy in a BINARY file.
        """
        return self._get_fortio_size()

    def set_name(self, name):
        if len(name) > 8:
            raise ValueError("Sorry: the name property must be max 8 characters long :-(")
        self._set_header(name)

    @property
    def name(self):
        n = self._get_header()
        return str(n) if n else ''

    @name.setter
    def name(self, value):
        self.set_name(value)

    def get_name(self):
        return self.name

    def resize(self, new_size):
        """
        Will set the new size of the kw to @new_size.
        """
        if new_size >= 0:
            self._resize(int(new_size))

        # Iteration is based on a pointer to the underlying storage,
        # that will generally by reset by the resize() call; i.e. we
        # need to call the __private_init() method again.
        self.__private_init()


    def get_min_max(self):
        """
        Will return a touple (min,max) for numerical types.

        Will raise TypeError exception if the keyword is not of
        numerical type.
        """
        if self.data_type.is_float():
            min_ = ctypes.c_float()
            max_ = ctypes.c_float()
            self._max_min_float(ctypes.byref(max_), ctypes.byref(min_))
        elif self.data_type.is_double():
            min_ = ctypes.c_double()
            max_ = ctypes.c_double()
            self._max_min_double(ctypes.byref(max_), ctypes.byref(min_))
        elif self.data_type.is_int():
            min_ = ctypes.c_int()
            max_ = ctypes.c_int()
            self._max_min_int(ctypes.byref(max_), ctypes.byref(min_))
        else:
            raise TypeError("min_max property not defined for keywords of type: %s" % self.type)
        return (min_.value, max_.value)


    def get_max(self):
        mm = self.getMinMax()
        return mm[1]


    def get_min(self):
        mm = self.getMinMax()
        return mm[0]

    @property
    def type(self):
        warnings.warn("ecl_kw.type is deprecated, use .data_type",
            DeprecationWarning)
        return self._get_type()

    @property
    def data_type(self):
        return self._get_data_type()

    @property
    def type_name(self):
        return self.data_type.type_name

    def type_name(self):
        return self.data_type.type_name

    def get_ecl_type(self):
        warnings.warn("EclTypeEnum is deprecated. " +
            "You should instead provide an EclDataType",
            DeprecationWarning)

        return self._get_type()




    @property
    def header(self):
        return (self.getName(), len(self), self.typeName())

    @property
    def array(self):
        a = self.data_ptr
        if not a == None:
            a.size        = len(self)
            a.__parent__  = self  # Inhibit GC
        return a


    def str_data(self, width, index1, index2, fmt):
        """
        Helper function for str() method.
        """
        data = []
        s = ""
        for index in range(index1, index2):
            data.append(self[index])
        for index in range(len(data)):
            s += fmt % data[ index ]
            if index % width == (width - 1):
                s+= "\n"
        return s


    def str(self, width=5, max_lines=10, fmt=None):
        """
        Return string representation of kw for pretty printing.

        The function will return a string consisting of a header, and
        then a chunk of data. The data will be formatted in @width
        columns, and a maximum of @max_lines lines. If @max_lines is
        not sufficient the first elements in the kewyord are
        represented, a .... continuation line and then the last part
        of the keyword. If @max_lines is None all of the vector will
        be printed, irrespective of how long it is.

        If a value is given for @fmt that is used as format string for
        each element, otherwise a type-specific default format is
        used. If given the @fmt string should contain spacing between
        the elements. The implementation of the builtin method
        __str__() is based on this method.
        """
        s = "%-8s %8d %-4s\n" % (self.getName(), len(self), self.typeName())
        lines = len(self) // width
        if not fmt:
            fmt = self.str_fmt + " "

        if max_lines is None or lines <= max_lines:
            s += self.str_data(width, 0, len(self), fmt)
        else:
            s1 = width * max_lines // 2
            s += self.str_data(width , 0, s1, fmt)
            s += "   ....   \n"
            s += self.str_data(width , len(self) - s1, len(self), fmt)

        return s

    def __str__(self):
        """
        Return string representation - see method str().
        """
        return self.str(width=5, max_lines=10)


    def numpy_view(self):
        """Will return a numpy view to the underlying data.

        The data in this numpy array is *shared* with the EclKW
        instance, meaning that updates in one will be reflected in the
        other.
        """

        if self.dtype is numpy.float64:
            ct = ctypes.c_double
        elif self.dtype is numpy.float32:
            ct = ctypes.c_float
        elif self.dtype is numpy.int32:
            ct = ctypes.c_int
        else:
            raise ValueError("Invalid type - numpy array only valid for int/float/double")

        ap = ctypes.cast(self.data_ptr, ctypes.POINTER(ct * len(self)))
        return numpy.frombuffer(ap.contents, dtype=self.dtype)


    def numpy_copy(self):
        """Will return a numpy array which contains a copy of the EclKW data.

        The numpy array has a separate copy of the data, so that
        changes to either the numpy array or the EclKW will *not* be
        reflected in the other datastructure. This is in contrast to
        the EclKW.numpyView() method where the underlying data is
        shared.
        """
        view = self.numpyView()
        return numpy.copy(view)

    def fwrite(self, fortio):
        self._fwrite(fortio)

    def write_grdecl(self, file):
        """
        Will write keyword in GRDECL format.

        This method will write the current keyword in GRDECL format,
        the @file argument must be a Python file handle to an already
        opened file. In the example below we load the porosity from an
        existing GRDECL file, set all poro values below 0.05 to 0.00
        and write back an updated GRDECL file.

            poro = ecl.EclKW.read_grdecl(open("poro1.grdecl", "r"), "PORO")
            grid = ecl.EclGrid("ECLIPSE.EGRID")
            reg  = ecl.EclRegion(grid, False)

            reg.select_below(poro, 0.05)
            poro.assign(0.0, mask=reg)

            fileH = open("poro2.grdecl", "w")
            poro.write_grdecl(fileH)
            fileH.close()

        """
        cfile = CFILE(file)
        self._fprintf_grdecl(cfile)



    def fprintf_data(self, file, fmt=None):
        """
        Will print the keyword data formatted to file.

        The @file argument should be a python file handle to a file
        opened for writing. The @fmt argument is used as fprintf()
        format specifier, observe that the format specifier should
        include a separation character between the elements. If no
        @fmt argument is supplied the default str_fmt specifier is
        used for every element, separated by a newline.

        In the case of boolean data the function will print o and 1
        for False and True respectively. For string data the function
        will print the data as 8 characters long string with blank
        padding on the right.
        """
        if fmt is None:
            fmt = self.str_fmt + "\n"
        cfile = CFILE(file)
        self._fprintf_data(fmt, cfile)

    def create_actnum(self, porv_limit = 0):
        """Will create ACTNUM keyword from PORV keyword.

        This quite specialized method will create an ACTNUM keyword based on
        interpreting the current keyword as a PORV keyword. The method will
        raise an exception if the current keyword is not ("PORV", FLOAT). The
        code implemented in C for speed is essentially:

             actnum = [ 1 if x > porv_limit else 0 for x in self ]

        """
        if not self.data_type.is_float():
            raise TypeError("The PORV keyword must be of type FLOAT")

        if not self.get_name() == "PORV":
            raise ValueError("Input argument must be PORV keyword")

        return self._create_actnum(porv_limit)


    def fix_uninitialized(self, grid):
        """
        Special case function for region code.
        """
        dims = grid.getDims()
        actnum = grid.exportACTNUM()
        self._fix_uninitialized(dims[0], dims[1], dims[2], actnum.getDataPtr())


    def get_data_ptr(self):
        if self.data_type.is_int():
            return self._int_ptr()
        elif self.data_type.is_float():
            return self._float_ptr()
        elif self.data_type.is_double():
            return self._double_ptr()
        else:
            raise ValueError("Only numeric types can export data pointer")


    def first_different(self, other, offset=0, epsilon=0, abs_epsilon=None, rel_epsilon=None):
        if len(self) != len(other):
            raise ValueError("Keywords must have equal size")

        if offset >= len(self):
            raise IndexError("Offset:%d invalid - size:%d" % (offset, len(self)))

        if self.data_type!= other.data_type:
            raise TypeError("The two keywords have different type")

        if abs_epsilon is None:
            abs_epsilon = epsilon

        if rel_epsilon is None:
            rel_epsilon = epsilon

        return self._first_different(other, offset, abs_epsilon, rel_epsilon)




    def scatter_copy(self, actnum):
        if not isinstance(actnum, EclKW):
            raise TypeError("The actnum argument must be of type EclKW")

        return self._global_copy(actnum)


    def safe_div(self, divisor):
        if not len(self) == len(divisor):
            raise ValueError("Length mismatch between %s and %s" % (self.name, divisor.name))

        if not self.is_numeric():
            raise TypeError("The self keyword must be of numeric type")

        if not divisor.is_numeric():
            raise TypeError("Must divide by numeric keyword")

        ok = self._safe_div( divisor )
        if not ok:
            raise NotImplementedError("safe_div not implemented for this type combination")




monkey_the_camel(EclKW, 'intKeywords', EclKW.int_keywords, classmethod)
monkey_the_camel(EclKW, 'isNumeric', EclKW.is_numeric)
monkey_the_camel(EclKW, 'fortIOSize', EclKW.fort_io_size)
monkey_the_camel(EclKW, 'setName', EclKW.set_name)
monkey_the_camel(EclKW, 'getName', EclKW.get_name)
monkey_the_camel(EclKW, 'getMinMax', EclKW.get_min_max)
monkey_the_camel(EclKW, 'getMax', EclKW.get_max)
monkey_the_camel(EclKW, 'getMin', EclKW.get_min)
monkey_the_camel(EclKW, 'typeName', EclKW.type_name)
monkey_the_camel(EclKW, 'getEclType', EclKW.get_ecl_type)
monkey_the_camel(EclKW, 'numpyView', EclKW.numpy_view)
monkey_the_camel(EclKW, 'numpyCopy', EclKW.numpy_copy)
monkey_the_camel(EclKW, 'fixUninitialized', EclKW.fix_uninitialized)
monkey_the_camel(EclKW, 'getDataPtr', EclKW.get_data_ptr)
monkey_the_camel(EclKW, 'firstDifferent', EclKW.first_different)
