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
		self.pore_pressure_in_reservoir_source      = "UNDEFINED"
		self.pore_pressure_outside_reservoir_source = "UNDEFINED"
		
		self.poisson_ratio_source = "UNDEFINED"
		self.ucs_source           = "UNDEFINED"
		self.obg0_source          = "UNDEFINED"
		self.df_source            = "UNDEFINED"
		self.k0sh_source          = "UNDEFINED"
		self.fg_shale_source      = "UNDEFINED"
		self.k0fg_source          = "UNDEFINED"

		self.user_pp_outside_reservoir = 1.05
		self.user_poission_ratio       = 0.35
		self.user_ucs                  = 100
		self.user_df                   = 0.7
		self.user_k0sh                 = 0.65
		self.fg_shale_multiplier       = 1.05
		self.user_k0fg                 = 0.75

	@classmethod
	def from_pdm_object(cls, pdm_object):
		params = cls()
		params.pore_pressure_in_reservoir_source      = pdm_object.get_value("PorePressureReservoirSource")
		params.pore_pressure_outside_reservoir_source = pdm_object.get_value("PorePressureNonReservoirSource")
		
		params.poisson_ratio_source = pdm_object.get_value("PoissonRatioSource")
		params.ucs_source           = pdm_object.get_value("UcsSource")
		params.obg0_source          = pdm_object.get_value("OBG0Source")
		params.df_source            = pdm_object.get_value("DFSource")
		params.k0sh_source          = pdm_object.get_value("K0SHSource")
		params.fg_shale_source      = pdm_object.get_value("FGShaleSource")
		params.k0fg_source          = pdm_object.get_value("K0FGSource")

		params.user_pp_outside_reservoir = pdm_object.get_value("UserPPNonReservoir")
		params.user_poisson_ratio        = pdm_object.get_value("UserPoissonRatio")
		params.user_ucs                  = pdm_object.get_value("UserUcs")
		params.user_df                   = pdm_object.get_value("UserDF")
		params.user_k0fg                 = pdm_object.get_value("UserK0FG")
		params.user_k0sh                 = pdm_object.get_value("UserK0SH")
		params.user_fg_shale             = pdm_object.get_value("FGMultiplier")
		return params

	def to_pdm_object(self, pdm_object):
		pdm_object.set_value("PorePressureReservoirSource", self.pore_pressure_in_reservoir_source)
		pdm_object.set_value("PorePressureNonReservoirSource", self.pore_pressure_outside_reservoir_source)
		
		pdm_object.set_value("UserPPNonReservoir", self.user_pp_outside_reservoir)
		pdm_object.set_value("PoissonRatioSource", self.poisson_ratio_source)
		pdm_object.set_value("UcsSource", self.ucs_source)
		pdm_object.set_value("OBG0Source", self.obg0_source)
		pdm_object.set_value("DFSource", self.df_source)
		pdm_object.set_value("K0SHSource", self.k0sh_source)
		pdm_object.set_value("FGShaleSource", self.fg_shale_source)
		pdm_object.set_value("K0FGSource", self.k0fg_source)

		pdm_object.set_value("UserPoissonRatio", self.user_poisson_ratio)
		pdm_object.set_value("UserUcs", self.user_ucs)
		pdm_object.set_value("UserDF", self.user_df)
		pdm_object.set_value("UserK0FG", self.user_k0fg)
		pdm_object.set_value("UserK0SH", self.user_k0sh)
		pdm_object.set_value("FGMultiplier", self.user_fg_shale)

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
			return WbsParameters.from_pdm_object(child)
		return None

	def set_parameters(self, wbs_parameters):
		children = self.children("WbsParameters")
		if len(children) == 1:
			pdm_params = children[0]
			wbs_parameters.to_pdm_object(pdm_params)
			pdm_params.update()
