import rips.Case # Circular import of Case, which already imports View. Use full name.
from rips.Commands import Commands
from rips.PdmObject import PdmObject

class View (PdmObject):
    """ResInsight view class

    Attributes:
        id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pbmObject):
        self.id = pbmObject.get_value("ViewId")

        PdmObject.__init__(self, pbmObject.pb2Object, pbmObject.channel)

    def showGridBox(self):
        """Check if the grid box is meant to be shown in the view"""
        return self.get_value("ShowGridBox")

    def setShowGridBox(self, value):
        """Set if the grid box is meant to be shown in the view"""
        self.set_value("ShowGridBox", value)

    def backgroundColor(self):
        """Get the current background color in the view"""
        return self.get_value("ViewBackgroundColor")

    def setBackgroundColor(self, bgColor):
        """Set the background color in the view"""
        self.set_value("ViewBackgroundColor", bgColor)

    def cellResult(self):
        """Retrieve the current cell results"""
        return self.children("GridCellResult")[0]

    def applyCellResult(self, resultType, resultVariable):
        """Apply a regular cell result
        
        Arguments:
            resultType (str): String representing the result category. The valid values are
                - DYNAMIC_NATIVE
                - STATIC_NATIVE
                - SOURSIMRL
                - GENERATED
                - INPUT_PROPERTY
                - FORMATION_NAMES
                - FLOW_DIAGNOSTICS
                - INJECTION_FLOODING
            resultVariable (str): String representing the result variable.
        """
        cellResult = self.cellResult()
        cellResult.set_value("ResultType", resultType)
        cellResult.set_value("ResultVariable", resultVariable)
        cellResult.update()

    def applyFlowDiagnosticsCellResult(self,
                                       resultVariable  = 'TOF',
                                       selectionMode   = 'FLOW_TR_BY_SELECTION',
                                       injectors = [],
                                       producers = []):
        """Apply a flow diagnostics cell result

        Arguments:
            resultVariable (str): String representing the result value
                The valid values are 'TOF', 'Fraction', 'MaxFractionTracer' and 'Communication'.
            selectionMode (str): String specifying which tracers to select.
                The valid values are
                - FLOW_TR_INJ_AND_PROD (all injector and producer tracers), 
                - FLOW_TR_PRODUCERS (all producers)
                - FLOW_TR_INJECTORS (all injectors),
                - FLOW_TR_BY_SELECTION (specify individual tracers in the
                injectorTracers and producerTracers variables)
            injectorTracers (list): List of injector names (strings) to select.
                Requires selectionMode to be 'FLOW_TR_BY_SELECTION'.
            producerTracers (list): List of producer tracers (strings) to select.
                Requires selectionMode to be 'FLOW_TR_BY_SELECTION'.
        """
        cellResult = self.cellResult()
        cellResult.set_value("ResultType", "FLOW_DIAGNOSTICS")
        cellResult.set_value("ResultVariable", resultVariable)
        cellResult.set_value("FlowTracerSelectionMode", selectionMode)
        if selectionMode == 'FLOW_TR_BY_SELECTION':
            cellResult.set_value("SelectedInjectorTracers", injectors)
            cellResult.set_value("SelectedProducerTracers", producers)
        cellResult.update()

    def case(self):
        """Get the case the view belongs to"""
        pdmCase = self.ancestor("EclipseCase")
        if pdmCase is None:
            pdmCase = self.ancestor("ResInsightGeoMechCase")
        if pdmCase is None:
            return None
        return rips.Case(self.channel, pdmCase.get_value("CaseId"))

    def clone(self):
        """Clone the current view"""
        viewId =  Commands(self.channel).clone_view(self.id)
        return self.case().view(viewId)