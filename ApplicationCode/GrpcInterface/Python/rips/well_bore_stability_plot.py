"""
ResInsight Well Bore Stability Plot module
"""

from rips.pdmobject import PdmObject
from rips.well_log_plot import WellLogPlot

class WbsParameters:
    """Well Bore Stability parameters
    
    Note that any parameter sources left at "UNDEFINED" will get ResInsight defaults
    which depends on the available data.

    Attributes:
        pore_pressure_in_reservoir_source(enum string): can be "GRID", "LAS_FILE" and "ELEMENT_PROPERTY_TABLE".
        pore_pressure_outside_reservoir_source(enum string): can be "HYDROSTATIC" and "USER_DEFINED".
        poisson_ratio_source(enum string): can be "LAS_FILE", "ELEMENT_PROPERTY_TABLE" or "USER_DEFINED".
        ucs_source(enum string): can be "LAS_FILE", "ELEMENT_PROPERTY_TABLE" or "USER_DEFINED".
        obg0_source(enum string): can be "GRID" or "LAS_FILE".
        df_source(enum string): can be "LAS_FILE", "ELEMENT_PROPERTY_TABLE" or "USER_DEFINED".
        fg_shale_source(enum string): can be "DERIVED_FROM_K0FG" or "PROPORTIONAL_TO_SH".
        k0fg_source(enum string): can be "LAS_FILE" or "USER_DEFINED"". Only relevant if fg_shale_source is "DERIVED_FROM_K0FG".

        user_pp_outside_reservoir(double): Used if pore_pressure_outside_reservoir_source is "USED_DEFINED". Default 1.05.
        user_poission_ratio(double): Used if poisson_ratio_source is "USER_DEFINED", default 0.35.
        user_ucs(double): Used if ucs_soruce is "USER_DEFINED", default 100.
        user_df(double): Used if df is "USER_DEFINED", default 0.7.
        user_k0sh(double): Used if k0sh_source is "USER_DEFINED", default 0.65.
        fg_shale_sh_multiplier(double): Used if fg_shale_source is "PROPORTIONAL_TO_SH", default 1.05.
        user_k0fg(double): Used if fg_shale_source is "DERIVED_FROM_K0FG" and k0fg_source is "USER_DEFINED", default 0.75.		
        
    """

    def __init__(self):
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

class WellBoreStabilityPlot(WellLogPlot):
    """ResInsight Well Bore Stability Plot
    """

    def __init__(self, pdm_object):
        WellLogPlot.__init__(self, pdm_object)

    @classmethod
    def from_pdm_object(cls, pdm_object):
        if isinstance(pdm_object, PdmObject):
            if pdm_object.class_keyword() == "WellBoreStabilityPlot":
                return cls(pdm_object)
        return None 

    def parameters(self):
        """Retrieve the parameters of the Plot
        """
        children = self.children("WbsParameters")
        if len(children) == 1:
            child = children[0]
            return child
        return None

    def set_parameters(self, wbs_parameters):
        children = self.children("WbsParameters")
        if len(children) == 1:
            pdm_params = children[0]
            pdm_params.copy_from(wbs_parameters)
