from cwrap import BaseCClass
from ecl.test  import ExtendedTestCase


class BaseCClassTest(ExtendedTestCase):

    def test_none_assertion(self):
        self.assertFalse(None > 0)

    def test_creation(self):
        with self.assertRaises(ValueError):
            obj = BaseCClass(0)


        obj = BaseCClass( 10 )
        self.assertTrue( obj )
        
        obj._invalidateCPointer( )
        self.assertFalse( obj )
