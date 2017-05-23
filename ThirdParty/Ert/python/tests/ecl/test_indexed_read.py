import ctypes
import ecl

from ecl.ecl import EclPrototype
from ecl.ecl import EclKW, EclFile, EclDataType, FortIO
from ecl.test import ExtendedTestCase, TestAreaContext
from ecl.util import IntVector

class EclIndexedReadTest(ExtendedTestCase):
    _freadIndexedData   = EclPrototype("void ecl_kw_fread_indexed_data_python(fortio, int, ecl_data_type, int, int_vector, char*)", bind = False) # fortio, offset, type, count, index_map, buffer
    _eclFileIndexedRead = EclPrototype("void ecl_file_indexed_read(ecl_file, char*, int, int_vector, char*)", bind = False) # ecl_file, kw, index, index_map, buffer

    def test_ecl_kw_indexed_read(self):
        with TestAreaContext("ecl_kw_indexed_read") as area:
            fortio = FortIO("index_test", mode=FortIO.WRITE_MODE)

            element_count = 100000
            ecl_kw = EclKW("TEST", element_count, EclDataType.ECL_INT)

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

            self._freadIndexedData(fortio, 24, EclDataType.ECL_INT, element_count, index_map, char_buffer)

            int_buffer = ctypes.cast(char_buffer, ctypes.POINTER(ctypes.c_int))

            for index, index_map_value in enumerate(index_map):
                self.assertEqual(index_map_value, int_buffer[index])



    def test_ecl_file_indexed_read(self):
        with TestAreaContext("ecl_file_indexed_read") as area:
            fortio = FortIO("ecl_file_index_test", mode=FortIO.WRITE_MODE)

            element_count = 100000
            ecl_kw_1 = EclKW("TEST1", element_count, EclDataType.ECL_INT)
            ecl_kw_2 = EclKW("TEST2", element_count, EclDataType.ECL_INT)

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

            self._eclFileIndexedRead(ecl_file, "TEST2", 0, index_map, char_buffer_2)
            self._eclFileIndexedRead(ecl_file, "TEST1", 0, index_map, char_buffer_1)

            int_buffer_1 = ctypes.cast(char_buffer_1, ctypes.POINTER(ctypes.c_int))
            int_buffer_2 = ctypes.cast(char_buffer_2, ctypes.POINTER(ctypes.c_int))

            for index, index_map_value in enumerate(index_map):
                self.assertEqual(index_map_value, int_buffer_1[index])
                self.assertEqual(index_map_value, int_buffer_2[index] - 3)
