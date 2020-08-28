from opm._common import eclArrType
from opm._common import EclFile
from opm._common import ERst
from opm._common import ESmry
from opm._common import EGrid
from opm._common import ERft
from opm._common import EclOutput

import sys
import datetime
import numpy as np
import datetime

# When extracting the strings from CHAR keywords we get a character array, in
# Python this becomes a list of bytes. This desperate monkey-patching is to
# ensure the EclFile class returns normal Python strings in the case of CHAR
# arrays. The return value is normal Python list of strings.

@property
def eclfile_get_list_of_arrays(self):

    if sys.version_info.major == 2:
        rawData = self.__get_list_of_arrays()
        return [ ( x[0].encode("utf-8"), x[1], x[2] ) for x in rawData ]
    else:
        return self.__get_list_of_arrays()


def getitem_eclfile(self, arg):

    if isinstance(arg, tuple):
        data, array_type = self.__get_data(str(arg[0]), int(arg[1]))
    else:
        data, array_type = self.__get_data(arg)

    if array_type == eclArrType.CHAR:
        return [ x.decode("utf-8") for x in data ]

    return data


def erst_get_list_of_arrays(self, arg):

    if sys.version_info.major==2:
        rawData = self.__get_list_of_arrays(arg)
        return [ ( x[0].encode("utf-8"), x[1], x[2] ) for x in rawData ]
    else:
        return self.__get_list_of_arrays(arg)


def getitem_erst(self, arg):

    if not isinstance(arg, tuple):
        raise ValueError("expecting tuple argument, (index, rstep), (name, rstep) or (name, rstep, occurrence) ")

    if len(arg) == 2:
        if isinstance(arg[0], int):
            data, array_type = self.__get_data(arg[0], int(arg[1]))
        else:
            data, array_type = self.__get_data(str(arg[0]), int(arg[1]), 0)  # default first occurrence
    elif len(arg) == 3:
        data, array_type = self.__get_data(str(arg[0]), int(arg[1]), int(arg[2]))
    else:
        raise ValueError("expecting tuple argument with 2 or 3 argumens: (index, rstep), (name, rstep) or (name, rstep, occurrence) ")

    if array_type == eclArrType.CHAR:
        return [ x.decode("utf-8") for x in data ]

    return data


def contains_erst(self, arg):

    if isinstance(arg, tuple):
        if len(arg) == 2:
            return self.__contains((arg[0], arg[1]))
        else:
            raise ValueError("expecting tuple (array name , report step number) or \
                              or report step number")

    elif isinstance(arg, int):
        return self.__has_report_step(arg)

    else:
        raise ValueError("expecting tuple (array name , report step number) or \
                          or report step number")

@property
def esmry_end_date(self):

    start = self.start_date
    time = self.__get_all("TIME")

    return start + datetime.timedelta(days = float(time[-1]))


def getitem_esmry(self, arg):

    if isinstance(arg, tuple):
        if arg[1] == True:
            return self.__get_at_rstep(arg[0])
        else:
            return self.__get_all(arg[0])
    else:
        return self.__get_all(arg)


def contains_erft(self, arg):

    if isinstance(arg, tuple):
        if len(arg) == 4:
            return self.__has_rft(arg[0], arg[1], arg[2], arg[3])
        elif len(arg) == 5:
            return self.__has_array(arg[0], arg[1], (arg[2], arg[3], arg[4]))
        elif len(arg) == 2:
            return self.__has_array(arg[0], arg[1])
        else:
            raise ValueError("expecting tuple (wellname, year, month, day) or \
                (arrayName, wellname, year, month, day) or (arrayName, report_index)")
    else:
        raise ValueError("expecting tuple (wellname, year, month, day) or \
                (arrayName, wellname, year, month, day) or (arrayName, report_index)")

@property
def erft_list_of_rfts(self):

    if sys.version_info.major==2:
        data = self.__get_list_of_rfts()
        return [ ( x[0].encode("utf-8"), x[1], x[2] ) for x in data ]
    else:
        return self.__get_list_of_rfts()


def erft_list_of_arrays(self, arg1, arg2 = None):

    if not arg2:
        data = self.__get_list_of_arrays(int(arg1))
    else:
        data = self.__get_list_of_arrays(str(arg1), int(arg2[0]), int(arg2[1]), int(arg2[2]))

    if sys.version_info.major==2:
        return [ ( x[0].encode("utf-8"), x[1], x[2] ) for x in data ]
    else:
        return data


def getitem_erft(self, arg):

    if isinstance(arg, tuple):
        if len(arg) == 2:
            data, array_type = self.__get_data(arg[0], arg[1])
        elif len(arg) == 5:
            data, array_type = self.__get_data(arg[0], arg[1], arg[2], arg[3], arg[4])
        else:
           raise ValueError("ERft.__getitem__, expecting tuple (name, index) or (name, well, y, m, d)")
    else:
        raise ValueError("ERft.__getitem__, expecting tuple (name, index) or (name, well, y, m, d)")

    if array_type == eclArrType.CHAR:
        return np.array([ x.decode("utf-8") for x in data ])
    else:
        return data


'''
  EclOutput supports writing of numpy arrays. Data types
  (CHAR, LOGI, REAL, DOUB and INTE) is derived from the numpy dtype property
  EclOutput partly supports writing of python lists
  (CHAR, LOGI, INTE)
'''

def ecloutput_write(self, name, array):

    if isinstance(array, list):
        if all(isinstance(element, str) for element in array):
            array = np.array(array)
        elif all(isinstance(element, bool) for element in array):
            array = np.array(array)
        elif all(isinstance(element, int) for element in array):
            array = np.array(array, dtype = "int32")
        elif sys.version_info.major == 2 and all(isinstance(element, unicode) for element in array):
            array = np.array(array)
        else:
            raise ValueError("!!array {} is python list, type {}, not supported".format(name, type(array[0])))

    if not isinstance(array, np.ndarray):
        raise ValueError("EclOutput - write function works only for numpy arrays")

    if array.dtype == "float32":
        self.__write_real_array(name, array)
    elif array.dtype == "int32":
        self.__write_inte_array(name, array)
    elif array.dtype == "int64":
        print ("!Warning, writing numpy dtype=int64 to 32 bit integer format")
        self.__write_inte_array(name, array)
    elif array.dtype == "float64":
        self.__write_doub_array(name, array)
    elif array.dtype == "bool":
        self.__write_logi_array(name, array)
    elif  array.dtype.kind in {'U', 'S'}:
        self.__write_char_array(name, array)
    else:
        raise ValueError("unknown array type for array {}".format(name))


setattr(EclFile, "__getitem__", getitem_eclfile)
setattr(EclFile, "arrays", eclfile_get_list_of_arrays)

setattr(ERst, "__contains__", contains_erst)
setattr(ERst, "arrays", erst_get_list_of_arrays)
setattr(ERst, "__getitem__", getitem_erst)

setattr(ESmry, "end_date", esmry_end_date)
setattr(ESmry, "__getitem__", getitem_esmry)

setattr(ERft, "__contains__", contains_erft)
setattr(ERft, "list_of_rfts", erft_list_of_rfts)
setattr(ERft, "arrays", erft_list_of_arrays)
setattr(ERft, "__getitem__",getitem_erft)

setattr(EclOutput, "write", ecloutput_write)
