from OpenGL.GL import *
from OpenGL.GLU import gluPerspective
from ert_gui.viewer import UnityQuaternion


class Camera(object):
    def __init__(self):
        super(Camera, self).__init__()

        self.__quaternion = None
        self.__zoom = None

        self.__width_aspect = 1.0
        self.__height_aspect = 1.0
        self.__aspect = 1.0

        self.__orthographic_projection = False

        self.__mirror_x = False
        self.__mirror_y = False
        self.__mirror_z = False

        self.resetCamera()


    def resetCamera(self):
        rot_y = UnityQuaternion(180.0, 0.0, 1.0, 0.0)
        self.__quaternion = rot_y * UnityQuaternion(0.0, 1.0, 0.0, 0.0)
        self.__zoom = 1.0

    def rotate(self, x, y, z=0.0):
        rot_x = UnityQuaternion(x, 1.0, 0.0, 0.0)
        rot_y = UnityQuaternion(y, 0.0, 1.0, 0.0)
        rot_z = UnityQuaternion(z, 0.0, 0.0, 1.0)

        self.__quaternion = rot_x * rot_y * rot_z * self.__quaternion

    def __applyProjection(self):
        if self.__orthographic_projection:
            w = self.__width_aspect * self.__zoom
            h = self.__height_aspect * self.__zoom
            glOrtho(-w, w, -h, h, 0, 2000)
        else:
            # glFrustum(-width_aspect, width_aspect, -height_aspect, height_aspect, 0.1, 20)
            gluPerspective(60 * self.__zoom, self.__aspect, 0.1, 3000)


    def applyCamera(self):
        glMatrixMode(GL_MODELVIEW)
        glTranslate(0.0, 0.0, -1.0)
        glRotate(self.__quaternion.getAngleAsDegrees(), self.__quaternion.X, self.__quaternion.Y, self.__quaternion.Z)

        if self.__mirror_x:
            glScale(-1.0, 1.0, 1.0)

        if self.__mirror_y:
            glScale(1.0, -1.0, 1.0)

        if self.__mirror_z:
            glScale(1.0, 1.0, -1.0)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()

        self.__applyProjection()

        glMatrixMode(GL_MODELVIEW)


    def setupProjection(self, width, height):
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()

        glViewport(0, 0, width, height)

        if width != 0 and height != 0:
            self.__aspect = width / float(height)
            if height > width:
                self.__height_aspect = float(height) / width
            else:
                self.__width_aspect = float(width) / height

        self.__applyProjection()

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()


    def adjustZoom(self, delta):
        self.__zoom += delta

        if self.__zoom < 0.1:
            self.__zoom = 0.1

        if self.__zoom > 2.0:
            self.__zoom = 2.0

    def useOrthographicProjection(self, on):
        self.__orthographic_projection = on

    def mirrorX(self, on):
        self.__mirror_x = on

    def mirrorY(self, on):
        self.__mirror_y = on

    def mirrorZ(self, on):
        self.__mirror_z = on


