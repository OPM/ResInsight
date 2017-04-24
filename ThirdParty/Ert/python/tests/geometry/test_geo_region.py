from ert.geo import GeoRegion, GeoPointset, CPolyline, Surface
from ert.test import ExtendedTestCase, TestAreaContext


class GeoRegionTest(ExtendedTestCase):

    def test_init(self):
        pointset = GeoPointset()
        georegion = GeoRegion(pointset)
        self.assertEqual(0, len(georegion))

    def test_repr(self):
        pointset = GeoPointset()
        georegion = GeoRegion(pointset)
        self.assertTrue(repr(georegion).startswith('GeoRegion'))

    @staticmethod
    def small_surface():
        ny,nx = 12,12
        xinc,yinc = 1, 1
        xstart,ystart = -1, -1
        angle = 0.0
        s_args = (None, nx, ny, xinc, yinc, xstart, ystart, angle)
        return Surface(*s_args)

    def test_select_polygon(self):
        surface = self.small_surface()
        pointset = GeoPointset.fromSurface(surface)
        georegion = GeoRegion(pointset)
        self.assertEqual(0, len(georegion))
        points = [(-0.1,2.0), (1.9,8.1), (6.1,8.1), (9.1,5), (7.1,0.9)]
        polygon = CPolyline(name='test_polygon', init_points=points)
        picked = 52  # https://www.futilitycloset.com/2013/04/24/picks-theorem/
        georegion.select_inside(polygon)
        self.assertEqual(picked, len(georegion))
        georegion.deselect_inside(polygon)
        self.assertEqual(0, len(georegion))
        georegion.select_outside(polygon)
        self.assertEqual(len(surface) - picked, len(georegion))
        georegion.deselect_outside(polygon)
        self.assertEqual(0, len(georegion))

        georegion.select_inside(polygon)
        georegion.select_outside(polygon)
        self.assertEqual(len(surface), len(georegion))
        georegion.deselect_inside(polygon)
        georegion.deselect_outside(polygon)
        self.assertEqual(0, len(georegion))

        georegion.select_inside(polygon)
        self.assertEqual(picked, len(georegion))
        internal_square = [(2.5,2.5), (2.5,6.5), (6.5,6.5), (6.5,2.5)]
        georegion.deselect_inside(CPolyline(init_points=internal_square))
        self.assertEqual(picked - 4*4, len(georegion))  # internal square is 4x4


    def test_select_halfspace(self):
        surface = self.small_surface()
        pointset = GeoPointset.fromSurface(surface)
        georegion = GeoRegion(pointset)
        self.assertEqual(0, len(georegion))
        line = [(-0.1,2.0), (1.9,8.1)]
        picked = 118
        georegion.select_above(line)
        self.assertEqual(picked, len(georegion))
        georegion.deselect_above(line)
        self.assertEqual(0, len(georegion))
        georegion.select_below(line)
        self.assertEqual(len(surface) - picked, len(georegion))
        georegion.deselect_below(line)
        self.assertEqual(0, len(georegion))

        georegion.select_above(line)
        georegion.select_below(line)
        self.assertEqual(len(surface), len(georegion))
        georegion.deselect_above(line)
        georegion.deselect_below(line)
        self.assertEqual(0, len(georegion))


    def test_raises(self):
        surface = self.small_surface()
        pointset = GeoPointset.fromSurface(surface)
        georegion = GeoRegion(pointset)
        with self.assertRaises(ValueError):
            georegion.select_above(((2,), (1, 3)))
        with self.assertRaises(ValueError):
            georegion.select_above((('not-a-number', 2), (1, 3)))
