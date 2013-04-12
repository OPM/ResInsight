/**
   This file contains verious default values which are compiled into
   the enkf executable. Everytime you add something here you should
   ask yourself:

    * Can we determine this automagically.
    * Should the user be required to enter this information.
    
*/

#ifndef __ENKF_DEFAULT__
#define __ENKF_DEFAULT__
#include <stdbool.h>

#define DEFAULT_RUNPATH_KEY  "DEFAULT_RUNPATH"
#define RERUN_RUNPATH_KEY    "DEFAULT_RERUN_PATH" 


/**
   The format string used when creating "search-strings" which should
   be replaced in the gen_kw template files - MUST contain one %s
   placeholder which will be replaced with the parameter name.
*/
#define DEFAULT_GEN_KW_TAG_FORMAT    "<%s>"


/**
   The format string used when creating search strings from user input
   with the 'DATA_KW' keyword. The format string must contain one '%s'
   placeholder which will be replaced with the user supplied key; can
   be just '%s' which means no ERT induced transformations.

      Example:
      -------
      DATA_KW   KEY1   VALUE1

      DATA_KW_FORMAT = [<%s>]

   In this case all occurences of '[<KEY1>]' will be replaced with
   'VALUE1'. The DATA_KW_TAG_FORMAT used on user supplied tag keys can
   in principle be different from the internal format, but this can of
   course be confusing. The internal format is hard linked to job
   description files, and can not easily be changed.
*/

//#define DEFAULT_DATA_KW_TAG_FORMAT    "<%s>"


/**
   This is the format used for tagging the internal variables like
   IENS, and ECLBASE. These values are written into the various job
   description files, care should therefor be taken before changing
   the value of this variable. It is not user modifiable, and can only
   be changed by recompiling.

*/
#define INTERNAL_DATA_KW_TAG_FORMAT   "<%s>"




#define DEFAULT_DBASE_TYPE "BLOCK_FS"

/** 
    The default number of block_fs instances allocated. 
*/
#define ENKF_DEFAULT_NUM_BLOCK_FS_DRIVERS 32


/* Eclipse IO  related stuff */
#define DEFAULT_FORMATTED   false
#define DEFAULT_UNIFIED     false



/* 
   Where the history is coming from - default value for config item:
   HISTORY_SOURCE Observe that the function:
   model_config_set_history_source() does currently not handle a
   default value different from SCHEDULE.
*/
#define DEFAULT_HISTORY_SOURCE  REFCASE_HISTORY


#define DEFAULT_MAX_SUBMIT           2        /* The number of times to resubmit - default value for config item: MAX_SUBMIT */
#define DEFAULT_MAX_INTERNAL_SUBMIT  1        /** Attached to keyword : MAX_RETRY */


#define DEFAULT_LOG_LEVEL 1
#define DEFAULT_LOG_FILE  "log"



/* 
   Defaults for the EnKF analysis. The analysis_config object is
   instantiated with these values.
*/
#define DEFAULT_ENKF_MODE               ENKF_STANDARD
#define DEFAULT_NCOMP                   1
#define DEFAULT_ENKF_TRUNCATION         0.99
#define DEFAULT_ENKF_ALPHA              1.50      /* Should be raised ?? */
#define DEFAULT_ENKF_STD_CUTOFF         1e-6
#define DEFAULT_MERGE_OBSERVATIONS      false
#define DEFAULT_RERUN                   false
#define DEFAULT_RERUN_START             0  
#define DEFAULT_UPDATE_LOG_PATH         "update_log"
#define DEFAULT_CV_NFOLDS               10
#define DEFAULT_ENKF_SCALING            true
#define DEFAULT_ENKF_KERNEL_REG         false
#define DEFAULT_ENKF_KERNEL_FUNC        1       /*Default is the Gaussian */
#define DEFAULT_ENKF_KERNEL_PARAM       1       /*Scale by the maximum value in the distance matrix */
#define DEFAULT_ENKF_CV                 false
#define DEFAULT_ENKF_BOOTSTRAP          false
#define DEFAULT_ENKF_PEN_PRESS          false
#define DEFAULT_ENKF_FORCE_NCOMP        false
#define DEFAULT_UPDATE_RESULTS          false
#define DEFAULT_SINGLE_NODE_UPDATE      true
#define DEFAULT_ANALYSIS_MODULE         "STD_ENKF"
#define DEFAULT_ANALYSIS_NUM_ITERATIONS 1

/* Default directories. */
#define DEFAULT_QC_PATH          "QC"
#define DEFAULT_REPORT_PATH      "reports" 
#define DEFAULT_PLOT_PATH        "plots"
#define DEFAULT_RUNPATH          "simulations/realization%d"
#define DEFAULT_ENSPATH          "storage"
#define DEFAULT_RFTPATH          "rft"
#define DEFAULT_PLOT_REFCASE     true
#define DEFAULT_REPORT_LARGE     false
#define DEFAULT_REPORT_TIMEOUT   120

#define DEFAULT_PRE_CLEAR_RUNPATH   false

#define DEFAULT_PLOT_WIDTH           1024
#define DEFAULT_PLOT_HEIGHT           768
#define DEFAULT_PLOT_ERRORBAR_MAX      25
#define DEFAULT_PLOT_ERRORBAR       false
#define DEFAULT_IMAGE_TYPE         "png"
#define DEFAULT_PLOT_DRIVER        "PLPLOT"
#define DEFAULT_IMAGE_VIEWER       "/usr/bin/display"
#define DEFAULT_PLOT_LOGY          false  

#define DEFAULT_STORE_PC           true
#define DEFAULT_PC_FILENAME        "PC_%04d-%04d-%s"
#define DEFAULT_PC_PATH            "PC"  

#define SUMMARY_KEY_JOIN_STRING ":"
#define USER_KEY_JOIN_STRING    ":"

#define DEFAULT_WORKFLOW_VERBOSE false

/*
  Some #define symbols used when saving configuration files.
*/
#define CONFIG_OPTION_FORMAT        " %s:%s"
#define CONFIG_FLOAT_OPTION_FORMAT  " %s:%g"
#define CONFIG_KEY_FORMAT           "%-24s"
#define CONFIG_VALUE_FORMAT         " %-32s"
#define CONFIG_FLOAT_FORMAT         " %32.4f"  /* One size - fits all :-) */
#define CONFIG_INT_FORMAT           " %32d"    /* One size - fits all :-) */
#define CONFIG_SHORT_VALUE_FORMAT   " %-12s"
#define CONFIG_ENDVALUE_FORMAT      " %-32s\n"
#define CONFIG_COMMENT_FORMAT       "-- %s\n"
#define CONFIG_COMMENTLINE_FORMAT   "----------------------------------------------------------------------\n"



/* The magic string used to signal that *ALL* static keywords should be included. */
#define DEFAULT_ALL_STATIC_KW "__ALL__"
#define NUM_STATIC_KW          56

/* 
   The whole thing is defined as one literal - just because I don't
   understand C-linkage.
*/
#define DEFAULT_STATIC_KW (const char *[NUM_STATIC_KW]) { \
  "ACAQ",       \
  "ACAQNUM",    \
  "DOUBHEAD",   \
  "ENDSOL",     \
  "HIDDEN",     \
  "IAAQ",       \
  "ICAQ",       \
  "ICAQNUM",    \
  "ICON",       \
  "IGRP",       \
  "ILBR",       \
  "ILBS",       \
  "INTEHEAD",   \
  "ISEG",       \
  "ISTHG",      \
  "ISTHW",      \
  "IWEL",       \
  "LOGIHEAD",   \
  "PRESSURE",   \
  "RS",         \
  "RSEG",       \
  "RV",         \
  "SAAQ",       \
  "SCAQ",       \
  "SCAQNUM",    \
  "SCON",       \
  "SGAS",       \
  "SGRP",       \
  "STARTSOL",   \
  "SWAT",       \
  "SWEL",       \
  "XAAQ",       \
  "XCON",       \
  "XGRP",       \
  "XWEL",       \
  "ZGRP",       \
  "ZWEL",       \
  "ENDLGR",     \
  "LGR",        \
  "LGRHEADD",   \
  "LGRHEADI",   \
  "LGRHEADQ",   \
  "LGRNAMES",   \
  "EOWC"    ,   \
  "IOWC"    ,   \
  "OWC"     ,   \
  "ZNODE"   ,   \
  "INODE"   ,   \
  "RNODE"   ,   \
  "LGWEL"   ,   \
  "IBRAN"   ,   \
  "INOBR"   ,   \
  "RBRAN"   ,   \
  "PRESROCC",   \
  "REGDIMS" ,   \
  "THRESHPR"}



/* 
   The string added at the beginning and end of string which should be
   replaced with the template parser.  
*/

#define DEFAULT_START_TAG "<"
#define DEFAULT_END_TAG   ">"

#define DEFAULT_MANUAL_URL "http://ert.nr.no/wiki/index.php/User_Manual"
#define DEFAULT_BROWSER    "firefox"

/*****************************************************************/

#define DEFAULT_VAR_DIR "/tmp/ert/var/run/ert"

/*****************************************************************/
/* file system spesific defaults. */

/**
  Name of the default case.
*/

#define CASE_LOG     "case-log"
#define CURRENT_CASE "current"
#define DEFAULT_CASE "default"

#define DEFAULT_PLAIN_NODE_PARAMETER_PATH           "tstep/%04d/mem%03d/Parameter"
#define DEFAULT_PLAIN_NODE_STATIC_PATH              "tstep/%04d/mem%03d/Static"
#define DEFAULT_PLAIN_NODE_DYNAMIC_FORECAST_PATH    "tstep/%04d/mem%03d/Forecast"
#define DEFAULT_PLAIN_NODE_DYNAMIC_ANALYZED_PATH    "tstep/%04d/mem%03d/Analyzed"
#define DEFAULT_PLAIN_NODE_INDEX_PATH               "tstep/%04d/mem%03d/Index"

#define DEFAULT_PLAIN_VECTOR_PARAMETER_PATH           "vectors/mem%03d/Parameter"
#define DEFAULT_PLAIN_VECTOR_STATIC_PATH              "vectors/mem%03d/Static"
#define DEFAULT_PLAIN_VECTOR_DYNAMIC_FORECAST_PATH    "vectors/mem%03d/Forecast"
#define DEFAULT_PLAIN_VECTOR_DYNAMIC_ANALYZED_PATH    "vectors/mem%03d/Analyzed"
#define DEFAULT_PLAIN_VECTOR_INDEX_PATH               "vectors/mem%03d/Index"


#define DEFAULT_CASE_PATH                        "%s/files"              // mountpoint
#define DEFAULT_CASE_MEMBER_PATH                 "%s/mem%03d/files"      // mountpoint/member
#define DEFAULT_CASE_TSTEP_PATH                  "%s/%04d/files"         // mountpoint/tstep
#define DEFAULT_CASE_TSTEP_MEMBER_PATH           "%s/%04d/mem%03d/files" // mountpoint/tstep/member   
// mountpoint = ENSPATH/case



#endif
