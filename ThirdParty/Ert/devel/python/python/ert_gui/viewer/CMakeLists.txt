set(PYTHON_SOURCES
    __init__.py
    bounds.py
    camera.py
    opengl_tools.py
    polyline_drawer.py
    shader_program.py
    shader_source.py
    slice_settings_widget.py
    slice_viewer.py
    slice_viewer_widget.py
    texture.py
    unity_quaternion.py
    vertex_buffer_object.py
)

add_python_package("python.ert_gui.viewer"  ${PYTHON_INSTALL_PREFIX}/ert_gui/viewer "${PYTHON_SOURCES}" True)

