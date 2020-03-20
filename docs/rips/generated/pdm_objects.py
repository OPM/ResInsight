class PdmObject:
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        if PdmObject.__custom_init__ is not None:
            PdmObject.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class Case(PdmObject):
    """
    The ResInsight base class for Cases

    Attributes:
        file_path (str): Case File Name
        id (int): Case ID
        name (str): Case Name
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.file_path = ""
        self.id = -1
        self.name = ""
        PdmObject.__init__(self, pb2_object, channel)
        if Case.__custom_init__ is not None:
            Case.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class Reservoir(Case):
    """
    Abtract base class for Eclipse Cases

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        Case.__init__(self, pb2_object, channel)
        if Reservoir.__custom_init__ is not None:
            Reservoir.__custom_init__(self, pb2_object=pb2_object, channel=channel)

    def views(self):
        """All Eclipse Views in the case
        Returns:
             List of EclipseView
        """
        return self.children("Views", EclipseView)


class EclipseCase(Reservoir):
    """
    The Regular Eclipse Results Case

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        Reservoir.__init__(self, pb2_object, channel)
        if EclipseCase.__custom_init__ is not None:
            EclipseCase.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class SummaryCase(PdmObject):
    """
    The Base Class for all Summary Cases

    Attributes:
        auto_shorty_name (str): Use Auto Display Name
        short_name (str): Display Name
        summary_header_filename (str): Summary Header File
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.auto_shorty_name = False
        self.short_name = "Display Name"
        self.summary_header_filename = ""
        PdmObject.__init__(self, pb2_object, channel)
        if SummaryCase.__custom_init__ is not None:
            SummaryCase.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class FileSummaryCase(SummaryCase):
    """
    A Summary Case based on SMSPEC files

    Attributes:
        include_restart_files (str): Include Restart Files
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.include_restart_files = False
        SummaryCase.__init__(self, pb2_object, channel)
        if FileSummaryCase.__custom_init__ is not None:
            FileSummaryCase.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class ViewWindow(PdmObject):
    """
    The Base Class for all Views and Plots in ResInsight

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        PdmObject.__init__(self, pb2_object, channel)
        if ViewWindow.__custom_init__ is not None:
            ViewWindow.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class View(ViewWindow):
    """
    Attributes:
        background_color (str): Background
        current_time_step (int): Current Time Step
        disable_lighting (str): Disable Results Lighting
        grid_z_scale (float): Z Scale
        id (int): View ID
        perspective_projection (str): Perspective Projection
        show_grid_box (str): Show Grid Box
        show_z_scale (str): Show Z Scale Label
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.background_color = "#b0c4de"
        self.current_time_step = 0
        self.disable_lighting = False
        self.grid_z_scale = 5
        self.id = -1
        self.perspective_projection = True
        self.show_grid_box = True
        self.show_z_scale = True
        ViewWindow.__init__(self, pb2_object, channel)
        if View.__custom_init__ is not None:
            View.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class GeoMechView(View):
    """
    The Geomechanical 3d View

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        View.__init__(self, pb2_object, channel)
        if GeoMechView.__custom_init__ is not None:
            GeoMechView.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class GridSummaryCase(SummaryCase):
    """
    A Summary Case based on extracting grid data.

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        SummaryCase.__init__(self, pb2_object, channel)
        if GridSummaryCase.__custom_init__ is not None:
            GridSummaryCase.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class WellPath(PdmObject):
    """
    The Base class for Well Paths

    Attributes:
        name (str): Name
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.name = ""
        PdmObject.__init__(self, pb2_object, channel)
        if WellPath.__custom_init__ is not None:
            WellPath.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class ModeledWellPath(WellPath):
    """
    A Well Path created interactively in ResInsight

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        WellPath.__init__(self, pb2_object, channel)
        if ModeledWellPath.__custom_init__ is not None:
            ModeledWellPath.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class GeoMechCase(Case):
    """
    The Abaqus Based GeoMech Case

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        Case.__init__(self, pb2_object, channel)
        if GeoMechCase.__custom_init__ is not None:
            GeoMechCase.__custom_init__(self, pb2_object=pb2_object, channel=channel)

    def views(self):
        """All GeoMech Views in the Case
        Returns:
             List of GeoMechView
        """
        return self.children("Views", GeoMechView)


class Project(PdmObject):
    """
    The ResInsight Project

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        PdmObject.__init__(self, pb2_object, channel)
        if Project.__custom_init__ is not None:
            Project.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class EclipseView(View):
    """
    The Eclipse 3d Reservoir View

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        View.__init__(self, pb2_object, channel)
        if EclipseView.__custom_init__ is not None:
            EclipseView.__custom_init__(self, pb2_object=pb2_object, channel=channel)

    def cell_result(self):
        """Cell Result
        Returns:
             CellColors
        """
        children = self.children("CellResult", CellColors)
        return children[0] if len(children) > 0 else None


    def cell_result_data(self):
        """Current Eclipse Cell Result
        Returns:
             str
        """
        return self._call_get_method("CellResultData")


    def set_cell_result_data(self, values):
        """Set Current Eclipse Cell Result
        Arguments:
            values (str): data
        """
        self._call_set_method("CellResultData", values)


class EclipseResult(PdmObject):
    """
    An eclipse result definition

    Attributes:
        flow_tracer_selection_mode (str): Tracers
        phase_selection (str): Phases
        porosity_model_type (str): Porosity
        result_type (str): Type
        result_variable (str): Variable
        selected_injector_tracers (str): Injector Tracers
        selected_producer_tracers (str): Producer Tracers
        selected_souring_tracers (str): Tracers
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.flow_tracer_selection_mode = "FLOW_TR_INJ_AND_PROD"
        self.phase_selection = "PHASE_ALL"
        self.porosity_model_type = "MATRIX_MODEL"
        self.result_type = "DYNAMIC_NATIVE"
        self.result_variable = "None"
        self.selected_injector_tracers = []
        self.selected_producer_tracers = []
        self.selected_souring_tracers = []
        PdmObject.__init__(self, pb2_object, channel)
        if EclipseResult.__custom_init__ is not None:
            EclipseResult.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class CellColors(EclipseResult):
    """
    Eclipse Cell Colors class

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        EclipseResult.__init__(self, pb2_object, channel)
        if CellColors.__custom_init__ is not None:
            CellColors.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class EclipseContourMap(EclipseView):
    """
    A contour map for Eclipse cases

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        EclipseView.__init__(self, pb2_object, channel)
        if EclipseContourMap.__custom_init__ is not None:
            EclipseContourMap.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class GeoMechContourMap(GeoMechView):
    """
    A contour map for GeoMech cases

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        GeoMechView.__init__(self, pb2_object, channel)
        if GeoMechContourMap.__custom_init__ is not None:
            GeoMechContourMap.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class GridCaseGroup(PdmObject):
    """
    A statistics case group

    Attributes:
        group_id (int): Case Group ID
        user_description (str): Name
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.group_id = -1
        self.user_description = "Grid Case Group"
        PdmObject.__init__(self, pb2_object, channel)
        if GridCaseGroup.__custom_init__ is not None:
            GridCaseGroup.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class PlotWindow(ViewWindow):
    """
    The Abstract base class for all MDI Windows in the Plot Window

    Attributes:
        id (int): View ID
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.id = -1
        ViewWindow.__init__(self, pb2_object, channel)
        if PlotWindow.__custom_init__ is not None:
            PlotWindow.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class Plot(PlotWindow):
    """
    The Abstract Base Class for all Plot Objects

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        PlotWindow.__init__(self, pb2_object, channel)
        if Plot.__custom_init__ is not None:
            Plot.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class SummaryPlot(Plot):
    """
    A Summary Plot

    Attributes:
        is_using_auto_name (str): Auto Title
        normalize_curve_y_values (str): Normalize all curves
        plot_description (str): Name
        show_plot_title (str): Plot Title
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.is_using_auto_name = True
        self.normalize_curve_y_values = False
        self.plot_description = "Summary Plot"
        self.show_plot_title = True
        Plot.__init__(self, pb2_object, channel)
        if SummaryPlot.__custom_init__ is not None:
            SummaryPlot.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class SummaryPlotCollection(PdmObject):
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        PdmObject.__init__(self, pb2_object, channel)
        if SummaryPlotCollection.__custom_init__ is not None:
            SummaryPlotCollection.__custom_init__(self, pb2_object=pb2_object, channel=channel)

    def new_summary_plot(self, summary_cases=[], ensemble=None, address=None):
        """
        Create a new Summary Plot
        Arguments:
            summary_cases (List of SummaryCase): Summary Cases
            ensemble (SummaryCaseSubCollection): Ensemble
            address (str): Formatted address string specifying the plot options
        Returns:
            SummaryPlot
        """
        return self._call_pdm_method("NewSummaryPlot", summary_cases=summary_cases, ensemble=ensemble, address=address)


class WbsParameters(PdmObject):
    """
    Attributes:
        df_source (str): Depletion Factor (DF)
        fg_multiplier (float): SH Multiplier for FG in Shale
        fg_shale_source (str): FG in Shale Calculation
        k0_fg_source (str): K0_FG
        k0_sh_source (str): K0_SH
        obg0_source (str): Initial Overburden Gradient
        poission_ratio_source (str): Poisson Ratio
        pore_pressure_non_reservoir_source (str): Non-Reservoir Pore Pressure
        pore_pressure_reservoir_source (str): Reservoir Pore Pressure
        ucs_source (str): Uniaxial Compressive Strength
        user_df (float): User Defined DF
        user_k0_fg (float): User Defined K0_FG
        user_k0_sh (float): User Defined K0_SH
        user_poisson_ratio (float): User Defined Poisson Ratio
        user_pp_non_reservoir (float):   Multiplier of hydrostatic PP
        user_ucs (float): User Defined UCS [bar]
        water_density (float): Density of Sea Water [g/cm^3]
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.df_source = "LAS_FILE"
        self.fg_multiplier = 1.05
        self.fg_shale_source = "DERIVED_FROM_K0FG"
        self.k0_fg_source = "LAS_FILE"
        self.k0_sh_source = "LAS_FILE"
        self.obg0_source = "GRID"
        self.poission_ratio_source = "LAS_FILE"
        self.pore_pressure_non_reservoir_source = "LAS_FILE"
        self.pore_pressure_reservoir_source = "GRID"
        self.ucs_source = "LAS_FILE"
        self.user_df = 0.7
        self.user_k0_fg = 0.75
        self.user_k0_sh = 0.65
        self.user_poisson_ratio = 0.35
        self.user_pp_non_reservoir = 1.05
        self.user_ucs = 100
        self.water_density = 1.03
        PdmObject.__init__(self, pb2_object, channel)
        if WbsParameters.__custom_init__ is not None:
            WbsParameters.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class SimulationWell(PdmObject):
    """
    An Eclipse Simulation Well

    Attributes:
        name (str): Name
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.name = ""
        PdmObject.__init__(self, pb2_object, channel)
        if SimulationWell.__custom_init__ is not None:
            SimulationWell.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class WellLogPlot(PlotWindow):
    """
    A Well Log Plot With a shared Depth Axis and Multiple Tracks

    Attributes:
        auto_scale_depth_enabled (str): Auto Scale
        depth_type (str): Type
        depth_unit (str): Unit
        maximum_depth (float): Max
        minimum_depth (float): Min
        show_depth_grid_lines (str): Show Grid Lines
        show_title_in_plot (str): Show Title
    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        self.auto_scale_depth_enabled = True
        self.depth_type = "MEASURED_DEPTH"
        self.depth_unit = "UNIT_METER"
        self.maximum_depth = 1000
        self.minimum_depth = 0
        self.show_depth_grid_lines = "GRID_X_MAJOR"
        self.show_title_in_plot = False
        PlotWindow.__init__(self, pb2_object, channel)
        if WellLogPlot.__custom_init__ is not None:
            WellLogPlot.__custom_init__(self, pb2_object=pb2_object, channel=channel)

class WellBoreStabilityPlot(WellLogPlot):
    """
    A GeoMechanical Well Bore Stabilit Plot

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        WellLogPlot.__init__(self, pb2_object, channel)
        if WellBoreStabilityPlot.__custom_init__ is not None:
            WellBoreStabilityPlot.__custom_init__(self, pb2_object=pb2_object, channel=channel)

    def parameters(self):
        """Well Bore Stability Parameters
        Returns:
             WbsParameters
        """
        children = self.children("Parameters", WbsParameters)
        return children[0] if len(children) > 0 else None


class FileWellPath(WellPath):
    """
    Well Paths Loaded From File

    """
    __custom_init__ = None #: Assign a custom init routine to be run at __init__

    def __init__(self, pb2_object=None, channel=None):
        WellPath.__init__(self, pb2_object, channel)
        if FileWellPath.__custom_init__ is not None:
            FileWellPath.__custom_init__(self, pb2_object=pb2_object, channel=channel)

def class_dict():
    classes = {}
    classes['Case'] = Case
    classes['CellColors'] = CellColors
    classes['EclipseCase'] = EclipseCase
    classes['EclipseContourMap'] = EclipseContourMap
    classes['EclipseResult'] = EclipseResult
    classes['EclipseView'] = EclipseView
    classes['FileSummaryCase'] = FileSummaryCase
    classes['FileWellPath'] = FileWellPath
    classes['GeoMechCase'] = GeoMechCase
    classes['GeoMechContourMap'] = GeoMechContourMap
    classes['GeoMechView'] = GeoMechView
    classes['GridCaseGroup'] = GridCaseGroup
    classes['GridSummaryCase'] = GridSummaryCase
    classes['ModeledWellPath'] = ModeledWellPath
    classes['PdmObject'] = PdmObject
    classes['Plot'] = Plot
    classes['PlotWindow'] = PlotWindow
    classes['Project'] = Project
    classes['Reservoir'] = Reservoir
    classes['SimulationWell'] = SimulationWell
    classes['SummaryCase'] = SummaryCase
    classes['SummaryPlot'] = SummaryPlot
    classes['SummaryPlotCollection'] = SummaryPlotCollection
    classes['View'] = View
    classes['ViewWindow'] = ViewWindow
    classes['WbsParameters'] = WbsParameters
    classes['WellBoreStabilityPlot'] = WellBoreStabilityPlot
    classes['WellLogPlot'] = WellLogPlot
    classes['WellPath'] = WellPath
    return classes

def class_from_keyword(class_keyword):
    all_classes = class_dict()
    if class_keyword in all_classes.keys():
        return all_classes[class_keyword]
    return None
