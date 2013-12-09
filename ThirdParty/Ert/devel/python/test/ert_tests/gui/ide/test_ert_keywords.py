from ert_gui.ide.keywords.definitions import IntegerArgument, StringArgument, BoolArgument, PathArgument, FloatArgument
from ert_gui.ide.keywords import ErtKeywords
from ert_gui.ide.keywords.definitions.proper_name_argument import ProperNameArgument
from ert_gui.ide.keywords.definitions.proper_name_format_argument import ProperNameFormatArgument
from ert_tests import ExtendedTestCase


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
        self.keywordTest("DATA_FILE", [PathArgument], "eclipse/data_file", "Eclipse", True)
        self.keywordTest("EQUIL_INIT_FILE", [PathArgument], "eclipse/equil_init_file", "Eclipse")
        self.keywordTest("ECLBASE", [StringArgument], "eclipse/ecl_base", "Eclipse", True)
        self.keywordTest("JOBNAME", [StringArgument], "eclipse/job_name", "Eclipse", True)
        self.keywordTest("GRID", [PathArgument], "eclipse/grid", "Eclipse", True)
        self.keywordTest("INIT_SECTION", [PathArgument], "eclipse/init_section", "Eclipse", True)
        self.keywordTest("SCHEDULE_FILE", [PathArgument], "ensemble/schedule_file", "Eclipse", True)
        self.keywordTest("DATA_KW", [StringArgument, StringArgument], "ensemble/data_kw", "Eclipse")
        self.keywordTest("IGNORE_SCHEDULE", [BoolArgument], "ensemble/ignore_schedule", "Eclipse")


    def test_ensemble_keywords(self):
        self.keywordTest("NUM_REALIZATIONS", [IntegerArgument], "ensemble/num_realizations", "Ensemble", True)
        self.keywordTest("END_DATE", [StringArgument], "ensemble/end_date", "Ensemble")
        self.keywordTest("ENSPATH", [PathArgument], "ensemble/enspath", "Ensemble")
        self.keywordTest("SELECT_CASE", [StringArgument], "ensemble/select_case", "Ensemble")
        self.keywordTest("HISTORY_SOURCE", [StringArgument], "ensemble/history_source", "Ensemble")
        self.keywordTest("REFCASE", [PathArgument], "ensemble/refcase", "Ensemble")
        self.keywordTest("REFCASE_LIST", [StringArgument], "ensemble/refcase_list", "Ensemble")
        self.keywordTest("INCLUDE", [PathArgument], "ensemble/include", "Ensemble")
        self.keywordTest("OBS_CONFIG", [PathArgument], "ensemble/obs_config", "Ensemble")
        self.keywordTest("RESULT_PATH", [PathArgument], "ensemble/result_path", "Ensemble")
        self.keywordTest("LICENSE_PATH", [PathArgument], "ensemble/license_path", "Ensemble")
        self.keywordTest("LOCAL_CONFIG", [StringArgument], "ensemble/local_config", "Ensemble")

    def test_run_keywords(self):
        self.keywordTest("INSTALL_JOB", [StringArgument, PathArgument], "run/install_job", "Run")
        self.keywordTest("DELETE_RUNPATH", [StringArgument], "run/delete_runpath", "Run")
        self.keywordTest("KEEP_RUNPATH", [StringArgument], "run/keep_runpath", "Run")
        self.keywordTest("RUNPATH", [PathArgument], "run/runpath", "Run")
        self.keywordTest("RERUN_PATH", [PathArgument], "run/rerun_path", "Run")
        self.keywordTest("RUNPATH_FILE", [PathArgument], "run/runpath_file", "Run")
        self.keywordTest("FORWARD_MODEL", [StringArgument], "run/forward_model", "Run")
        self.keywordTest("JOB_SCRIPT", [PathArgument], "run/job_script", "Run")
        self.keywordTest("RUN_TEMPLATE", [PathArgument, StringArgument], "run/run_template", "Run")
        self.keywordTest("LOG_LEVEL", [IntegerArgument], "run/log_level", "Run")
        self.keywordTest("LOG_FILE", [PathArgument], "run/log_file", "Run")
        self.keywordTest("MAX_SUBMIT", [IntegerArgument], "run/max_submit", "Run")
        self.keywordTest("MAX_RESAMPLE", [IntegerArgument], "run/max_resample", "Run")
        self.keywordTest("PRE_CLEAR_RUNPATH", [BoolArgument], "run/pre_clear_runpath", "Run")



    def test_control_simulations_keywords(self):
        self.keywordTest("MAX_RUNTIME", [IntegerArgument], "control_simulations/max_runtime", "Simulation Control")
        self.keywordTest("MIN_REALIZATIONS", [IntegerArgument], "control_simulations/min_realizations", "Simulation Control")
        self.keywordTest("STOP_LONG_RUNNING", [BoolArgument], "control_simulations/stop_long_running", "Simulation Control")

    def test_parametrization_keywords(self):
        self.keywordTest("FIELD", [StringArgument,StringArgument,StringArgument], "parametrization/field", "Parametrization")
        self.keywordTest("GEN_DATA", [StringArgument,StringArgument,StringArgument], "parametrization/gen_data", "Parametrization")
        self.keywordTest("GEN_KW", [StringArgument,StringArgument,StringArgument], "parametrization/gen_kw", "Parametrization")
        self.keywordTest("GEN_KW_TAG_FORMAT", [StringArgument], "parametrization/gen_kw_tag_format", "Parametrization")
        self.keywordTest("GEN_PARAM", [StringArgument,StringArgument,StringArgument], "parametrization/gen_param", "Parametrization")
        self.keywordTest("SUMMARY", [StringArgument], "parametrization/summary", "Parametrization")
        self.keywordTest("DBASE_TYPE", [StringArgument], "parametrization/dbase_type", "Parametrization")
        self.keywordTest("STORE_SEED", [StringArgument], "parametrization/store_seed", "Parametrization")
        self.keywordTest("LOAD_SEED", [StringArgument], "parametrization/load_seed", "Parametrization")
        self.keywordTest("SURFACE", [StringArgument], "parametrization/surface", "Parametrization")


    def test_enkf_control_keywords(self):
        self.keywordTest("ENKF_ALPHA", [FloatArgument], "enkf_control/enkf_alpha", "Enkf Control")
        self.keywordTest("ENKF_BOOTSTRAP", [BoolArgument], "enkf_control/enkf_bootstrap", "Enkf Control")
        self.keywordTest("ENKF_CV_FOLDS", [IntegerArgument], "enkf_control/enkf_cv_folds", "Enkf Control")
        self.keywordTest("ENKF_FORCE_NCOMP", [BoolArgument], "enkf_control/enkf_force_ncomp", "Enkf Control")
        self.keywordTest("ENKF_LOCAL_CV", [BoolArgument], "enkf_control/enkf_local_cv", "Enkf Control")
        self.keywordTest("ENKF_PEN_PRESS", [BoolArgument], "enkf_control/enkf_pen_press", "Enkf Control")
        self.keywordTest("ENKF_MODE", [StringArgument], "enkf_control/enkf_mode", "Enkf Control")
        self.keywordTest("ENKF_MERGE_OBSERVATIONS", [BoolArgument], "enkf_control/enkf_merge_observations", "Enkf Control")
        self.keywordTest("ENKF_NCOMP", [IntegerArgument], "enkf_control/enkf_ncomp", "Enkf Control")
        self.keywordTest("ENKF_RERUN", [BoolArgument], "enkf_control/enkf_rerun", "Enkf Control")
        self.keywordTest("RERUN_START", [IntegerArgument], "enkf_control/rerun_start", "Enkf Control")
        self.keywordTest("ENKF_SCALING", [BoolArgument], "enkf_control/enkf_scaling", "Enkf Control")
        self.keywordTest("ENKF_TRUNCATION", [FloatArgument], "enkf_control/enkf_truncation", "Enkf Control")
        self.keywordTest("UPDATE_LOG_PATH", [PathArgument], "enkf_control/update_log_path", "Enkf Control")
        self.keywordTest("UPDATE_RESULTS", [BoolArgument], "enkf_control/update_results", "Enkf Control")
        self.keywordTest("ENKF_CROSS_VALIDATION", [StringArgument], "enkf_control/enkf_cross_validation", "Enkf Control")
        self.keywordTest("ENKF_KERNEL_REGRESSION", [StringArgument], "enkf_control/enkf_kernel_regression", "Enkf Control")
        self.keywordTest("ENKF_KERNEL_FUNCTION", [StringArgument], "enkf_control/enkf_kernel_function", "Enkf Control")
        self.keywordTest("ENKF_KERNEL_PARAM", [StringArgument], "enkf_control/enkf_kernel_param", "Enkf Control")
        self.keywordTest("ENKF_SCHED_FILE", [PathArgument], "enkf_control/enkf_sched_file", "Enkf Control")
        self.keywordTest("CASE_TABLE", [StringArgument], "enkf_control/case_table", "Enkf Control")
        self.keywordTest("CONTAINER", [StringArgument], "enkf_control/container", "Enkf Control")



    def test_analysis_module_keywords(self):
        self.keywordTest("ANALYSIS_LOAD", [StringArgument,StringArgument], "analysis_module/analysis_load", "Analysis Module")
        self.keywordTest("ANALYSIS_SELECT", [StringArgument], "analysis_module/analysis_select", "Analysis Module")
        self.keywordTest("ANALYSIS_SET_VAR", [StringArgument, StringArgument, StringArgument], "analysis_module/analysis_set_var", "Analysis Module")
        self.keywordTest("ANALYSIS_COPY", [StringArgument, StringArgument], "analysis_module/analysis_copy", "Analysis Module")
        self.keywordTest("ITER_RUNPATH", [PathArgument], "analysis_module/iter_runpath", "Analysis Module")
        self.keywordTest("ITER_CASE", [ProperNameFormatArgument], "analysis_module/iter_case", "Analysis Module")
        self.keywordTest("ITER_COUNT", [IntegerArgument], "analysis_module/iter_count", "Analysis Module")
        self.keywordTest("STD_CUTOFF", [FloatArgument], "analysis_module/std_cutoff", "Analysis Module")
        self.keywordTest("SINGLE_NODE_UPDATE", [BoolArgument], "analysis_module/single_node_update", "Analysis Module")


    def test_advanced_keywords(self):
        self.keywordTest("ADD_FIXED_LENGTH_SCHEDULE_KW", [StringArgument, StringArgument], "advanced/add_fixed_length_schedule_kw", "Advanced")
        self.keywordTest("ADD_STATIC_KW", [StringArgument, StringArgument], "advanced/add_static_kw", "Advanced")
        self.keywordTest("DEFINE", [ProperNameArgument, StringArgument], "advanced/define", "Advanced")
        self.keywordTest("SCHEDULE_PREDICTION_FILE", [PathArgument], "advanced/schedule_prediction_file", "Advanced")

    def test_queue_system_keywords(self):
        self.keywordTest("QUEUE_SYSTEM", [StringArgument], "queue_system/queue_system", "Queue System")
        self.keywordTest("QUEUE_OPTION", [StringArgument, StringArgument, StringArgument], "queue_system/queue_option", "Queue System")
        self.keywordTest("LSF_SERVER", [StringArgument], "queue_system/lsf_server", "Queue System")
        self.keywordTest("LSF_QUEUE", [StringArgument], "queue_system/lsf_queue", "Queue System")
        self.keywordTest("LSF_RESOURCES", [StringArgument], "queue_system/lsf_resources", "Queue System")
        self.keywordTest("MAX_RUNNING_LSF", [IntegerArgument], "queue_system/max_running_lsf", "Queue System")
        self.keywordTest("TORQUE_QUEUE", [StringArgument], "queue_system/torque_queue", "Queue System")
        self.keywordTest("MAX_RUNNING_LOCAL", [IntegerArgument], "queue_system/max_running_local", "Queue System")
        self.keywordTest("RSH_HOST", [StringArgument, StringArgument], "queue_system/rsh_host", "Queue System")
        self.keywordTest("RSH_COMMAND", [PathArgument], "queue_system/rsh_command", "Queue System")
        self.keywordTest("MAX_RUNNING_RSH", [IntegerArgument], "queue_system/max_running_rsh", "Queue System")
        self.keywordTest("HOST_TYPE", [StringArgument], "queue_system/host_type", "Queue System")


    def test_plot_keywords(self):
        self.keywordTest("IMAGE_VIEWER", [PathArgument], "plot/image_viewer", "Plot")
        self.keywordTest("IMAGE_TYPE", [StringArgument], "plot/image_type", "Plot")
        self.keywordTest("PLOT_DRIVER", [StringArgument], "plot/plot_driver", "Plot")
        self.keywordTest("PLOT_ERRORBAR", [BoolArgument], "plot/plot_errorbar", "Plot")
        self.keywordTest("PLOT_ERRORBAR_MAX", [IntegerArgument], "plot/plot_errorbar_max", "Plot")
        self.keywordTest("PLOT_WIDTH", [IntegerArgument], "plot/plot_width", "Plot")
        self.keywordTest("PLOT_HEIGHT", [IntegerArgument], "plot/plot_height", "Plot")
        self.keywordTest("PLOT_REFCASE", [BoolArgument], "plot/plot_refcase", "Plot")
        self.keywordTest("PLOT_REFCASE_LIST", [StringArgument], "plot/plot_refcase_list", "Plot")
        self.keywordTest("PLOT_PATH", [PathArgument], "plot/plot_path", "Plot")
        self.keywordTest("RFT_CONFIG", [PathArgument], "plot/rft_config", "Plot")
        self.keywordTest("RFTPATH", [PathArgument], "plot/rftpath", "Plot")

    def test_workflow_keywords(self):
        self.keywordTest("INTERNAL", [BoolArgument], "workflow_jobs/internal", "Workflow Jobs")
        self.keywordTest("FUNCTION", [StringArgument], "workflow_jobs/function", "Workflow Jobs")
        self.keywordTest("MODULE", [PathArgument], "workflow_jobs/module", "Workflow Jobs")
        self.keywordTest("EXECUTABLE", [PathArgument], "workflow_jobs/executable", "Workflow Jobs")
        self.keywordTest("MIN_ARG", [IntegerArgument], "workflow_jobs/min_arg", "Workflow Jobs")
        self.keywordTest("MAX_ARG", [IntegerArgument], "workflow_jobs/max_arg", "Workflow Jobs")
        self.keywordTest("ARG_TYPE", [StringArgument], "workflow_jobs/arg_type", "Workflow Jobs")
        self.keywordTest("LOAD_WORKFLOW_JOB", [StringArgument], "workflow_jobs/load_workflow_job", "Workflow Jobs")
        self.keywordTest("WORKFLOW_JOB_DIRECTORY", [PathArgument], "workflow_jobs/workflow_job_directory", "Workflow Jobs")
        self.keywordTest("LOAD_WORKFLOW", [PathArgument], "workflow_jobs/load_workflow", "Workflow Jobs")

    def test_qc_keywords(self):
        self.keywordTest("QC_WORKFLOW", [StringArgument], "qc/qc_workflow", "Quality Check")
        self.keywordTest("QC_PATH", [PathArgument], "qc/qc_path", "Quality Check")

    def test_report_keywords(self):
        self.keywordTest("REPORT_CONTEXT", [StringArgument, StringArgument], "report/report_context", "Report")
        self.keywordTest("REPORT_LIST", [StringArgument], "report/report_list", "Report")
        self.keywordTest("REPORT_PATH", [PathArgument], "report/report_path", "Report")
        self.keywordTest("REPORT_SEARCH_PATH", [StringArgument], "report/report_search_path", "Report")
        self.keywordTest("REPORT_WELL_LIST", [StringArgument, StringArgument], "report/report_well_list", "Report")
        self.keywordTest("REPORT_GROUP_LIST", [StringArgument, StringArgument], "report/report_group_list", "Report")
        self.keywordTest("REPORT_TIMEOUT", [IntegerArgument], "report/report_timeout", "Report")
        self.keywordTest("REPORT_LARGE", [BoolArgument], "report/report_large", "Report")



    def test_unix_environment_keywords(self):
        self.keywordTest("SETENV", [StringArgument, StringArgument], "unix_environment/setenv", "Unix")
        self.keywordTest("UMASK", [IntegerArgument], "unix_environment/umask", "Unix")
        self.keywordTest("UPDATE_PATH", [StringArgument,PathArgument], "unix_environment/update_path", "Unix")



