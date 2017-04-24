from ert.test import TestAreaContext, ExtendedTestCase

from ert.ecl import EclDataType, EclTypeEnum

class EclDataTypeTest(ExtendedTestCase):

    # All of the below should list their elements in the same order as
    # EclTypeEnum!
    # [char, float, double, int, bool, mess]

    ELEMENT_SIZE = [9, 4, 8, 4, 4, 0]

    VERIFIERS = [
            EclDataType.is_char,
            EclDataType.is_float,
            EclDataType.is_double,
            EclDataType.is_int,
            EclDataType.is_bool,
            EclDataType.is_mess
            ]

    TYPE_NAMES = ["CHAR", "REAL", "DOUB", "INTE", "LOGI", "MESS"]

    def test_alloc_from_type(self):
        for (ecl_type, element_size) in zip(EclTypeEnum.enums(), self.ELEMENT_SIZE):
            data_type = EclDataType(ecl_type)
            self.assertEqual(ecl_type, data_type.type)
            self.assertEqual(element_size, data_type.element_size)

    def test_alloc(self):
        for (ecl_type, element_size) in zip(EclTypeEnum.enums(), self.ELEMENT_SIZE):
            data_type = EclDataType(ecl_type, element_size)
            self.assertEqual(ecl_type, data_type.type)
            self.assertEqual(element_size, data_type.element_size)
    
    def test_type_verifiers(self):
        for (ecl_type, verifier) in zip(EclTypeEnum.enums(), self.VERIFIERS):
            ecl_type = EclDataType(ecl_type)
            self.assertTrue(verifier(ecl_type))

    def test_get_type_name(self):
        for (ecl_type, type_name) in zip(EclTypeEnum.enums(), self.TYPE_NAMES):
            self.assertEqual(type_name, EclDataType(ecl_type).type_name)

    def test_initialization_validation(self):
        invalid_args = [
                        (None, 0, self.TYPE_NAMES[0]),
                        (1, None, self.TYPE_NAMES[0]),
                        (1, 0, self.TYPE_NAMES[0]),
                        (None, None, None),
                        (None, 12, None)
                    ]

        for inv_arg in invalid_args:
            with self.assertRaises(ValueError):
                EclDataType(inv_arg[0], inv_arg[1], inv_arg[2])

    def test_create_from_type_name(self):
        for (ecl_type, type_name) in zip(EclTypeEnum.enums(), self.TYPE_NAMES):
            self.assertEqual(ecl_type, EclDataType.create_from_type_name(type_name).type)

    def test_is_numeric(self):
        numeric_types = [
                    EclTypeEnum.ECL_INT_TYPE,
                    EclTypeEnum.ECL_FLOAT_TYPE,
                    EclTypeEnum.ECL_DOUBLE_TYPE
                ]

        for ecl_type in numeric_types:
            self.assertTrue(EclDataType(ecl_type).is_numeric())

        for ecl_type in set(EclTypeEnum.enums())-set(numeric_types):
            self.assertFalse(EclDataType(ecl_type).is_numeric())

    def test_equals(self):
        for ecl_type in EclTypeEnum.enums():
            self.assertTrue( EclDataType(ecl_type).is_equal(EclDataType(ecl_type)) )
            self.assertEqual( EclDataType(ecl_type), EclDataType(ecl_type) )

            for other in set(EclTypeEnum.enums())-set([ecl_type]):
                self.assertFalse( EclDataType(ecl_type).is_equal(EclDataType(other)) )
                self.assertNotEqual( EclDataType(ecl_type), EclDataType(other) )

    def test_hash(self):
        all_types = set()

        for index, ecl_type in enumerate(EclTypeEnum.enums()):
            all_types.add(EclDataType(ecl_type))
            self.assertEqual(index+1, len(all_types))

        for index, ecl_type in enumerate(EclTypeEnum.enums()):
            all_types.add(EclDataType(ecl_type))
        self.assertEqual(len(EclTypeEnum.enums()), len(all_types))
