import rips.Case # Circular import of Case, which already imports View. Use full name.
from rips.Commands import Commands
from rips.PdmObject import PdmObject

class View (PdmObject):
    """ResInsight view class

    Attributes:
        id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pdm_object):
        self.id = pdm_object.get_value("ViewId")

        PdmObject.__init__(self, pdm_object.pb2_object, pdm_object.channel)

    def show_grid_box(self):
        """Check if the grid box is meant to be shown in the view"""
        return self.get_value("ShowGridBox")

    def set_show_grid_box(self, value):
        """Set if the grid box is meant to be shown in the view"""
        self.set_value("ShowGridBox", value)

    def background_color(self):
        """Get the current background color in the view"""
        return self.get_value("ViewBackgroundColor")

    def set_background_color(self, bgcolor):
        """Set the background color in the view"""
        self.set_value("ViewBackgroundColor", bgcolor)

    def set_cell_result(self):
        """Retrieve the current cell results"""
        return self.children("GridCellResult")[0]

    def apply_cell_result(self, result_type, result_variable):
        """Apply a regular cell result
        
        Arguments:
            result_type (str): String representing the result category. The valid values are
                - DYNAMIC_NATIVE
                - STATIC_NATIVE
                - SOURSIMRL
                - GENERATED
                - INPUT_PROPERTY
                - FORMATION_NAMES
                - FLOW_DIAGNOSTICS
                - INJECTION_FLOODING
            result_variable (str): String representing the result variable.
        """
        cell_result = self.set_cell_result()
        cell_result.set_value("ResultType", result_type)
        cell_result.set_value("ResultVariable", result_variable)
        cell_result.update()

    def apply_flow_diagnostics_cell_result(self,
                                       result_variable  = 'TOF',
                                       selection_mode   = 'FLOW_TR_BY_SELECTION',
                                       injectors = [],
                                       producers = []):
        """Apply a flow diagnostics cell result

        Arguments:
            result_variable (str): String representing the result value
                The valid values are 'TOF', 'Fraction', 'MaxFractionTracer' and 'Communication'.
            selection_mode (str): String specifying which tracers to select.
                The valid values are
                - FLOW_TR_INJ_AND_PROD (all injector and producer tracers), 
                - FLOW_TR_PRODUCERS (all producers)
                - FLOW_TR_INJECTORS (all injectors),
                - FLOW_TR_BY_SELECTION (specify individual tracers in the
                injectors and producers variables)
            injectors (list): List of injector names (strings) to select.
                Requires selection_mode to be 'FLOW_TR_BY_SELECTION'.
            producers (list): List of producer tracers (strings) to select.
                Requires selection_mode to be 'FLOW_TR_BY_SELECTION'.
        """
        cell_result = self.set_cell_result()
        cell_result.set_value("ResultType", "FLOW_DIAGNOSTICS")
        cell_result.set_value("ResultVariable", result_variable)
        cell_result.set_value("FlowTracerSelectionMode", selection_mode)
        if selection_mode == 'FLOW_TR_BY_SELECTION':
            cell_result.set_value("SelectedInjectorTracers", injectors)
            cell_result.set_value("SelectedProducerTracers", producers)
        cell_result.update()

    def case(self):
        """Get the case the view belongs to"""
        pdm_case = self.ancestor("EclipseCase")
        if pdm_case is None:
            pdm_case = self.ancestor("ResInsightGeoMechCase")
        if pdm_case is None:
            return None
        return rips.Case(self.channel, pdm_case.get_value("CaseId"))

    def clone(self):
        """Clone the current view"""
        view_id =  Commands(self.channel).clone_view(self.id)
        return self.case().view(view_id)
