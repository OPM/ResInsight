import rips.Case
from rips.Commands import Commands
from rips.PdmObject import PdmObject

class View (PdmObject):
    """ResInsight view class

    Attributes:
        id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pbmObject):
        self.id = pbmObject.getValue("ViewId")

        PdmObject.__init__(self, pbmObject.pb2Object, pbmObject.channel)

    def showGridBox(self):
        """Check if the grid box is meant to be shown in the view"""
        return self.getValue("ShowGridBox")

    def setShowGridBox(self, value):
        """Set if the grid box is meant to be shown in the view"""
        self.setValue("ShowGridBox", value)

    def backgroundColor(self):
        """Get the current background color in the view"""
        return self.getValue("ViewBackgroundColor")

    def setBackgroundColor(self, bgColor):
        """Set the background color in the view"""
        self.setValue("ViewBackgroundColor", bgColor)

    def cellResult(self):
        """Retrieve the current cell results"""
        return self.children("GridCellResult")[0]

    def applyCellResult(self, resultType, resultVariable):
        """Apply a regular cell result
        Arguments:
            resultType [str]: String representing the result category
                The valid values are: "DYNAMIC_NATIVE", "STATIC_NATIVE", "SOURSIMRL",
                                      "GENERATED", "INPUT_PROPERTY", "FORMATION_NAMES",
                                      "FLOW_DIAGNOSTICS" and "INJECTION_FLOODING"
            resultVariable [str]: String representing the result value.
        """
        cellResult = self.cellResult()
        cellResult.setValue("ResultType", resultType)
        cellResult.setValue("ResultVariable", resultVariable)
        cellResult.update()

    def applyFlowDiagnosticsCellResult(self,
                                       resultVariable  = 'TOF',
                                       selectionMode   = 'FLOW_TR_BY_SELECTION',
                                       injectors = [],
                                       producers = []):
        """Apply a flow diagnostics cell result

        Arguments:
            resultVariable [str]: String representing the result value
                The valid values are 'TOF', 'Fraction', 'MaxFractionTracer' and 'Communication'.
            selectionMode [str]: String specifying which tracers to select.
                The valid values are FLOW_TR_INJ_AND_PROD (all injector and producer tracers)
                                     FLOW_TR_PRODUCERS (all producers)
                                     FLOW_TR_INJECTORS (all injectors)
                                     FLOW_TR_BY_SELECTION (specify individual tracers in the
                                        injectorTracers and producerTracers variables)
            injectorTracers [list]: List of injector names (strings) to select.
                Requires selectionMode to be 'FLOW_TR_BY_SELECTION'.
            producerTracers [list]: List of producer tracers (strings) to select.
                Requires selectionMode to be 'FLOW_TR_BY_SELECTION'.
        """
        cellResult = self.cellResult()
        cellResult.setValue("ResultType", "FLOW_DIAGNOSTICS")
        cellResult.setValue("ResultVariable", resultVariable)
        cellResult.setValue("FlowTracerSelectionMode", selectionMode)
        if selectionMode == 'FLOW_TR_BY_SELECTION':
            cellResult.setValue("SelectedInjectorTracers", injectors)
            cellResult.setValue("SelectedProducerTracers", producers)
        cellResult.update()

    def case(self):
        """Get the case the view belongs to"""
        pdmCase = self.ancestor("EclipseCase")
        if pdmCase is None:
            pdmCase = self.ancestor("ResInsightGeoMechCase")
        if pdmCase is None:
            return None
        return rips.Case(self.channel, pdmCase.getValue("CaseId"))

    def clone(self):
        """Clone the current view"""
        viewId =  Commands(self.channel).cloneView(self.id)
        return self.case().view(viewId)