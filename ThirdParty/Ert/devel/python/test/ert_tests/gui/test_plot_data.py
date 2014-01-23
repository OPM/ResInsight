from ert.enkf.plot import DictProperty
from ert_tests import ExtendedTestCase


class PlotDataTest(ExtendedTestCase):

    def test_dict_properties(self):

        class Test(dict):
            x = DictProperty("x")
            user_data = DictProperty("user_data")
            dirty = DictProperty("dirty")

            def __init__(self):
                super(Test, self).__init__()
                self.x = 0.0
                self["dirty"] = True



        t1 = Test()
        self.assertEqual(t1.x, 0.0)
        with self.assertRaises(AttributeError):
            user_data = t1.user_data

        self.assertTrue(t1.dirty)

        t1.user_data = None

        self.assertEqual(t1.x, t1["x"])
        self.assertEqual(t1.user_data, t1["user_data"])
        self.assertEqual(t1.dirty, t1["dirty"])

        t2 = Test()
        t2.user_data = None
        self.assertEqual(t1.x, t2.x)
        self.assertEqual(t1.user_data, t2.user_data)
        self.assertEqual(t1.dirty, t2.dirty)

        t2.x = 0.5
        t2.user_data = "Hello!"
        t2.dirty = False

        self.assertEqual(t2.x, 0.5)
        self.assertEqual(t2.user_data, "Hello!")
        self.assertFalse(t2.dirty)

        self.assertNotEqual(t1.user_data, t2.user_data)


        # print(t1, t2)

    def test_dict_property_mismatch(self):
        # property name and dictionary name should match
        class Test(dict):
            value = DictProperty("valeu")
            max = DictProperty("max")

            def __init__(self):
                super(Test, self).__init__()
                self.value = 5
                self.max = 10


        t = Test()
        with self.assertRaises(AttributeError):
            v = t.value

        m = t.max