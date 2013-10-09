from .load_fail_type_enum import LoadFailTypeEnum
from .enkf_var_type_enum import EnkfVarType
from .enkf_state_type_enum import EnkfStateType
from .enkf_run_enum import EnkfRunEnum
from .enkf_obs_impl_type_enum import EnkfObservationImplementationType




# #-------------------------------------------------------------------
# #    enum implementations
# #-------------------------------------------------------------------
#
#
#
# class enkf_impl_type(enum):
#     """Defined in enkf_types.h"""
#     #INVALID = 0
#     #IMPL_TYPE_OFFSET = 100
#     #STATIC = 100
#     FIELD = None
#     GEN_KW = None
#     SUMMARY = None
#     GEN_DATA = None
#     #MAX_IMPL_TYPE = 113  #! not good to have several with same value, resolveValue fails!!!
#
# enkf_impl_type.FIELD = enkf_impl_type("Field", 104)
# enkf_impl_type.GEN_KW = enkf_impl_type("Keyword", 107)
# enkf_impl_type.SUMMARY = enkf_impl_type("Summary", 110)
# enkf_impl_type.GEN_DATA = enkf_impl_type("Data", 113)
#
#

#
#
#
#
# class gen_data_file_format(enum):
#     #defined in gen_data_config.h
#     GEN_DATA_UNDEFINED = None
#     ASCII          = None
#     ASCII_TEMPLATE = None
#     BINARY_DOUBLE  = None
#     BINARY_FLOAT   = None
#
#     INPUT_TYPES  = None
#     OUTPUT_TYPES = None
#
# gen_data_file_format.GEN_DATA_UNDEFINED = gen_data_file_format("", 0)
# gen_data_file_format.ASCII = gen_data_file_format("ASCII", 1)
# gen_data_file_format.ASCII_TEMPLATE = gen_data_file_format("ASCII_TEMPLATE", 2)
# gen_data_file_format.BINARY_DOUBLE = gen_data_file_format("BINARY_DOUBLE", 3)
# gen_data_file_format.BINARY_FLOAT = gen_data_file_format("BINARY_FLOAT", 4)
#
# gen_data_file_format.INPUT_TYPES = [gen_data_file_format.GEN_DATA_UNDEFINED,
#                                     gen_data_file_format.ASCII,
#                                     gen_data_file_format.BINARY_FLOAT,
#                                     gen_data_file_format.BINARY_DOUBLE]
#
# gen_data_file_format.OUTPUT_TYPES = [gen_data_file_format.GEN_DATA_UNDEFINED,
#                                      gen_data_file_format.ASCII,
#                                      gen_data_file_format.ASCII_TEMPLATE,
#                                      gen_data_file_format.BINARY_FLOAT,
#                                      gen_data_file_format.BINARY_DOUBLE]
#
#
# class field_type(enum):
#     ECLIPSE_RESTART = None
#     ECLIPSE_PARAMETER = None
#     GENERAL = None
#
# field_type.ECLIPSE_RESTART = field_type("Dynamic", 1)
# field_type.ECLIPSE_PARAMETER = field_type("Parameter", 2)
# field_type.GENERAL = field_type("General", 3)
#
#
# class truncation_type(enum):
#     TRUNCATE_NONE = None
#     TRUNCATE_MIN = None
#     TRUNCATE_MAX = None
#
#     @staticmethod
#     def resolveTruncationType(minimum, maximum):
#         if minimum == "" and maximum == "":
#             return truncation_type.TRUNCATE_NONE
#         elif not minimum == "" and not maximum == "":
#             return truncation_type.TRUNCATE_MIN + truncation_type.TRUNCATE_MAX
#         elif not minimum == "":
#             return truncation_type.TRUNCATE_MIN
#         elif not maximum == "":
#             return truncation_type.TRUNCATE_MAX
#         else:
#             raise AssertionError("This should not happen! o_O")
#
#
# truncation_type.TRUNCATE_NONE = truncation_type("TRUNCATE_NONE", 0)
# truncation_type.TRUNCATE_MIN = truncation_type("TRUNCATE_MIN", 1)
# truncation_type.TRUNCATE_MAX = truncation_type("TRUNCATE_MAX", 2)
#
# #print enum._enums
#
# class keep_runpath_type(enum):
#     DEFAULT_KEEP = None
#     EXPLICIT_DELETE = None
#     EXPLICIT_KEEP = None
#
# keep_runpath_type.DEFAULT_KEEP = keep_runpath_type("DEFAULT_KEEP", 0)
# keep_runpath_type.EXPLICIT_DELETE = keep_runpath_type("EXPLICIT_DELETE", 1)
# keep_runpath_type.EXPLICIT_KEEP = keep_runpath_type("EXPLICIT_KEEP", 2)
#
# class  run_mode_type(enum):
#     ENKF_ASSIMILATION = None
#     ENSEMBLE_EXPERIMENT = None
#     SMOOTHER_UPDATE = None
#     INIT_ONLY = None
#
# run_mode_type.ENKF_ASSIMILATION = run_mode_type( "ENKF_ASSIMILATION", 1)
# run_mode_type.ENKF_EXPERIMENT = run_mode_type( "ENKF_EXPERIMENT", 2)
# run_mode_type.SMOOTHER_UPDATE = run_mode_type( "SMOOTHER_UPDATE", 4)
# run_mode_type.INIT_ONLY = run_mode_type( "INIT_ONLY", 8)
#
# class history_source_type(enum):
#     SCHEDULE = None
#     REFCASE_SIMULATED = None
#     REFCASE_HISTORY = None
#
# history_source_type.SCHEDULE = history_source_type("SCHEDULE", 0)
# history_source_type.REFCASE_SIMULATED = history_source_type("REFCASE_SIMULATED", 1)
# history_source_type.REFCASE_HISTORY = history_source_type("REFCASE_HISTORY", 2)
#
#
# class obs_impl_type(enum):
#     GEN_OBS = None
#     SUMMARY_OBS = None
#     FIELD_OBS = None
#
# obs_impl_type.GEN_OBS = obs_impl_type("GEN_OBS", 1)
# obs_impl_type.SUMMARY_OBS = obs_impl_type("SUMMARY_OBS", 2)
# obs_impl_type.FIELD_OBS = obs_impl_type("FIELD_OBS", 3)
