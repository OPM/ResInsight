from ecl.geo import GeoPointset, Surface
from ecl.test import ExtendedTestCase, TestAreaContext


class GeoPointsetTest(ExtendedTestCase):

    def test_init(self):
        gp = GeoPointset()
        self.assertEqual(0, len(gp))

    def test_repr(self):
        gp = GeoPointset()
        self.assertTrue(repr(gp).startswith('GeoPointset'))

    def test_from_surface(self):
        srf_path  = self.createTestPath("local/geometry/surface/valid_ascii.irap")
        srf = Surface(srf_path)
        gp = GeoPointset.fromSurface(srf)
        self.assertEqual(3871, len(srf))
        self.assertEqual(len(srf), len(gp))

    def test_getitem(self):
        srf_path  = self.createTestPath("local/geometry/surface/valid_ascii.irap")
        srf = Surface(srf_path)
        gp = GeoPointset.fromSurface(srf)
        for i in (561, 1105, 1729, 2465, 2821):
            self.assertEqual(gp[i], srf[i])
