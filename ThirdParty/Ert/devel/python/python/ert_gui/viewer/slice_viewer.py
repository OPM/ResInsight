import os
from collections import OrderedDict
from OpenGL.GL import *
from PyQt4.QtCore import Qt
from PyQt4.QtGui import QApplication, QMainWindow, QDockWidget
import sys
import traceback
from ert.ecl import EclTypeEnum, EclKW, EclGrid 
from ert.ecl.faults import  FaultCollection
from ert.geo.xyz_io import XYZIo
from ert_gui.viewer import Texture3D, Bounds, SliceViewer, SliceSettingsWidget, Texture1D


def loadGrid(path, load_actnum=True):
    """ @rtype: EclGrid """
    with open(path, "r") as f:
        specgrid = EclKW.read_grdecl(f, "SPECGRID", ecl_type=EclTypeEnum.ECL_INT_TYPE, strict=False)
        zcorn = EclKW.read_grdecl(f, "ZCORN")
        coord = EclKW.read_grdecl(f, "COORD")

        actnum = None
        if load_actnum:
            actnum = EclKW.read_grdecl(f, "ACTNUM", ecl_type=EclTypeEnum.ECL_INT_TYPE)

        mapaxes = EclKW.read_grdecl(f, "MAPAXES")
        grid = EclGrid.create(specgrid, zcorn, coord, actnum, mapaxes=mapaxes)

    return grid


def loadKW(keyword, ecl_type, path):
    """ @rtype: EclKW """
    with open(path, "r") as f:
        kw_data = EclKW.read_grdecl(f, keyword, ecl_type=ecl_type)

    return kw_data

def loadGridData(path):
    grid = loadGrid(path)

    nx, ny, nz, nactive = grid.dims
    print(nx, ny, nz)

    bounds = Bounds()

    grid_data = []
    index = 0
    for z in range(nz):
        for y in range(ny):
            for x in range(nx):
                # x, y, z = grid.get_corner_xyz(0, global_index=index)
                x, y, z = grid.get_xyz(global_index=index)
                active = grid.active(global_index=index)
                if active:
                    active = 1.0
                else:
                    active = 0.0

                bounds.addPoint(x, y, z)

                grid_data.append(x)
                grid_data.append(y)
                grid_data.append(z)
                grid_data.append(active)
                index += 1

    print(bounds)

    return nx, ny, nz, grid_data, bounds,grid


def loadKWData(path, keyword, ecl_type=EclTypeEnum.ECL_FLOAT_TYPE):
    kw_data = loadKW(keyword, ecl_type, path)

    print(kw_data.min, kw_data.max)

    min_value = kw_data.min
    data_range = kw_data.max - kw_data.min

    #result = (value - min) / range
    result = []
    for value in kw_data:
        value = float(value - min_value) / data_range
        result.append(value)

    return result, data_range


def rgb(r, g, b):
    return [r / 255.0, g / 255.0, b / 255.0, 1.0]

def createColorBrewerScale():
    color_list = [rgb(141,211,199),
                  rgb(255,255,179),
                  rgb(190,186,218),
                  rgb(251,128,114),
                  rgb(128,177,211),
                  rgb(253,180,98),
                  rgb(179,222,105),
                  rgb(252,205,229),
                  rgb(217,217,217),
                  rgb(188,128,189),
                  rgb(204,235,197),
                  rgb(255,237,111)]

    colors = [component for color in color_list for component in color]

    return Texture1D(len(colors) / 4, colors)

def createSeismicScale():
    color_list = [rgb(0, 0, 255), rgb(255, 255, 255), rgb(255, 0, 0)]
    colors = [component for color in color_list for component in color]

    return Texture1D(len(colors) / 4, colors, wrap=GL_CLAMP_TO_EDGE)

def createLinearGreyScale():
    color_list = [rgb(128, 128, 128), rgb(255, 255, 255)]
    colors = [component for color in color_list for component in color]

    return Texture1D(len(colors) / 4, colors, wrap=GL_CLAMP_TO_EDGE)

def createRainbowScale():
    color_list = [rgb(200, 0, 255), rgb(0, 0, 255), rgb(0, 255, 0), rgb(255, 255, 0), rgb(255, 127, 0), rgb(255, 0, 0)]
    colors = [component for color in color_list for component in color]

    return Texture1D(len(colors) / 4, colors, wrap=GL_CLAMP_TO_EDGE, internal_format=GL_RGBA8)

def createColorScales():
    return {
        "region_colors": createColorBrewerScale(),
        "seismic": createSeismicScale(),
        "linear_grey": createLinearGreyScale(),
        "rainbow": createRainbowScale()
    }


def loadFaults(grid , fault_file):
    faults = FaultCollection( grid )
    faults.load( fault_file )        
    try:
        faults.load( fault_file )        
    except Exception as e:
        traceback.print_tb(e)
        print("Loading from fault file:%s failed" % fault_file)

    return faults
        
    

def createDataStructures(grid_path=None, grid_data_path=None , polyline_root_path = None):
    if grid_path is not None:
        nx, ny, nz, grid_data, bounds, grid = loadGridData(grid_path)
        data, data_range = loadKWData(grid_data_path, "REGIONS", ecl_type=EclTypeEnum.ECL_INT_TYPE)
        #faults = loadFaults( grid , os.path.join(polyline_root_path , "faults.grdecl"))
    else:
        # nx, ny, nz, grid_data, bounds = loadGridData("/Volumes/Statoil/data/faultregion/grid.grdecl")
        # data, data_range = loadKWData("/Volumes/Statoil/data/faultregion/fltblck.grdecl", "FLTBLCK", ecl_type=EclTypeEnum.ECL_INT_TYPE)

        nx, ny, nz, grid_data, bounds, grid = loadGridData("/Volumes/Statoil/data/TestCase/eclipse/include/example_grid_sim.GRDECL")
        data, data_range = loadKWData("/Volumes/Statoil/data/TestCase/eclipse/include/example_permx.GRDECL", "PERMX", ecl_type=EclTypeEnum.ECL_FLOAT_TYPE)
        faults = loadFaults( grid , os.path.join("/Volumes/Statoil/data/TestCase/eclipse/include" , "example_faults_sim.GRDECL"))

        


    grid_texture = Texture3D(nx, ny, nz, grid_data, GL_RGBA32F, GL_RGBA)
    attribute_texture = Texture3D(nx, ny, nz, data)


    textures = {"grid": grid_texture,
                "grid_data": attribute_texture}

    faults = None
    return textures, bounds, nx, ny, nz, data_range , faults


def readPolylines(root_path):
    polyline_files = ["pol1.xyz",
                      "pol2.xyz",
                      "pol3.xyz",
                      "pol4.xyz",
                      "pol5.xyz",
                      "pol6.xyz",
                      "pol7.xyz",
                      "pol8.xyz",
                      "pol9.xyz",
                      "pol10.xyz",
                      "pol11.xyz"]

    polylines = []

    if root_path is not None and os.path.exists(root_path):
        for polyline_file in polyline_files:
            path = os.path.join(root_path, polyline_file)
            polyline = XYZIo.readXYZFile(path)
            polylines.append(polyline)

    return polylines

if __name__ == '__main__':

    grid_path = None
    grid_data_path = None
    polyline_root_path = None

    grid_path = "/d/proj/bg/enkf/ErtTestData/ECLIPSE/Mariner2/input/grids/maureen.grid.42"
    grid_data_path = "/d/proj/bg/enkf/ErtTestData/ECLIPSE/Mariner2/output/maureen.regions"
    

    if len(sys.argv) == 4:
        grid_path = sys.argv[1]
        grid_data_path = sys.argv[2]
        polyline_root_path = sys.argv[3]
        
    app = QApplication(["Slice Viewer"])
    window = QMainWindow()
    window.resize(1024, 768)

    textures, bounds, nx, ny, nz, data_range , faults = createDataStructures(grid_path, grid_data_path , polyline_root_path)


    polylines = readPolylines(root_path=polyline_root_path)

    color_scales = createColorScales()
    textures["color_scale"] = color_scales[color_scales.keys()[0]]

    viewer = SliceViewer(textures=textures, volume_bounds=bounds, color_scales=color_scales, data_range=data_range, polylines=polylines , faults = faults)
    viewer.setSliceSize(width=nx, height=ny)

    slice_settings = SliceSettingsWidget(max_slice_count=nz, color_scales=color_scales.keys())
    slice_settings.inactiveCellsHidden.connect(viewer.hideInactiveCells)
    slice_settings.currentSliceChanged.connect(viewer.setCurrentSlice)

    slice_settings.toggleOrthographicProjection.connect(viewer.useOrthographicProjection)

    slice_settings.toggleLighting.connect(viewer.useLighting)
    slice_settings.colorScalesChanged.connect(viewer.changeColorScale)
    slice_settings.regionToggling.connect(viewer.useRegionScaling)
    slice_settings.toggleInterpolation.connect(viewer.useInterpolationOnData)
    slice_settings.mirrorX.connect(viewer.mirrorX)
    slice_settings.mirrorY.connect(viewer.mirrorY)
    slice_settings.mirrorZ.connect(viewer.mirrorZ)
    slice_settings.toggleFlatPolylines.connect(viewer.toggleFlatPolylines)


    dock_widget = QDockWidget("Settings")
    dock_widget.setObjectName("SliceSettingsDock")
    dock_widget.setWidget(slice_settings)
    dock_widget.setAllowedAreas(Qt.AllDockWidgetAreas)
    dock_widget.setFeatures(QDockWidget.NoDockWidgetFeatures)

    window.addDockWidget(Qt.LeftDockWidgetArea, dock_widget)


    window.setCentralWidget(viewer)

    window.show()
    window.activateWindow()
    window.raise_()
    app.exec_()
