import os
from OpenGL.GL import *
from ert_gui.viewer import GLTools


class ShaderSource(object):

    def __init__(self, shader_source):
        super(ShaderSource, self).__init__()

        if not os.path.exists(shader_source):
            raise IOError("Shader source file '%s' does not exist!" % shader_source)

        if shader_source.endswith(".vertex") or shader_source.endswith(".vp"):
            self.__type = GL_VERTEX_SHADER
        elif shader_source.endswith(".fragment") or shader_source.endswith(".fp"):
            self.__type = GL_FRAGMENT_SHADER
        else:
            raise UserWarning("Unknown shader file type: %s" % shader_source)

        self.__time_stamp = 0
        self.__shader_source = shader_source
        self.__shader_object = None
        self.__compiled = False
        self.__attached = None


    def compileShader(self):
        if self.__shader_object is None:
            self.__shader_object = glCreateShader(self.__type)

        if self.needsCompile():
            with open(self.__shader_source, "r") as f:
                shader_code = f.read()

            glShaderSource(self.__shader_object, shader_code)
            GLTools.checkForGLError()

            glCompileShader(self.__shader_object)
            # self.checkForError()

            self.checkCompileStatus()

            if self.__compiled:
                self.__time_stamp = os.path.getmtime(self.__shader_source)
                print("Shader source '%s' compiled!" % self.__shader_source)

    def getType(self):
        return self.__type

    def needsCompile(self):
        """ @rtype: bool """
        return not self.__compiled or os.path.getmtime(self.__shader_source) != self.__time_stamp

    def checkCompileStatus(self):
        self.__compiled = True
        compile_status = glGetShaderiv(self.__shader_object, GL_COMPILE_STATUS)
        if compile_status == GL_FALSE:
            length = glGetShaderiv(self.__shader_object, GL_INFO_LOG_LENGTH)
            if length > 0:
                print(glGetShaderInfoLog(self.__shader_object))

            print("Compilation of shader '%s' failed!" % self.__shader_source)

            self.__compiled = False

    def attach(self, program_object):
        if self.__attached is None:
            self.__attached = program_object
            glAttachShader(program_object, self.__shader_object)

    def detach(self, program_object):
        if self.__attached is not None:
            self.__attached = None
            glDetachShader(program_object, self.__shader_object)
