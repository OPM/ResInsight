from unittest2 import TestCase
from ert.ecl import EclDefault


class EclDefaultTest(TestCase):

    def test_ecl_defaults_methods(self):
        try:
            import ert.ecl.ecl_local as ecl_local

            self.assertEqual(EclDefault.ecl_cmd(), ecl_local.ecl_cmd)
            self.assertEqual(EclDefault.ecl_version(), ecl_local.ecl_version)
            self.assertEqual(EclDefault.lsf_resource_request(), ecl_local.lsf_resource_request)
            self.assertEqual(EclDefault.driver_type(), ecl_local.driver_type)
            self.assertEqual(EclDefault.driver_options(), ecl_local.driver_options)
        except ImportError:
            self.fail("Unable to import ecl_local.py")

