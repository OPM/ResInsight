from OpenGL.GL import *
from ert_gui.viewer import GLTools


class Texture3D(object):
    def __init__(self, width=64, height=64, depth=64, data=None, internal_format=GL_ALPHA16, pixel_format=GL_ALPHA, pixel_type=GL_FLOAT):
        super(Texture3D, self).__init__()

        self.__width = width
        self.__height = height
        self.__depth = depth

        self.__internal_format = internal_format
        self.__pixel_format = pixel_format
        self.__pixel_type = pixel_type
        self.__filter_mode = GL_NEAREST

        self.__texture_reference = None

        self.__data = data

        self.__data_changed = data is not None


    def bind(self):
        glEnable(GL_TEXTURE_3D)

        if self.__texture_reference is None:
            self.createTexture()

        if self.__data_changed:
            self.uploadData()


        glBindTexture(GL_TEXTURE_3D, self.__texture_reference)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, self.__filter_mode)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, self.__filter_mode) #GL_LINEAR or GL_NEAREST
        GLTools.checkForGLError()


    def unbind(self):
        glBindTexture(GL_TEXTURE_3D, 0)


    def createTexture(self):
        glEnable(GL_TEXTURE_3D)

        self.__texture_reference = glGenTextures(1)

        glBindTexture(GL_TEXTURE_3D, self.__texture_reference)

        # glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE)
        # glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, [0, 0, 0, 0])
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, self.__filter_mode)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, self.__filter_mode) #GL_LINEAR or GL_NEAREST

        GLTools.checkForGLError()

        glTexImage3D(GL_TEXTURE_3D, 0, self.__internal_format, self.__width, self.__height, self.__depth, 0, self.__pixel_format, self.__pixel_type, None)

        GLTools.checkForGLError()

        glBindTexture(GL_TEXTURE_3D, 0)


    def uploadData(self):
        glBindTexture(GL_TEXTURE_3D, self.__texture_reference)
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, self.__width, self.__height, self.__depth, self.__pixel_format, self.__pixel_type, self.__data)
        GLTools.checkForGLError()
        glBindTexture(GL_TEXTURE_3D, 0)
        self.__data_changed = False


    def setData(self, data):
        self.__data_changed = True
        self.__data = data

    def getWidth(self):
        return self.__width

    def getHeight(self):
        return self.__height

    def getDepth(self):
        return self.__depth

    def setFilterMode(self, mode):
        self.__filter_mode = mode



class Texture1D(object):
    def __init__(self, width=64, data=None, internal_format=GL_RGBA16F, pixel_format=GL_RGBA, pixel_type=GL_FLOAT, wrap=GL_REPEAT):
        super(Texture1D, self).__init__()

        self.__width = width

        self.__internal_format = internal_format
        self.__pixel_format = pixel_format
        self.__pixel_type = pixel_type
        self.__wrap = wrap

        self.__texture_reference = None

        self.__data = data

        self.__data_changed = data is not None


    def bind(self):
        glEnable(GL_TEXTURE_1D)

        if self.__texture_reference is None:
            self.createTexture()

        if self.__data_changed:
            self.uploadData()


        glBindTexture(GL_TEXTURE_1D, self.__texture_reference)
        GLTools.checkForGLError()


    def unbind(self):
        glBindTexture(GL_TEXTURE_1D, 0)


    def createTexture(self):
        glEnable(GL_TEXTURE_1D)

        self.__texture_reference = glGenTextures(1)

        glBindTexture(GL_TEXTURE_1D, self.__texture_reference)

        # glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, self.__wrap)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) #GL_LINEAR or GL_NEAREST

        GLTools.checkForGLError()

        glTexImage1D(GL_TEXTURE_1D, 0, self.__internal_format, self.__width, 0, self.__pixel_format, self.__pixel_type, None)

        GLTools.checkForGLError()

        glBindTexture(GL_TEXTURE_1D, 0)


    def uploadData(self):
        glBindTexture(GL_TEXTURE_1D, self.__texture_reference)
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, self.__width, self.__pixel_format, self.__pixel_type, self.__data)
        GLTools.checkForGLError()
        glBindTexture(GL_TEXTURE_1D, 0)
        self.__data_changed = False


    def setData(self, data):
        self.__data_changed = True
        self.__data = data

    def getWidth(self):
        return self.__width
