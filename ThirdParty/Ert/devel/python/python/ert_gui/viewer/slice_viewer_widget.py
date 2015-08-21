import os
from OpenGL.GL import *
from PyQt4.QtCore import Qt
from PyQt4.QtOpenGL import QGLWidget
from ert_gui.viewer import ShaderSource, Texture3D, ShaderProgram, VertexBufferObject, Bounds, Camera
from ert_gui.viewer.polyline_drawer import PolylineDrawer


class SliceViewer(QGLWidget):


    def __init__(self, textures=None, volume_bounds=None, color_scales=None, data_range=None, polylines=None, faults = None , parent=None):
        """
        @type textures: dict of (str, Texture3D)
        @type volume_bounds: Bounds
        """
        super(SliceViewer, self).__init__(parent=parent)

        self.__width = 1.0
        self.__height = 1.0

        self.__shader = self.createShader()


        if textures is not None:
            self.__textures = textures
        else:
            self.__textures = {}

        if color_scales is not None:
            self.__color_scales = color_scales
        else:
            self.__color_scales = {}

        if volume_bounds is None:
            self.__volume_bounds = Bounds()
            self.__volume_bounds.addPoint(0, 0, 0)
            self.__volume_bounds.addPoint(1, 1, 1)
        else:
            self.__volume_bounds = volume_bounds

        if data_range is None:
            self.__data_range = 1.0
        else:
            self.__data_range = data_range

        if polylines is None:
            self.__polylines = []
        else:
            self.__polylines = polylines

        if faults is None:
            self.__faults = [ ]
        else:
            self.__faults = faults


        self.__current_slice = 0


        self.__layer_count = 1
        self.__step_size = 1.0

        if "grid" in self.__textures:
            texture = self.__textures["grid"]
            self.__layer_count = texture.getDepth()
            self.__step_size = 1.0 / self.__layer_count
            self.__vbo = VertexBufferObject()
            self.__vbo.createGrid(texture.getWidth(), texture.getHeight())


        self.__camera = Camera()
        self.__mouse = {}

        self.__hide_inactive_cells = False
        self.__lighting = False
        self.__region_scaling = True
        self.__flat_polylines = False



    def createShader(self):
        vp = self.createShaderSource("default.vp")
        fp = self.createShaderSource("texturing.fp")

        shader = ShaderProgram("Slicer")
        shader.addShaderSource(vp)
        shader.addShaderSource(fp)

        return shader


    def paintGL(self):
        glPushMatrix()
        glPushAttrib(GL_ALL_ATTRIB_BITS)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        self.__shader.bindProgram()

        z = (self.__current_slice * self.__step_size) + (self.__step_size / 2.0)

        index = 0
        for key, texture in self.__textures.iteritems():
            glActiveTexture(GL_TEXTURE0 + index)
            texture.bind()
            self.__shader.setUniformi(key, index)

            glMatrixMode(GL_TEXTURE)
            glLoadIdentity()
            glTranslate(0.0, 0.0, z)


            glMatrixMode(GL_MODELVIEW)
            index += 1


        if "grid" in self.__textures:
            texture = self.__textures["grid"]
            self.__shader.setUniformi("grid_size", texture.getWidth(), texture.getHeight(), texture.getDepth())

        self.__shader.setUniformi("hide_inactive_cells", 1 if self.__hide_inactive_cells else 0)
        self.__shader.setUniformi("lighting", 1 if self.__lighting else 0)
        self.__shader.setUniformi("region_scaling", 1 if self.__region_scaling else 0)
        self.__shader.setUniformf("data_range", self.__data_range)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

        self.__camera.applyCamera()


        glScalef(1.0 / self.__volume_bounds.diffX, 1.0 / self.__volume_bounds.diffY, 1.0)
        glScalef(1.0, 1.0, 1.0 / self.__volume_bounds.diffZ)

        glTranslate(-self.__volume_bounds.minX - self.__volume_bounds.diffX / 2.0, -self.__volume_bounds.minY - self.__volume_bounds.diffY / 2.0, 0)
        glTranslate(0, 0, -self.__volume_bounds.minZ - self.__volume_bounds.diffZ / 2.0)

        glTranslate(-0.5, -0.5, 0.0)


        self.__vbo.draw()


        index = 0
        for key, texture in self.__textures.iteritems():
            glActiveTexture(GL_TEXTURE0 + index)
            texture.unbind()
            index += 1

        self.__shader.unbindProgram()

        for polyline in self.__polylines:
            if self.__flat_polylines:
                PolylineDrawer.drawPolylineFlat(polyline, self.__volume_bounds.minZ)
            else:
                PolylineDrawer.drawPolyline(polyline)


        for fault in self.__faults:
            layer = fault[8]
            for fault_line in layer:
                    
                polyline = fault_line.getPolyline()
                if self.__flat_polylines:
                    PolylineDrawer.drawPolylineFlat(polyline, self.__volume_bounds.minZ)
                else:
                    PolylineDrawer.drawPolyline(polyline)
                    

        glPopAttrib()
        glPopMatrix()



    def resizeGL(self, width, height):
        self.__camera.setupProjection(width, height)


    def initializeGL(self):
        print("OpenGL version %s" % glGetString(GL_VERSION))

        glClearColor(0.0, 0.0, 0.0, 1.0)
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
        glDisable(GL_LIGHTING)
        # glClearDepth(1.0)
        # glEnable(GL_BLEND)
        # glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        # glEnable(GL_NORMALIZE)
        glEnable(GL_DEPTH_TEST)
        # glShadeModel(GL_SMOOTH)

        print("Has non power of two support: %s" % extensions.hasGLExtension("GL_ARB_texture_non_power_of_two"))

        print("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS %d" % glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS))
        print("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS %d" % glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS))


    def setSliceSize(self, width=1.0, height=1.0):
        """ Aspect of rectangle on screen """

        if width > height:
            self.__width = 1.0
            self.__height = float(height) / width
        else:
            self.__width = float(width) / height
            self.__height = 1.0

    def createShaderSource(self, shader_path):
        root_path = os.getenv("ERT_SHARE_PATH")
        root_path = os.path.join(root_path, "viewer/shaders")
        path = os.path.join(root_path, shader_path)
        return ShaderSource(path)

    def wheelEvent(self, q_wheel_event):
        delta = q_wheel_event.delta() / 120.0

        self.__camera.adjustZoom(delta / 10.0)

        self.updateGL()

    def mouseMoveEvent(self, q_mouse_event):
        """@type q_mouse_event: QMouseEvent"""
        if q_mouse_event.buttons() & Qt.LeftButton == Qt.LeftButton:
            if "x" in self.__mouse and "y" in self.__mouse:
                diff_x = q_mouse_event.x() - self.__mouse["x"]
                diff_y = q_mouse_event.y() - self.__mouse["y"]

                self.__camera.rotate(diff_y, diff_x)

            self.__mouse["x"] = q_mouse_event.x()
            self.__mouse["y"] = q_mouse_event.y()

        if q_mouse_event.buttons() & Qt.RightButton == Qt.RightButton:
            if "x" in self.__mouse and "y" in self.__mouse:
                diff_x = q_mouse_event.x() - self.__mouse["x"]
                diff_y = q_mouse_event.y() - self.__mouse["y"]

                #self.__camera.rotate(0.0, 0.0, diff_y / 2.0)
                self.__camera.translate(diff_x / 1000.0 , diff_y / 1000.0 )

            self.__mouse["x"] = q_mouse_event.x()
            self.__mouse["y"] = q_mouse_event.y()

        self.updateGL()

    def mouseReleaseEvent(self, q_mouse_event):
        """@type q_mouse_event: QMouseEvent"""
        self.__mouse.clear()

    def mousePressEvent(self, q_mouse_event):
        """@type q_mouse_event: QMouseEvent"""
        if q_mouse_event.buttons() & Qt.MiddleButton == Qt.MiddleButton:
            self.__camera.resetCamera()
            self.updateGL()


    def hideInactiveCells(self, hide):
        self.__hide_inactive_cells = hide
        self.updateGL()

    def setCurrentSlice(self, slice_number):
        self.__current_slice = slice_number
        self.updateGL()

    def useOrthographicProjection(self, on):
        self.__camera.useOrthographicProjection(on)
        self.updateGL()

    def useLighting(self, on):
        self.__lighting = on
        self.updateGL()

    def changeColorScale(self, color_scale):
        color_scale = str(color_scale)
        self.__textures["color_scale"] = self.__color_scales[color_scale]
        self.updateGL()

    def useRegionScaling(self, on):
        self.__region_scaling = on
        self.updateGL()

    def useInterpolationOnData(self, interpolate):
        if "grid_data" in self.__textures:
            if interpolate:
                self.__textures["grid_data"].setFilterMode(GL_LINEAR)
            else:
                self.__textures["grid_data"].setFilterMode(GL_NEAREST)

            self.updateGL()

    def mirrorX(self, on):
        self.__camera.mirrorX(on)
        self.updateGL()

    def mirrorY(self, on):
        self.__camera.mirrorY(on)
        self.updateGL()

    def mirrorZ(self, on):
        self.__camera.mirrorZ(on)
        self.updateGL()

    def toggleFlatPolylines(self, on):
        self.__flat_polylines = on
        self.updateGL()
