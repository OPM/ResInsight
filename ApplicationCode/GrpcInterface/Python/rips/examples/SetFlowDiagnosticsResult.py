# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

view = resInsight.project.view(0)
view.applyFlowDiagnosticsCellResult(resultVariable='Fraction',
                                    selectionMode='FLOW_TR_INJ_AND_PROD')
                                    
# Example of setting individual wells. Commented out because well names are case specific.
#view.applyFlowDiagnosticsCellResult(resultVariable='Fraction',
#                                    selectionMode='FLOW_TR_BY_SELECTION',
#                                    injectors = ['C-1H', 'C-2H', 'F-2H'],
#                                    producers = ['B-1AH', 'B-3H', 'D-1H'])
