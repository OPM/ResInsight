# pylint: disable=too-many-arguments
# pylint: disable=no-member
"""
The ResInsight project module
"""
import grpc

from rips.case import Case
from rips.gridcasegroup import GridCaseGroup
from rips.pdmobject import PdmObject
from rips.plot import Plot
from rips.view import View

import rips.generated.Commands_pb2 as Cmd
from rips.generated.Definitions_pb2 import Empty
import rips.generated.Project_pb2_grpc as Project_pb2_grpc


class Project(PdmObject):
    """ResInsight project. Not intended to be created separately.

    Automatically created and assigned to Instance.
    """
    def __init__(self, channel):
        self._project_stub = Project_pb2_grpc.ProjectStub(channel)
        PdmObject.__init__(self, self._project_stub.GetPdmObject(Empty()),
                           channel)

    def open(self, path):
        """Open a new project from the given path

        Arguments:
            path(str): path to project file

        """
        self._execute_command(openProject=Cmd.FilePathRequest(path=path))
        return self

    def close(self):
        """Close the current project (and open new blank project)"""
        self._execute_command(closeProject=Empty())

    def load_case(self, path):
        """Load a new case from the given file path

        Arguments:
            path(str): file path to case
        Returns:
            A rips Case object
        """
        command_reply = self._execute_command(loadCase=Cmd.FilePathRequest(
            path=path))
        return Case(self._channel, command_reply.loadCaseResult.id, self)

    def selected_cases(self):
        """Get a list of all cases selected in the project tree

        Returns:
            A list of rips Case objects
        """
        case_infos = self._project_stub.GetSelectedCases(Empty())
        cases = []
        for case_info in case_infos.data:
            cases.append(Case(self._channel, case_info.id, self))
        return cases

    def cases(self):
        """Get a list of all cases in the project

        Returns:
            A list of rips Case objects
        """
        try:
            case_infos = self._project_stub.GetAllCases(Empty())

            cases = []
            for case_info in case_infos.data:
                cases.append(Case(self._channel, case_info.id, self))
            return cases
        except grpc.RpcError as rpc_error:
            if rpc_error.code() == grpc.StatusCode.NOT_FOUND:
                return []
            print("ERROR: ", rpc_error)
            return []

    def case(self, case_id):
        """Get a specific case from the provided case Id

        Arguments:
            id(int): case id
        Returns:
            A rips Case object
        """
        try:
            case = Case(self._channel, case_id, self)
            return case
        except grpc.RpcError:
            return None

    def replace_source_cases(self, grid_list_file, case_group_id=0):
        """Replace all source cases within a case group
        
        Arguments:
            grid_list_file (str): path to file containing a list of cases
            case_group_id (int): id of the case group to replace
        """
        return self._execute_command(
            replaceSourceCases=Cmd.ReplaceSourceCasesRequest(
                gridListFile=grid_list_file, caseGroupId=case_group_id))

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

    def views(self):
        """Get a list of views belonging to a project"""
        pdm_objects = self.descendants("ReservoirView")
        view_list = []
        for pdm_object in pdm_objects:
            view_list.append(View(pdm_object))
        return view_list

    def view(self, view_id):
        """Get a particular view belonging to a case by providing view id
        
        Arguments:
            view_id(int): view id
        Returns: a view object
        """
        views = self.views()
        for view_object in views:
            if view_object.view_id == view_id:
                return view_object
        return None

    def well_log_plots(self):
        """Get a list of all plots belonging to a project"""
        pdm_objects = self.descendants("WellLogPlot")
        plot_list = []
        for pdm_object in pdm_objects:
            plot_list.append(Plot(pdm_object))
        return plot_list

    def well_log_plot(self, view_id):
        """Get a particular plot by providing view id
        Arguments:
            view_id(int): view id
        Returns: a plot object
        """
        plots = self.well_log_plots()
        for plot_object in plots:
            if plot_object.view_id == view_id:
                return plot_object
        return None

    def well_paths(self):
        """Get a list of all the well path names in the project"""
        pdm_objects = self.descendants("WellPath")
        pdm_objects += self.descendants("ModelledWellPath")
        well_path_list  = []
        for pdm_object in pdm_objects:
            well_path_list.append(pdm_object.get_value("WellPathName"))
        return well_path_list

    def grid_case_groups(self):
        """Get a list of all grid case groups in the project"""
        case_groups = self.descendants("RimIdenticalGridCaseGroup")

        case_group_list = []
        for pdm_group in case_groups:
            case_group_list.append(GridCaseGroup(pdm_group))
        return case_group_list

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

    def export_multi_case_snapshots(self, grid_list_file):
        """Export snapshots for a set of cases
        
        Arguments:
            grid_list_file (str): Path to a file containing a list of grids to export snapshot for
        """
        return self._execute_command(
            exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(
                gridListFile=grid_list_file))

    def export_snapshots(self, snapshot_type='ALL', prefix=''):
        """ Export all snapshots of a given type
        
        Arguments:
            snapshot_type (str): Enum string ('ALL', 'VIEWS' or 'PLOTS')
            prefix (str): Exported file name prefix
        """
        return self._execute_command(
            exportSnapshots=Cmd.ExportSnapshotsRequest(
                type=snapshot_type, prefix=prefix, caseId=-1))

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
