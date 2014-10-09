from OpenGL.GL import *
from OpenGL.raw.GLU import gluErrorString

class GLTools(object):
    @staticmethod
    def checkForGLError(message=""):
        glFlush()
        error = glGetError()
        if error != GL_NO_ERROR:
            print("Error occured: %s" % message)
            raise UserWarning("OpenGL Error code: %d with message: %s" % (error, gluErrorString(error)))
