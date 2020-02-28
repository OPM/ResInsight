"""
ResInsight Well Bore Stability Plot module
"""

from rips.pdmobject import PdmObject, add_method
from rips.well_log_plot import WellLogPlot
from rips.generated.pdm_objects import WellBoreStabilityPlot, WbsParameters

@add_method(WbsParameters)
def __custom_init__(self, pb2_object=None, channel=None):
    self.pore_pressure_reservoir_source     = "UNDEFINED"
    self.pore_pressure_non_reservoir_source = "UNDEFINED"
        
    self.poisson_ratio_source = "UNDEFINED"
    self.ucs_source           = "UNDEFINED"
    self.obg0_source          = "UNDEFINED"
    self.df_source            = "UNDEFINED"
    self.k0sh_source          = "UNDEFINED"
    self.fg_shale_source      = "UNDEFINED"
    self.k0fg_source          = "UNDEFINED"

    self.user_pp_non_reservoir = 1.05
    self.user_poission_ratio   = 0.35
    self.user_ucs              = 100
    self.user_df               = 0.7
    self.user_k0sh             = 0.65
    self.fg_multiplier         = 1.05
    self.user_k0fg             = 0.75

@add_method(WellBoreStabilityPlot)
def parameters(self):
    """Retrieve the parameters of the Plot
    """
    children = self.children("WbsParameters", WbsParameters)
    if len(children) == 1:
        child = children[0]
        return child
    return None

@add_method(WellBoreStabilityPlot)
def set_parameters(self, wbs_parameters):
    children = self.children("WbsParameters", WbsParameters)
    if len(children) == 1:
        pdm_params = children[0]
        pdm_params.copy_from(wbs_parameters)
