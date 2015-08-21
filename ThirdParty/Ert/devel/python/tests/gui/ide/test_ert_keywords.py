from ert_gui.ide.keywords.definitions import IntegerArgument, StringArgument, BoolArgument, PathArgument, FloatArgument , PercentArgument
from ert_gui.ide.keywords import ErtKeywords
from ert_gui.ide.keywords.definitions.proper_name_argument import ProperNameArgument
from ert_gui.ide.keywords.definitions.proper_name_format_argument import ProperNameFormatArgument
from ert.test import ExtendedTestCase


class ErtKeywordTest(ExtendedTestCase):

    def setUp(self):
        self.keywords = ErtKeywords()

    def keywordTest(self, name, argument_types, documentation_link, group, required=False):
        self.assertTrue(name in self.keywords)

        cld = self.keywords[name]

        self.assertEqual(cld.keywordDefinition().name(), name)
        self.assertEqual(cld.group(), group)
        self.assertEqual(cld.documentationLink(), documentation_link)
        self.assertEqual(cld.isRequired(), required)

        arguments = cld.argumentDefinitions()

        self.assertEqual(len(arguments), len(argument_types))

        for index in range(len(arguments)):
            self.assertIsInstance(arguments[index], argument_types[index])


    def test_eclipse_keywords(self):
        self.keywordTest("DATA_FILE", [PathArgument], "keywords/data_file", "Eclipse", True)
        self.keywordTest("EQUIL_INIT_FILE", [PathArgument], "keywords/equil_init_file", "Eclipse")
        self.keywordTest("ECLBASE", [StringArgument], "keywords/eclbase", "Eclipse", True)
        self.keywordTest("JOBNAME", [StringArgument], "keywords/job_name", "Eclipse", True)
        self.keywordTest("GRID", [PathArgument], "keywords/grid", "Eclipse", True)
        self.keywordTest("INIT_SECTION", [PathArgument], "keywords/init_section", "Eclipse", True)
        self.keywordTest("SCHEDULE_FILE", [PathArgument], "keywords/schedule_file", "Eclipse", True)
        self.keywordTest("DATA_KW", [StringArgument, StringArgument], "keywords/data_kw", "Eclipse")
        self.keywordTest("IGNORE_SCHEDULE", [BoolArgument], "keywords/ignore_schedule", "Eclipse")


    def test_ensemble_keywords(self):
        self.keywordTest("NUM_REALIZATIONS", [IntegerArgument], "keywords/num_realizations", "Ensemble", True)
        self.keywordTest("END_DATE", [StringArgument], "keywords/end_date", "Ensemble")
        self.keywordTest("ENSPATH", [PathArgument], "keywords/enspath", "Ensemble")
        self.keywordTest("SELECT_CASE", [StringArgument], "keywords/select_case", "Ensemble")
        self.keywordTest("HISTORY_SOURCE", [StringArgument], "keywords/history_source", "Ensemble")
        self.keywordTest("REFCASE", [PathArgument], "keywords/refcase", "Ensemble")
        self.keywordTest("REFCASE_LIST", [StringArgument], "keywords/refcase_list", "Ensemble")
        self.keywordTest("INCLUDE", [PathArgument], "keywords/include", "Ensemble")
        self.keywordTest("OBS_CONFIG", [PathArgument], "keywords/obs_config", "Ensemble")
        self.keywordTest("RESULT_PATH", [PathArgument], "keywords/result_path", "Ensemble")
        self.keywordTest("LICENSE_PATH", [PathArgument], "keywords/license_path", "Ensemble")
        self.keywordTest("LOCAL_CONFIG", [StringArgument], "keywords/local_config", "Ensemble")

    def test_run_keywords(self):
        self.keywordTest("INSTALL_JOB", [StringArgument, PathArgument], "keywords/install_job", "Run")
        self.keywordTest("DELETE_RUNPATH", [StringArgument], "keywords/delete_runpath", "Run")
        self.keywordTest("KEEP_RUNPATH", [StringArgument], "keywords/keep_runpath", "Run")
        self.keywordTest("RUNPATH", [PathArgument], "keywords/runpath", "Run")
        self.keywordTest("RUNPATH_FILE", [PathArgument], "keywords/runpath_file", "Run")
        self.keywordTest("FORWARD_MODEL", [StringArgument], "keywords/forward_model", "Run")
        self.keywordTest("JOB_SCRIPT", [PathArgument], "keywords/job_script", "Run")
        self.keywordTest("RUN_TEMPLATE", [PathArgument, StringArgument], "keywords/run_template", "Run")
        self.keywordTest("LOG_LEVEL", [IntegerArgument], "keywords/log_level", "Run")
        self.keywordTest("LOG_FILE", [PathArgument], "keywords/log_file", "Run")
        self.keywordTest("MAX_SUBMIT", [IntegerArgument], "keywords/max_submit", "Run")
        self.keywordTest("MAX_RESAMPLE", [IntegerArgument], "keywords/max_resample", "Run")
        self.keywordTest("PRE_CLEAR_RUNPATH", [BoolArgument], "keywords/pre_clear_runpath", "Run")



    def test_control_simulations_keywords(self):
        self.keywordTest("MAX_RUNTIME", [IntegerArgument], "keywords/max_runtime", "Simulation Control")
        self.keywordTest("MIN_REALIZATIONS", [IntegerArgument,PercentArgument], "keywords/min_realizations", "Simulation Control")
        self.keywordTest("STOP_LONG_RUNNING", [BoolArgument], "keywords/stop_long_running", "Simulation Control")

    def test_parametrization_keywords(self):
        self.keywordTest("FIELD", [StringArgument,StringArgument,StringArgument], "keywords/field", "Parametrization")
        self.keywordTest("GEN_DATA", [StringArgument,StringArgument,StringArgument], "keywords/gen_data", "Parametrization")
        self.keywordTest("GEN_KW", [StringArgument,StringArgument,StringArgument], "keywords/gen_kw", "Parametrization")
        self.keywordTest("GEN_KW_TAG_FORMAT", [StringArgument], "keywords/gen_kw_tag_format", "Parametrization")
        self.keywordTest("GEN_PARAM", [StringArgument,StringArgument,StringArgument], "keywords/gen_param", "Parametrization")
        self.keywordTest("SUMMARY", [StringArgument], "keywords/summary", "Parametrization")
        self.keywordTest("DBASE_TYPE", [StringArgument], "keywords/dbase_type", "Parametrization")
        self.keywordTest("STORE_SEED", [StringArgument], "keywords/store_seed", "Parametrization")
        self.keywordTest("LOAD_SEED", [StringArgument], "keywords/load_seed", "Parametrization")
        self.keywordTest("SURFACE", [StringArgument], "keywords/surface", "Parametrization")


    def test_enkf_control_keywords(self):
        self.keywordTest("ENKF_ALPHA", [FloatArgument], "keywords/enkf_alpha", "Enkf Control")
        self.keywordTest("ENKF_BOOTSTRAP", [BoolArgument], "keywords/enkf_bootstrap", "Enkf Control")
        self.keywordTest("ENKF_CV_FOLDS", [IntegerArgument], "keywords/enkf_cv_folds", "Enkf Control")
        self.keywordTest("ENKF_FORCE_NCOMP", [BoolArgument], "keywords/enkf_force_ncomp", "Enkf Control")
        self.keywordTest("ENKF_LOCAL_CV", [BoolArgument], "keywords/enkf_local_cv", "Enkf Control")
        self.keywordTest("ENKF_PEN_PRESS", [BoolArgument], "keywords/enkf_pen_press", "Enkf Control")
        self.keywordTest("ENKF_MODE", [StringArgument], "keywords/enkf_mode", "Enkf Control")
        self.keywordTest("ENKF_MERGE_OBSERVATIONS", [BoolArgument], "keywords/enkf_merge_observations", "Enkf Control")
        self.keywordTest("ENKF_NCOMP", [IntegerArgument], "keywords/enkf_ncomp", "Enkf Control")
        self.keywordTest("ENKF_RERUN", [BoolArgument], "keywords/enkf_rerun", "Enkf Control")
        self.keywordTest("RERUN_START", [IntegerArgument], "keywords/rerun_start", "Enkf Control")
        self.keywordTest("ENKF_SCALING", [BoolArgument], "keywords/enkf_scaling", "Enkf Control")
        self.keywordTest("ENKF_TRUNCATION", [FloatArgument], "keywords/enkf_truncation", "Enkf Control")
        self.keywordTest("UPDATE_LOG_PATH", [PathArgument], "keywords/update_log_path", "Enkf Control")
        self.keywordTest("UPDATE_RESULTS", [BoolArgument], "keywords/update_results", "Enkf Control")
        self.keywordTest("ENKF_CROSS_VALIDATION", [StringArgument], "keywords/enkf_cross_validation", "Enkf Control")
        self.keywordTest("ENKF_KERNEL_REGRESSION", [StringArgument], "keywords/enkf_kernel_regression", "Enkf Control")
        self.keywordTest("ENKF_KERNEL_FUNCTION", [StringArgument], "keywords/enkf_kernel_function", "Enkf Control")
        self.keywordTest("ENKF_KERNEL_PARAM", [StringArgument], "keywords/enkf_kernel_param", "Enkf Control")
        self.keywordTest("ENKF_SCHED_FILE", [PathArgument], "keywords/enkf_sched_file", "Enkf Control")
        self.keywordTest("CASE_TABLE", [StringArgument], "keywords/case_table", "Enkf Control")
        self.keywordTest("CONTAINER", [StringArgument], "keywords/container", "Enkf Control")



    def test_analysis_module_keywords(self):
        self.keywordTest("ANALYSIS_LOAD", [StringArgument,StringArgument], "keywords/analysis_load", "Analysis Module")
        self.keywordTest("ANALYSIS_SELECT", [StringArgument], "keywords/analysis_select", "Analysis Module")
        self.keywordTest("ANALYSIS_SET_VAR", [StringArgument, StringArgument, StringArgument], "keywords/analysis_set_var", "Analysis Module")
        self.keywordTest("ANALYSIS_COPY", [StringArgument, StringArgument], "keywords/analysis_copy", "Analysis Module")
        self.keywordTest("ITER_CASE", [ProperNameFormatArgument], "keywords/iter_case", "Analysis Module")
        self.keywordTest("ITER_COUNT", [IntegerArgument], "keywords/iter_count", "Analysis Module")
        self.keywordTest("STD_CUTOFF", [FloatArgument], "keywords/std_cutoff", "Analysis Module")
        self.keywordTest("SINGLE_NODE_UPDATE", [BoolArgument], "keywords/single_node_update", "Analysis Module")


    def test_advanced_keywords(self):
        self.keywordTest("ADD_FIXED_LENGTH_SCHEDULE_KW", [StringArgument, StringArgument], "keywords/add_fixed_length_schedule_kw", "Advanced")
        self.keywordTest("ADD_STATIC_KW", [StringArgument, StringArgument], "keywords/add_static_kw", "Advanced")
        self.keywordTest("DEFINE", [ProperNameArgument, StringArgument], "keywords/define", "Advanced")
        self.keywordTest("SCHEDULE_PREDICTION_FILE", [PathArgument], "keywords/schedule_prediction_file", "Advanced")

    def test_queue_system_keywords(self):
        self.keywordTest("QUEUE_SYSTEM", [StringArgument], "keywords/queue_system", "Queue System")
        self.keywordTest("QUEUE_OPTION", [StringArgument, StringArgument, StringArgument], "keywords/queue_option", "Queue System")
        self.keywordTest("LSF_SERVER", [StringArgument], "keywords/lsf_server", "Queue System")
        self.keywordTest("LSF_QUEUE", [StringArgument], "keywords/lsf_queue", "Queue System")
        self.keywordTest("LSF_RESOURCES", [StringArgument], "keywords/lsf_resources", "Queue System")
        self.keywordTest("MAX_RUNNING_LSF", [IntegerArgument], "keywords/max_running_lsf", "Queue System")
        self.keywordTest("TORQUE_QUEUE", [StringArgument], "keywords/torque_queue", "Queue System")
        self.keywordTest("MAX_RUNNING_LOCAL", [IntegerArgument], "keywords/max_running_local", "Queue System")
        self.keywordTest("RSH_HOST", [StringArgument, StringArgument], "keywords/rsh_host", "Queue System")
        self.keywordTest("RSH_COMMAND", [PathArgument], "keywords/rsh_command", "Queue System")
        self.keywordTest("MAX_RUNNING_RSH", [IntegerArgument], "keywords/max_running_rsh", "Queue System")
        self.keywordTest("HOST_TYPE", [StringArgument], "keywords/host_type", "Queue System")


    def test_plot_keywords(self):
        self.keywordTest("IMAGE_VIEWER", [PathArgument], "keywords/image_viewer", "Plot")
        self.keywordTest("IMAGE_TYPE", [StringArgument], "keywords/image_type", "Plot")
        self.keywordTest("PLOT_DRIVER", [StringArgument], "keywords/plot_driver", "Plot")
        self.keywordTest("PLOT_ERRORBAR", [BoolArgument], "keywords/plot_errorbar", "Plot")
        self.keywordTest("PLOT_ERRORBAR_MAX", [IntegerArgument], "keywords/plot_errorbar_max", "Plot")
        self.keywordTest("PLOT_WIDTH", [IntegerArgument], "keywords/plot_width", "Plot")
        self.keywordTest("PLOT_HEIGHT", [IntegerArgument], "keywords/plot_height", "Plot")
        self.keywordTest("PLOT_REFCASE", [BoolArgument], "keywords/plot_refcase", "Plot")
        self.keywordTest("PLOT_REFCASE_LIST", [StringArgument], "keywords/plot_refcase_list", "Plot")
        self.keywordTest("PLOT_PATH", [PathArgument], "keywords/plot_path", "Plot")
        self.keywordTest("RFT_CONFIG", [PathArgument], "keywords/rft_config", "Plot")
        self.keywordTest("RFTPATH", [PathArgument], "keywords/rftpath", "Plot")

    def test_workflow_keywords(self):
        self.keywordTest("INTERNAL", [BoolArgument], "keywords/internal", "Workflow Jobs")
        self.keywordTest("FUNCTION", [StringArgument], "keywords/function", "Workflow Jobs")
        self.keywordTest("MODULE", [PathArgument], "keywords/module", "Workflow Jobs")
        self.keywordTest("EXECUTABLE", [PathArgument], "keywords/executable", "Workflow Jobs")
        self.keywordTest("MIN_ARG", [IntegerArgument], "keywords/min_arg", "Workflow Jobs")
        self.keywordTest("MAX_ARG", [IntegerArgument], "keywords/max_arg", "Workflow Jobs")
        self.keywordTest("ARG_TYPE", [StringArgument], "keywords/arg_type", "Workflow Jobs")
        self.keywordTest("LOAD_WORKFLOW_JOB", [StringArgument], "keywords/load_workflow_job", "Workflow Jobs")
        self.keywordTest("WORKFLOW_JOB_DIRECTORY", [PathArgument], "keywords/workflow_job_directory", "Workflow Jobs")
        self.keywordTest("LOAD_WORKFLOW", [PathArgument, StringArgument], "keywords/load_workflow", "Workflow Jobs")

    def test_qc_keywords(self):
        self.keywordTest("QC_WORKFLOW", [StringArgument], "keywords/qc_workflow", "Quality Check")
        self.keywordTest("QC_PATH", [PathArgument], "keywords/qc_path", "Quality Check")

    def test_report_keywords(self):
        self.keywordTest("REPORT_CONTEXT", [StringArgument, StringArgument], "keywords/report_context", "Report")
        self.keywordTest("REPORT_LIST", [StringArgument], "keywords/report_list", "Report")
        self.keywordTest("REPORT_PATH", [PathArgument], "keywords/report_path", "Report")
        self.keywordTest("REPORT_SEARCH_PATH", [StringArgument], "keywords/report_search_path", "Report")
        self.keywordTest("REPORT_WELL_LIST", [StringArgument, StringArgument], "keywords/report_well_list", "Report")
        self.keywordTest("REPORT_GROUP_LIST", [StringArgument, StringArgument], "keywords/report_group_list", "Report")
        self.keywordTest("REPORT_TIMEOUT", [IntegerArgument], "keywords/report_timeout", "Report")
        self.keywordTest("REPORT_LARGE", [BoolArgument], "keywords/report_large", "Report")



    def test_unix_environment_keywords(self):
        self.keywordTest("SETENV", [StringArgument, StringArgument], "keywords/setenv", "Unix")
        self.keywordTest("UMASK", [IntegerArgument], "keywords/umask", "Unix")
        self.keywordTest("UPDATE_PATH", [StringArgument,PathArgument], "keywords/update_path", "Unix")



