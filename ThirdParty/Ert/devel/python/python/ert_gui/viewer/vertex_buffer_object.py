from OpenGL.GL import *
import numpy
from ert_gui.viewer import GLTools


class VertexBufferObject(object):
    def __init__(self):
        super(VertexBufferObject, self).__init__()

        self.__vertex_buffer_object = None
        self.__texture_buffer_object = None

        self.__vertices = []
        self.__texture_coordinates = []

        self.__data_changed = False


    def bind(self):
        if self.__vertex_buffer_object is None:
            self.createVertexBufferObject()

        if self.__data_changed:
            self.uploadData()

        glBindBuffer(GL_ARRAY_BUFFER, self.__vertex_buffer_object)
        glVertexPointer(3, GL_FLOAT, 0, None)

        glBindBuffer(GL_ARRAY_BUFFER, self.__texture_buffer_object)
        glTexCoordPointer(3, GL_FLOAT, 0, None)



    def createVertexBufferObject(self):
        self.__vertex_buffer_object = glGenBuffers(1)
        self.__texture_buffer_object = glGenBuffers(1)
        GLTools.checkForGLError()


    def uploadData(self):
        glBindBuffer(GL_ARRAY_BUFFER, self.__vertex_buffer_object)
        buffer = numpy.array(self.__vertices, dtype="f")
        glBufferData(GL_ARRAY_BUFFER, buffer, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__texture_buffer_object)
        buffer = numpy.array(self.__texture_coordinates, dtype="f")
        glBufferData(GL_ARRAY_BUFFER, buffer, GL_STATIC_DRAW)

        self.__data_changed = False
        GLTools.checkForGLError()


    def draw(self):
        glEnableClientState(GL_VERTEX_ARRAY)
        glEnableClientState(GL_TEXTURE_COORD_ARRAY)
        glClientActiveTexture(GL_TEXTURE0)

        self.bind()
        glDrawArrays(GL_QUADS, 0, len(self.__vertices) / 3)

        glDisableClientState(GL_VERTEX_ARRAY)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY)


    def addTexturedVertex(self, x, y, z):
        self.__vertices.append(x)
        self.__vertices.append(y)
        self.__vertices.append(z)
        self.__texture_coordinates.append(x)
        self.__texture_coordinates.append(y)
        self.__texture_coordinates.append(z)

        self.__data_changed = True


    def createGrid(self, nx, ny):
        for j in range(ny):
            for i in range(nx):
                p1 = self.xyz(i, j, nx, ny)
                p2 = self.xyz(i, j + 1, nx, ny)
                p3 = self.xyz(i + 1, j + 1, nx, ny)
                p4 = self.xyz(i + 1, j, nx, ny)

                self.addTexturedVertex(*p1)
                self.addTexturedVertex(*p2)
                self.addTexturedVertex(*p3)
                self.addTexturedVertex(*p4)


    def xyz(self, i, j, nx, ny):
        return float(i) / float(nx - 1), float(j) / float(ny - 1), float(0.0)