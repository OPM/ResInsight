/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'config_keys.h' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
 */


#ifndef  ERT_CONFIG_KEYS_H
#define  ERT_CONFIG_KEYS_H
#ifdef   __cplusplus
extern "C" {
#endif

/* These keys are used as options in KEY:VALUE statements */
#define  BASE_SURFACE_KEY                  "BASE_SURFACE"
#define  DEFINE_KEY                        "DEFINE"
#define  DYNAMIC_KEY                       "DYNAMIC"
#define  ECL_FILE_KEY                      "ECL_FILE"
#define  FORWARD_INIT_KEY                  "FORWARD_INIT"
#define  GENERAL_KEY                       "GENERAL"
#define  INCLUDE_KEY                       "INCLUDE"
#define  INIT_FILES_KEY                    "INIT_FILES"
#define  INIT_TRANSFORM_KEY                "INIT_TRANSFORM"
#define  INPUT_FORMAT_KEY                  "INPUT_FORMAT"
#define  INPUT_TRANSFORM_KEY               "INPUT_TRANSFORM"
#define  KEY_KEY                           "KEY"
#define  MAX_KEY                           "MAX"
#define  MIN_KEY                           "MIN"
#define  MIN_STD_KEY                       "MIN_STD"
#define  OUTPUT_FILE_KEY                   "OUTPUT_FILE"
#define  OUTPUT_FORMAT_KEY                 "OUTPUT_FORMAT"
#define  OUTPUT_TRANSFORM_KEY              "OUTPUT_TRANSFORM"
#define  PARAMETER_KEY                     "PARAMETER"
#define  REPORT_STEPS_KEY                  "REPORT_STEPS"
#define  RESULT_FILE_KEY                   "RESULT_FILE"
#define  TEMPLATE_KEY                      "TEMPLATE"


#define  ADD_FIXED_LENGTH_SCHEDULE_KW_KEY  "ADD_FIXED_LENGTH_SCHEDULE_KW"
#define  ANALYSIS_COPY_KEY                 "ANALYSIS_COPY"
#define  ANALYSIS_LOAD_KEY                 "ANALYSIS_LOAD"
#define  ANALYSIS_SET_VAR_KEY              "ANALYSIS_SET_VAR"
#define  ANALYSIS_SELECT_KEY               "ANALYSIS_SELECT"
#define  CASE_TABLE_KEY                    "CASE_TABLE"
#define  CONTAINER_KEY                     "CONTAINER"
#define  CUSTOM_KW_KEY                     "CUSTOM_KW"
#define  DATA_FILE_KEY                     "DATA_FILE"
#define  DATA_KW_KEY                       "DATA_KW"
#define  DBASE_TYPE_KEY                    "DBASE_TYPE"
#define  DELETE_RUNPATH_KEY                "DELETE_RUNPATH"
#define  ECLBASE_KEY                       "ECLBASE"
#define  END_DATE_KEY                      "END_DATE"
#define  ENKF_BOOTSTRAP_KEY                "ENKF_BOOTSTRAP"
#define  ENKF_PEN_PRESS_KEY                "ENKF_PEN_PRESS"
#define  ENKF_ALPHA_KEY                    "ENKF_ALPHA"
#define  ENKF_CROSS_VALIDATION_KEY         "ENKF_CROSS_VALIDATION"
#define  ENKF_CV_FOLDS_KEY                 "ENKF_CV_FOLDS"
#define  ENKF_FORCE_NCOMP_KEY              "ENKF_FORCE_NCOMP"
#define  ENKF_NCOMP_KEY                    "ENKF_NCOMP"
#define  ENKF_SCALING_KEY                  "ENKF_SCALING"
#define  ENKF_KERNEL_REG_KEY               "ENKF_KERNEL_REGRESSION"
#define  ENKF_KERNEL_FUNC_KEY              "ENKF_KERNEL_FUNCTION"
#define  ENKF_KERNEL_PARAM_KEY             "ENKF_KERNEL_PARAM"
#define  ENKF_LOCAL_CV_KEY                 "ENKF_LOCAL_CV"
#define  ENKF_MERGE_OBSERVATIONS_KEY       "ENKF_MERGE_OBSERVATIONS"
#define  ENKF_MODE_KEY                     "ENKF_MODE"
#define  ENKF_RERUN_KEY                    "ENKF_RERUN"
#define  ENKF_TRUNCATION_KEY               "ENKF_TRUNCATION"
#define  ENSPATH_KEY                       "ENSPATH"
#define  ITER_CASE_KEY                     "ITER_CASE"
#define  ITER_COUNT_KEY                    "ITER_COUNT"
#define  ITER_RETRY_COUNT_KEY              "ITER_RETRY_COUNT"
#define  FIELD_KEY                         "FIELD"
#define  FORWARD_MODEL_KEY                 "FORWARD_MODEL"
#define  GEN_DATA_KEY                      "GEN_DATA"
#define  GEN_KW_KEY                        "GEN_KW"
#define  GEN_KW_TAG_FORMAT_KEY             "GEN_KW_TAG_FORMAT"
#define  GEN_KW_EXPORT_FILE_KEY            "GEN_KW_EXPORT_FILE"
#define  GEN_PARAM_KEY                     "GEN_PARAM"
#define  GRID_KEY                          "GRID"
#define  HISTORY_SOURCE_KEY                "HISTORY_SOURCE"
#define  HOSY_TYPE_KEY                     "HOST_TYPE"
#define  IGNORE_SCHEDULE_KEY               "IGNORE_SCHEDULE"
#define  INIT_SECTION_KEY                  "INIT_SECTION"
#define  INSTALL_JOB_KEY                   "INSTALL_JOB"
#define  INSTALL_JOB_DIRECTORY_KEY         "INSTALL_JOB_DIRECTORY"
#define  JOB_SCRIPT_KEY                    "JOB_SCRIPT"
#define  JOBNAME_KEY                       "JOBNAME"
#define  LICENSE_PATH_KEY                  "LICENSE_PATH"
#define  LOAD_SEED_KEY                     "LOAD_SEED"
#define  LOCAL_CONFIG_KEY                  "LOCAL_CONFIG"
#define  LOG_FILE_KEY                      "LOG_FILE"
#define  LOG_LEVEL_KEY                     "LOG_LEVEL"
#define  LSF_QUEUE_KEY                     "LSF_QUEUE"
#define  LSF_RESOURCES_KEY                 "LSF_RESOURCES"
#define  LSF_SERVER_KEY                    "LSF_SERVER"
#define  TORQUE_QUEUE_KEY                  "TORQUE_QUEUE"
#define  MAX_RESAMPLE_KEY                  "MAX_RESAMPLE"
#define  MAX_RUNNING_LOCAL_KEY             "MAX_RUNNING_LOCAL"
#define  MAX_RUNNING_LSF_KEY               "MAX_RUNNING_LSF"
#define  MAX_RUNNING_RSH_KEY               "MAX_RUNNING_RSH"
#define  MAX_SUBMIT_KEY                    "MAX_SUBMIT"
#define  NUM_REALIZATIONS_KEY              "NUM_REALIZATIONS"
#define  MIN_REALIZATIONS_KEY              "MIN_REALIZATIONS"
#define  OBS_CONFIG_KEY                    "OBS_CONFIG"
#define  PLOT_PATH_KEY                     "PLOT_PATH"
#define  PRE_CLEAR_RUNPATH_KEY             "PRE_CLEAR_RUNPATH"
#define  QUEUE_SYSTEM_KEY                  "QUEUE_SYSTEM"
#define  QUEUE_OPTION_KEY                  "QUEUE_OPTION"
#define  QC_PATH_KEY                       "QC_PATH"
#define  QC_WORKFLOW_KEY                   "QC_WORKFLOW"
#define  HOOK_WORKFLOW_KEY                 "HOOK_WORKFLOW"
#define  REFCASE_KEY                       "REFCASE"
#define  REFCASE_LIST_KEY                  "REFCASE_LIST"
#define  RERUN_START_KEY                   "RERUN_START"
#define  RSH_COMMAND_KEY                   "RSH_COMMAND"
#define  RSH_HOST_KEY                      "RSH_HOST"
#define  RUNPATH_FILE_KEY                  "RUNPATH_FILE"
#define  RUNPATH_KEY                       "RUNPATH"
#define  ITER_RUNPATH_KEY                  "ITER_RUNPATH"
#define  RERUN_PATH_KEY                    "RERUN_PATH"
#define  RUN_TEMPLATE_KEY                  "RUN_TEMPLATE"
#define  RFT_CONFIG_KEY                    "RFT_CONFIG"
#define  RFTPATH_KEY                       "RFTPATH"
#define  SCHEDULE_FILE_KEY                 "SCHEDULE_FILE"
#define  SCHEDULE_PREDICTION_FILE_KEY      "SCHEDULE_PREDICTION_FILE"
#define  SETENV_KEY                        "SETENV"
#define  STATIC_KW_KEY                     "ADD_STATIC_KW"
#define  STD_CUTOFF_KEY                    "STD_CUTOFF"
#define  SUMMARY_KEY                       "SUMMARY"
#define  SURFACE_KEY                       "SURFACE"
#define  UPDATE_LOG_PATH_KEY               "UPDATE_LOG_PATH"
#define  UPDATE_PATH_KEY                   "UPDATE_PATH"
#define  SINGLE_NODE_UPDATE_KEY            "SINGLE_NODE_UPDATE"
#define  STORE_SEED_KEY                    "STORE_SEED"
#define  UMASK_KEY                         "UMASK"
#define  WORKFLOW_JOB_DIRECTORY_KEY        "WORKFLOW_JOB_DIRECTORY"
#define  LOAD_WORKFLOW_KEY                 "LOAD_WORKFLOW"
#define  LOAD_WORKFLOW_JOB_KEY             "LOAD_WORKFLOW_JOB"
#define  STOP_LONG_RUNNING_KEY             "STOP_LONG_RUNNING"
#define  MAX_RUNTIME_KEY                   "MAX_RUNTIME"
#define  TIME_MAP_KEY                      "TIME_MAP"
#define  EXT_JOB_SEARCH_PATH_KEY           "EXT_JOB_SEARCH_PATH"
#define  STD_SCALE_CORRELATED_OBS_KEY      "STD_SCALE_CORRELATED_OBS"

#define CONFIG_BOOL_STRING( var ) (var) ? "TRUE" : "FALSE"




#ifdef   __cplusplus
}
#endif
#endif
