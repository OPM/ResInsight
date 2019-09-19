import grpc
import os
import sys

from rips.Case import Case
from rips.Commands import Commands
from rips.GridCaseGroup import GridCaseGroup
from rips.PdmObject import PdmObject
from rips.View import View

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

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
        Commands(self.channel).open_project(path)
        return self

    def close(self):
        """Close the current project (and open new blank project)"""
        Commands(self.channel).close_project()

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

    def load_case(self, path):
        """Load a new case from the given file path

        Arguments:
            path(str): file path to case
        Returns:
            A rips Case object
        """
        return Commands(self.channel).load_case(path)

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
            if case_group.groupId == group_id:
                return case_group
        return None

    def create_grid_case_group(self, case_paths):
        """Create a new grid case group from the provided case paths
        Arguments:
            casePaths(list): a list of paths to the cases to be loaded and included in the group
        Returns:
            A new grid case group object
        """
        group_id, group_name = Commands(self.channel).create_grid_case_group(case_paths)
        return self.grid_case_group(group_id)