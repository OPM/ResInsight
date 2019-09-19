import grpc
import os
import sys

from rips.Case import Case
from rips.GridCaseGroup import GridCaseGroup
from rips.PdmObject import PdmObject
from rips.View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

import Commands_pb2 as Cmd
from Definitions_pb2 import Empty
import Project_pb2
import Project_pb2_grpc

class Project (PdmObject):
    """ResInsight project. Not intended to be created separately.

    Automatically created and assigned to Instance.
    """
    def __init__(self, channel):
        self.channel = channel
        self.project = Project_pb2_grpc.ProjectStub(channel)
    
        PdmObject.__init__(self, self.project.GetPdmObject(Empty()), self.channel)
    
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
        command_reply = self._execute_command(loadCase=Cmd.FilePathRequest(path=path))
        return Case(self.channel, command_reply.loadCaseResult.id)

    def selected_cases(self):
        """Get a list of all cases selected in the project tree

        Returns:
            A list of rips Case objects
        """
        case_infos = self.project.GetSelectedCases(Empty())
        cases = []
        for case_info in case_infos.data:
            cases.append(Case(self.channel, case_info.id))
        return cases

    def cases(self):
        """Get a list of all cases in the project
        
        Returns:
            A list of rips Case objects
        """
        try:
            case_infos = self.project.GetAllCases(Empty())

            cases = []
            for case_info in case_infos.data:
                cases.append(Case(self.channel, case_info.id))
            return cases
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return []
            else:
                print("ERROR: ", e)
                return []

    def case(self, id):
        """Get a specific case from the provided case Id

        Arguments:
            id(int): case id
        Returns:
            A rips Case object
        """
        try:
            case = Case(self.channel, id)
            return case
        except grpc.RpcError as e:
            return None
    
    def replace_source_cases(self, grid_list_file, case_group_id=0):
        """Replace all source cases within a case group
        
        Arguments:
            grid_list_file (str): path to file containing a list of cases
            case_group_id (int): id of the case group to replace
        
        """
        return self._execute_command(replaceSourceCases=Cmd.ReplaceSourceCasesRequest(gridListFile=grid_list_file,
                                                                               caseGroupId=case_group_id))

    def create_grid_case_group(self, case_paths):
        """Create a Grid Case Group from a list of cases

        Arguments:
            case_paths (list): list of file path strings

        Returns:
            A case group id and name
        """
        commandReply = self._execute_command(createGridCaseGroup=Cmd.CreateGridCaseGroupRequest(casePaths=case_paths))
        return self.grid_case_group(commandReply.createGridCaseGroupResult.groupId)

    def views(self):
        """Get a list of views belonging to a project"""
        pdm_objects = self.descendants("ReservoirView")
        view_list = []
        for pdm_object in pdm_objects:
            view_list.append(View(pdm_object))
        return view_list

    def view(self, id):
        """Get a particular view belonging to a case by providing view id
        Arguments:
            id(int): view id                
        
        Returns: a view object
        
        """
        views = self.views()
        for view_object in views:
            if view_object.id == id:
                return view_object
        return None

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
        return self._execute_command(exportMultiCaseSnapshot=Cmd.ExportMultiCaseRequest(gridListFile=grid_list_file))

    def export_snapshots(self, type = 'ALL', prefix=''):
        """ Export all snapshots of a given type
        
        Arguments:
            type (str): Enum string ('ALL', 'VIEWS' or 'PLOTS')
            prefix (str): Exported file name prefix
        
        """
        return self._execute_command(exportSnapshots=Cmd.ExportSnapshotsRequest(type=type,
                                                                         prefix=prefix,
                                                                         caseId=-1))

    def export_well_paths(self, well_paths=[], md_step_size=5.0):
        if isinstance(well_paths, str):
            well_paths = [well_paths]
        return self._execute_command(exportWellPaths=Cmd.ExportWellPathRequest(wellPathNames=well_paths, mdStepSize=md_step_size))

    def scale_fracture_template(self, id, half_length, height, dfactor, conductivity):
        return self._execute_command(scaleFractureTemplate=Cmd.ScaleFractureTemplateRequest(id=id,
                                                                                   halfLength=half_length,
                                                                                   height=height,
                                                                                   dFactor=dfactor,
                                                                                   conductivity=conductivity))

    def set_fracture_containment(self, id, top_layer, base_layer):
        return self._execute_command(setFractureContainment=Cmd.SetFracContainmentRequest(id=id,
                                                                                 topLayer=top_layer,
                                                                                 baseLayer=base_layer))
