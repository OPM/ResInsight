# pylint: disable=too-many-arguments
# pylint: disable=no-member
"""
The ResInsight project module
"""
import builtins
import grpc

from rips.case import Case
from rips.gridcasegroup import GridCaseGroup
from rips.pdmobject import PdmObject, add_method, add_static_method
from rips.plot import Plot
from rips.view import View

import rips.generated.Commands_pb2 as Cmd
from rips.generated.Definitions_pb2 import Empty
import rips.generated.Project_pb2_grpc as Project_pb2_grpc
import rips.generated.Project_pb2 as Project_pb2
import rips.generated.PdmObject_pb2 as PdmObject_pb2
from rips.generated.pdm_objects import Project, PlotWindow, WellPath


@add_method(Project)
def __custom_init__(self, pb2_object, channel):
    self._project_stub = Project_pb2_grpc.ProjectStub(self._channel)

@add_static_method(Project)
def create(channel):
    project_stub = Project_pb2_grpc.ProjectStub(channel)
    pb2_object = project_stub.GetPdmObject(Empty())
    return Project(pb2_object, channel)

@add_method(Project)
def open(self, path):
    """Open a new project from the given path

    Arguments:
        path(str): path to project file

    """
    self._execute_command(openProject=Cmd.FilePathRequest(path=path))
    return self
        
@add_method(Project)
def save(self, path=""):
    """Save the project to the existing project file, or to a new file
    Arguments:
        path(str): File path to the file to save the project to. If empty, saves to the active project file
    """
    self._execute_command(saveProject=Cmd.SaveProjectRequest(filePath=path))
    return self

@add_method(Project)
def close(self):
    """Close the current project (and open new blank project)"""
    self._execute_command(closeProject=Empty())

@add_method(Project)
def load_case(self, path):
    """Load a new case from the given file path

    Arguments:
        path(str): file path to case
    Returns:
        A rips Case object
    """
    command_reply = self._execute_command(loadCase=Cmd.FilePathRequest(
        path=path))
    return self.case(command_reply.loadCaseResult.id)

@add_method(Project)
def selected_cases(self):
    """Get a list of all cases selected in the project tree

    Returns:
        A list of rips Case objects
    """
    case_infos = self._project_stub.GetSelectedCases(Empty())
    cases = []
    for case_info in case_infos.data:
        cases.append(self.case(case_info.id))
    return cases

@add_method(Project)
def cases(self):
    """Get a list of all cases in the project

    Returns:
        A list of rips Case objects
    """
    return self.descendants(Case)

@add_method(Project)
def case(self, case_id):
    """Get a specific case from the provided case Id

    Arguments:
        id(int): case id
    Returns:
        A rips Case object
    """
    allCases = self.cases()
    for case in allCases:
        if case.id == case_id:
            return case
    return None

@add_method(Project)
def replace_source_cases(self, grid_list_file, case_group_id=0):
    """Replace all source cases within a case group
        
    Arguments:
        grid_list_file (str): path to file containing a list of cases
        case_group_id (int): id of the case group to replace
    """
    return self._execute_command(
        replaceSourceCases=Cmd.ReplaceSourceCasesRequest(
            gridListFile=grid_list_file, caseGroupId=case_group_id))

@add_method(Project)
def create_grid_case_group(self, case_paths):
    """Create a Grid Case Group from a list of cases
        
    Arguments:
        case_paths (list): list of file path strings
    Returns:
        A case group id and name
    """
    command_reply = self._execute_command(
        createGridCaseGroup=Cmd.CreateGridCaseGroupRequest(
            casePaths=case_paths))
    return self.grid_case_group(
        command_reply.createGridCaseGroupResult.groupId)

@add_method(Project)
def views(self):
    """Get a list of views belonging to a project"""
    return self.descendants(View)

@add_method(Project)
def view(self, view_id):
    """Get a particular view belonging to a case by providing view id
        
    Arguments:
        view_id(int): view id
    Returns: a view object
    """
    views = self.views()
    for view_object in views:
        if view_object.id == view_id:
            return view_object
    return None

@add_method(Project)
def plots(self):
    """Get a list of all plots belonging to a project"""
    pdm_objects = self.descendants(PlotWindow)
    plot_list = []
    for pdm_object in pdm_objects:
        if pdm_object.id != -1:
            plot_list.append(pdm_object)
    return plot_list

@add_method(Project)
def plot(self, view_id):
    """Get a particular plot by providing view id
    Arguments:
        view_id(int): view id
    Returns: a plot object
    """
    plots = self.plots()
    for plot_object in plots:
        if plot_object.id == view_id:
            return plot_object
    return None

@add_method(Project)
def grid_case_groups(self):
    """Get a list of all grid case groups in the project"""
    case_groups = self.descendants(GridCaseGroup)
    return case_groups

@add_method(Project)
def grid_case_group(self, group_id):
    """Get a particular grid case group belonging to a project
        
    Arguments:
        groupId(int): group id

    Returns: a grid case group object
    """
    case_groups = self.grid_case_groups()
    for case_group in case_groups:
        if case_group.group_id == group_id:
            return case_group
    return None

@add_method(Project)
def export_multi_case_snapshots(self, grid_list_file):
    """Export snapshots for a set of cases
        
    Arguments:
        grid_list_file (str): Path to a file containing a list of grids to export snapshot for
    """
    return self._execute_command(
        exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(
            gridListFile=grid_list_file))

@add_method(Project)
def export_snapshots(self, snapshot_type='ALL', prefix='', plot_format='PNG'):
    """ Export all snapshots of a given type
        
    Arguments:
        snapshot_type (str): Enum string ('ALL', 'VIEWS' or 'PLOTS')
        prefix (str): Exported file name prefix
        plot_format(str): Enum string, 'PNG' or 'PDF'
    """
    return self._execute_command(
        exportSnapshots=Cmd.ExportSnapshotsRequest(
            type=snapshot_type, prefix=prefix, caseId=-1, viewId=-1, plotOutputFormat=plot_format))

@add_method(Project)
def export_well_paths(self, well_paths=None, md_step_size=5.0):
    """ Export a set of well paths
        
    Arguments:
        well_paths(list): List of strings of well paths. If none, export all.
        md_step_size(double): resolution of the exported well path
    """
    if well_paths is None:
        well_paths = []
    elif isinstance(well_paths, str):
        well_paths = [well_paths]
    return self._execute_command(exportWellPaths=Cmd.ExportWellPathRequest(
        wellPathNames=well_paths, mdStepSize=md_step_size))

@add_method(Project)
def scale_fracture_template(self, template_id, half_length, height,
                            d_factor, conductivity):
    """ Scale fracture template parameters
        
    Arguments:
        template_id(int): ID of fracture template
        half_length (double): Half Length scale factor
        height (double): Height scale factor
        d_factor (double): D-factor scale factor
        conductivity (double): Conductivity scale factor
    """
    return self._execute_command(
        scaleFractureTemplate=Cmd.ScaleFractureTemplateRequest(
            id=template_id,
            halfLength=half_length,
            height=height,
            dFactor=d_factor,
            conductivity=conductivity))

@add_method(Project)
def set_fracture_containment(self, template_id, top_layer, base_layer):
    """ Set fracture template containment parameters
        
    Arguments:
        template_id(int): ID of fracture template
        top_layer (int): Top layer containment
        base_layer (int): Base layer containment
    """
    return self._execute_command(
        setFractureContainment=Cmd.SetFracContainmentRequest(
            id=template_id, topLayer=top_layer, baseLayer=base_layer))

@add_method(Project)
def import_well_paths(self, well_path_files=None, well_path_folder=''):
    """ Import well paths into project

    Arguments:
        well_path_files(list): List of file paths to import
        well_path_folder(str): A folder path containing files to import

    Returns:
        A list of WellPath objects
    """
    if well_path_files is None:
        well_path_files = []
        
    res = self._execute_command(importWellPaths=Cmd.ImportWellPathsRequest(wellPathFolder=well_path_folder,
                                                                            wellPathFiles=well_path_files))
    well_paths = []
    for well_path_name in res.importWellPathsResult.wellPathNames:
        well_paths.append(self.well_path_by_name(well_path_name))
    return well_paths

@add_method(Project)
def well_paths(self):
    """Get a list of all well paths in the project

    Returns:
        A list of rips WellPath objects
    """
    return self.descendants(WellPath)

@add_method(Project)
def well_path_by_name(self, well_path_name):
    """Get a specific well path by name from the project

    Returns:
        A WellPath object
    """
    all_well_paths = self.well_paths()
    for well_path in all_well_paths:
        if well_path.name == well_path_name:
            return well_path
    return None

@add_method(Project)
def import_well_log_files(self, well_log_files=None, well_log_folder=''):
    """ Import well log files into project

    Arguments:
        well_log_files(list): List of file paths to import
        well_log_folder(str): A folder path containing files to import

    Returns:
        A list of well path names (strings) that had logs imported
    """

    if well_log_files is None:
        well_log_files = []
    res = self._execute_command(importWellLogFiles=Cmd.ImportWellLogFilesRequest(wellLogFolder=well_log_folder,
                                                                                    wellLogFiles=well_log_files))
    return res.importWellLogFilesResult.wellPathNames

@add_method(Project)
def import_formation_names(self, formation_files=None):
    """ Import formation names into project

    Arguments:
        formation_files(list): list of files to import

    """
    if formation_files is None:
        formation_files = []
    elif isinstance(formation_files, str):
        formation_files = [formation_files]

    res = self._execute_command(importFormationNames=Cmd.ImportFormationNamesRequest(formationFiles=formation_files,
                                                                                        applyToCaseId=-1))
