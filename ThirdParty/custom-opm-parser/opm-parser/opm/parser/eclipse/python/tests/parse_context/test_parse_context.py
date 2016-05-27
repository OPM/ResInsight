from unittest import TestCase

from ert.test import TestAreaContext,ExtendedTestCase

from opm.parser import ParseContext,ErrorAction



class ParseContextTest(ExtendedTestCase):

    def test_parse_mode(self):
        pm = ParseContext()

        pm.update( "PARSE*" , ErrorAction.IGNORE )


        
    def test_action_enum(self):
        #source_file_path = "../../../Parser/InputErrorAction.hpp"
        #self.assertEnumIsFullyDefined(ErrorAction , "Action", source_file_path)

        for (key,value) in [("THROW_EXCEPTION" , 0), ("WARN" , 1) , ("IGNORE" , 2)]:
            self.assertTrue(ErrorAction.__dict__.has_key(key), "Enum does not have identifier: %s" % key)
            class_value = ErrorAction.__dict__[key]
            self.assertEqual(class_value, value, "Enum value for identifier: %s does not match: %s != %s" % (key, class_value, value))

