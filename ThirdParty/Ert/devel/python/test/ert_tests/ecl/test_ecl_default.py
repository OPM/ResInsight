
try:
    from unittest2 import TestCase
except ImportError:
    from unittest import TestCase

from ert.ecl import EclDefault

def has_ecl_local():
    try:
        import ert.ecl.ecl_local
        return True
    except ImportError:
        return False


class EclDefaultTest(TestCase):

    def test_ecl_defaults_methods(self):
        if (has_ecl_local()):
            try:
                import ert.ecl.ecl_local as ecl_local

                if hasattr(ecl_local, 'ecl_cmd'):
                    self.assertEqual(EclDefault.ecl_cmd(), ecl_local.ecl_cmd)
                else:
                    with self.assertRaises(NotImplementedError):
                        EclDefault.ecl_cmd()

                if hasattr(ecl_local, 'ecl_version'):
                    self.assertEqual(EclDefault.ecl_version(), ecl_local.ecl_version)
                else:
                    with self.assertRaises(NotImplementedError):
                        EclDefault.ecl_version()

                if hasattr(ecl_local, 'lsf_resource_request'):
                    self.assertEqual(EclDefault.lsf_resource_request(), ecl_local.lsf_resource_request)
                else:
                    with self.assertRaises(NotImplementedError):
                        EclDefault.lsf_resource_request()
                    
                if hasattr(ecl_local, 'driver_type'):
                    self.assertEqual(EclDefault.driver_type(), ecl_local.driver_type)
                else:
                    with self.assertRaises(NotImplementedError):
                        EclDefault.driver_type()

                if hasattr(ecl_local, 'driver_options'):
                    self.assertEqual(EclDefault.driver_options(), ecl_local.driver_options)
                else:
                    with self.assertRaises(NotImplementedError):
                        EclDefault.driver_options()

            except ImportError:
                self.fail("Unable to import ecl_local.py")
        else:
            with self.assertRaises(NotImplementedError):
                EclDefault.ecl_cmd()
                EclDefault.ecl_version()
                EclDefault.lsf_resource_request()
                EclDefault.driver_type()
                EclDefault.driver_options()
