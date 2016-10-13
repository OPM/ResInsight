import ctypes
import ert
from cwrap import CWrapper
from ert.ecl import EclKW, EclFile, EclTypeEnum, FortIO
from ert.test import ExtendedTestCase, TestAreaContext
from ert.util import IntVector


ecl_lib = ert.load("libecl")
ecl_wrapper = CWrapper(ecl_lib)


freadIndexedData = ecl_wrapper.prototype("void ecl_kw_fread_indexed_data(fortio, int, int, int, int_vector, char*)") # fortio, offset, type, count, index_map, buffer
eclFileIndexedRead = ecl_wrapper.prototype("void ecl_file_indexed_read(ecl_file, char*, int, int_vector, char*)") # ecl_file, kw, index, index_map, buffer


class EclIndexedReadTest(ExtendedTestCase):
    def test_ecl_kw_indexed_read(self):
        with TestAreaContext("ecl_kw_indexed_read") as area:
            fortio = FortIO("index_test", mode=FortIO.WRITE_MODE)

            element_count = 100000
            ecl_kw = EclKW.create("TEST", element_count, EclTypeEnum.ECL_INT_TYPE)

            for index in range(element_count):
                ecl_kw[index] = index

            ecl_kw.fwrite(fortio)

            fortio.close()


            fortio = FortIO("index_test", mode=FortIO.READ_MODE)

            new_ecl_kw = EclKW.fread(fortio)

            for index in range(element_count):
                self.assertEqual(new_ecl_kw[index], index)

            index_map = IntVector()
            index_map.append(2)
            index_map.append(3)
            index_map.append(5)
            index_map.append(7)
            index_map.append(11)
            index_map.append(13)
            index_map.append(313)
            index_map.append(1867)
            index_map.append(5227)
            index_map.append(7159)
            index_map.append(12689)
            index_map.append(18719)
            index_map.append(32321)
            index_map.append(37879)
            index_map.append(54167)
            index_map.append(77213)
            index_map.append(88843)
            index_map.append(99991)

            char_buffer = ctypes.create_string_buffer(len(index_map) * ctypes.sizeof(ctypes.c_int))

            freadIndexedData(fortio, 24, EclTypeEnum.ECL_INT_TYPE, element_count, index_map, char_buffer)

            int_buffer = ctypes.cast(char_buffer, ctypes.POINTER(ctypes.c_int))

            for index, index_map_value in enumerate(index_map):
                self.assertEqual(index_map_value, int_buffer[index])



    def test_ecl_file_indexed_read(self):
        with TestAreaContext("ecl_file_indexed_read") as area:
            fortio = FortIO("ecl_file_index_test", mode=FortIO.WRITE_MODE)

            element_count = 100000
            ecl_kw_1 = EclKW.create("TEST1", element_count, EclTypeEnum.ECL_INT_TYPE)
            ecl_kw_2 = EclKW.create("TEST2", element_count, EclTypeEnum.ECL_INT_TYPE)

            for index in range(element_count):
                ecl_kw_1[index] = index
                ecl_kw_2[index] = index + 3

            ecl_kw_1.fwrite(fortio)
            ecl_kw_2.fwrite(fortio)

            fortio.close()

            ecl_file = EclFile("ecl_file_index_test")

            index_map = IntVector()
            index_map.append(2)
            index_map.append(3)
            index_map.append(5)
            index_map.append(7)
            index_map.append(11)
            index_map.append(13)
            index_map.append(313)
            index_map.append(1867)
            index_map.append(5227)
            index_map.append(7159)
            index_map.append(12689)
            index_map.append(18719)
            index_map.append(32321)
            index_map.append(37879)
            index_map.append(54167)
            index_map.append(77213)
            index_map.append(88843)
            index_map.append(99991)

            char_buffer_1 = ctypes.create_string_buffer(len(index_map) * ctypes.sizeof(ctypes.c_int))
            char_buffer_2 = ctypes.create_string_buffer(len(index_map) * ctypes.sizeof(ctypes.c_int))

            eclFileIndexedRead(ecl_file, "TEST2", 0, index_map, char_buffer_2)
            eclFileIndexedRead(ecl_file, "TEST1", 0, index_map, char_buffer_1)

            int_buffer_1 = ctypes.cast(char_buffer_1, ctypes.POINTER(ctypes.c_int))
            int_buffer_2 = ctypes.cast(char_buffer_2, ctypes.POINTER(ctypes.c_int))

            for index, index_map_value in enumerate(index_map):
                self.assertEqual(index_map_value, int_buffer_1[index])
                self.assertEqual(index_map_value, int_buffer_2[index] - 3)
