from OpenGL.GL import *
from ert.geo.polyline import Polyline


class PolylineDrawer(object):
    @staticmethod
    def drawPolyline(polyline):
        assert isinstance(polyline, Polyline)

        glColor(1.0, 1.0, 1.0)
        glLineWidth(2.0)

        glBegin(GL_LINE_STRIP)

        for x, y, z in polyline:
            glVertex(x, y, z)

        glEnd()

    @staticmethod
    def drawPolylineFlat(polyline, static_z):
        assert isinstance(polyline, Polyline)

        glColor(1.0, 1.0, 1.0)
        glLineWidth(2.0)

        glBegin(GL_LINE_STRIP)

        for x, y, z in polyline:
            glVertex(x, y, static_z)

        glEnd()