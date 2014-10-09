from OpenGL.GL import *
from ert_gui.viewer import GLTools, ShaderSource

class ShaderProgram(object):
    def __init__(self, name):
        super(ShaderProgram, self).__init__()
        self.__name = name
        self.__shader_sources = []
        """ :type: list of ShaderSource """

        self.__program_object = None

        self.__compiled = False


    def addShaderSource(self, shader_source):
        """
        @type shader_source: ShaderSource
        """
        self.__shader_sources.append(shader_source)


    def compileProgram(self):
        if self.__program_object is None:
            self.__program_object = glCreateProgram()

        if self.needsCompile():
            for shader_source in self.__shader_sources:
                shader_source.detach(self.__program_object)
                shader_source.compileShader()
                shader_source.attach(self.__program_object)

            glLinkProgram(self.__program_object)
            self.checkLinkingStatus()

            if self.__compiled:
                print("Program '%s' compiled and linked!" % self.__name)


    def needsCompile(self):
        """ @rtype: bool """
        source_needs_compile = False

        for shader_source in self.__shader_sources:
            if shader_source.needsCompile():
                source_needs_compile = True

        return not self.__compiled or source_needs_compile
            

    def checkLinkingStatus(self):
        self.__compiled = True
        link_status = glGetProgramiv(self.__program_object, GL_LINK_STATUS)
        if link_status == GL_FALSE:
            length = glGetProgramiv(self.__program_object, GL_INFO_LOG_LENGTH)
            if length > 0:
                print(glGetProgramInfoLog(self.__program_object))

            print("Linking of shader '%s' failed!" % self.__name)
            self.__compiled = False


    # def setUniform1i(self, name, value):
    #     if not self.needsCompile():
    #         location = glGetUniformLocation(self.__program_object, name)
    #         glUniform1i(location, value)
    #
    # def setUniform1f(self, name, value):
    #     if not self.needsCompile():
    #         location = glGetUniformLocation(self.__program_object, name)
    #         glUniform1f(location, value)


    def setUniformi(self, name, *args):
        if not self.needsCompile():
            count = len(args)
            if 0 < count <= 4:
                location = glGetUniformLocation(self.__program_object, name)

                if count == 1:
                    glUniform1i(location, args[0])
                elif count == 2:
                    glUniform2i(location, args[0], args[1])
                elif count == 3:
                    glUniform3i(location, args[0], args[1], args[2])
                elif count == 4:
                    glUniform4i(location, args[0], args[1], args[2], args[3])

    def setUniformf(self, name, *args):
        if not self.needsCompile():
            count = len(args)
            if 0 < count <= 4:
                location = glGetUniformLocation(self.__program_object, name)

                if count == 1:
                    glUniform1f(location, args[0])
                elif count == 2:
                    glUniform2f(location, args[0], args[1])
                elif count == 3:
                    glUniform3f(location, args[0], args[1], args[2])
                elif count == 4:
                    glUniform4f(location, args[0], args[1], args[2], args[3])

    # def setUniform4i(self, name, v1, v2, v3, v4):
    #         if not self.needsCompile():
    #             location = glGetUniformLocation(self.__program_object, name)
    #             glUniform4i(location, v1, v2, v3, v4)
    #
    #     def setUniform4f(self, name, v1, v2, v3, v4):
    #         if not self.needsCompile():
    #             location = glGetUniformLocation(self.__program_object, name)
    #             glUniform4f(location, v1, v2, v3, v4)
    #
    # def setUniform4i(self, name, v1, v2, v3, v4):
    #     if not self.needsCompile():
    #         location = glGetUniformLocation(self.__program_object, name)
    #         glUniform4i(location, v1, v2, v3, v4)
    #
    # def setUniform4f(self, name, v1, v2, v3, v4):
    #     if not self.needsCompile():
    #         location = glGetUniformLocation(self.__program_object, name)
    #         glUniform4f(location, v1, v2, v3, v4)


    def bindProgram(self):
        GLTools.checkForGLError()

        if self.needsCompile():
            self.unbindProgram()
            self.compileProgram()

        if not self.needsCompile():
            glUseProgram(self.__program_object)
            GLTools.checkForGLError()

    def unbindProgram(self):
        GLTools.checkForGLError()
        glUseProgram(0)
        GLTools.checkForGLError()