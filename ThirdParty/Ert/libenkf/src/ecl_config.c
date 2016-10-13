/*
 Copyright (C) 2011  Statoil ASA, Norway.

 The file 'ecl_config.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/set.h>
#include <ert/util/path_fmt.h>
#include <ert/util/util.h>
#include <ert/util/parser.h>
#include <ert/util/ui_return.h>

#include <ert/sched/sched_file.h>

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>
#include <ert/config/config_schema_item.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_io_config.h>
#include <ert/ecl/ecl_util.h>

#include <ert/enkf/enkf_util.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/ecl_refcase_list.h>
#include <ert/enkf/enkf_defaults.h>

/**
 This file implements a struct which holds configuration information
 needed to run ECLIPSE.

 Pointers to the fields in this structure are passed on to e.g. the
 enkf_state->shared_info object, but this struct is the *OWNER* of
 this information, and hence responsible for booting and deleting
 these objects.

 Observe that the distinction of what goes in model_config, and what
 goes in ecl_config is not entirely clear.
 */

struct ecl_config_struct
{
  ecl_io_config_type * io_config;       /* This struct contains information of whether the eclipse files should be formatted|unified|endian_fliped */
  path_fmt_type * eclbase;              /* A path_fmt instance with one %d specifer which will be used for eclbase - members will allocate private eclbase; i.e. updates will not be reflected. */
  sched_file_type * sched_file;         /* Will only contain the history - if predictions are active the member_config objects will have a private sched_file instance. */
  hash_type * fixed_length_kw;          /* Set of user-added SCHEDULE keywords with fixed length. */
  bool include_all_static_kw;           /* If true all static keywords are stored.*/
  set_type * static_kw_set;             /* Minimum set of static keywords which must be included to make valid restart files. */
  stringlist_type * user_static_kw;
  char * data_file;                     /* Eclipse data file. */
  time_t start_date;                    /* The start date of the ECLIPSE simulation - parsed from the data_file. */
  time_t end_date;                      /* An optional date value which can be used to check if the ECLIPSE simulation has been 'long enough'. */
  ecl_refcase_list_type * refcase_list;
  ecl_grid_type * grid;                 /* The grid which is active for this model. */
  char * schedule_prediction_file;      /* Name of schedule prediction file - observe that this is internally handled as a gen_kw node. */
  char * schedule_target_file;          /* File name to write schedule info to */
  char * input_init_section;            /* File name for ECLIPSE (EQUIL) initialisation - can be NULL if the user has not supplied INIT_SECTION. */
  char * init_section;                  /* Equal to the full path of input_init_section IFF input_init_section points to an existing file - otherwise equal to input_init_section. */
  int last_history_restart;
  bool can_restart;                     /* Have we found the <INIT> tag in the data file? */
  int num_cpu;                          /* We should parse the ECLIPSE data file and determine how many cpus this eclipse file needs. */
  ert_ecl_unit_enum unit_system;        /* Either metric, field or lab */
};

/*****************************************************************/

/**
 With this function we try to determine whether ECLIPSE is active
 for this case, i.e. if ECLIPSE is part of the forward model. This
 should ideally be inferred from the FORWARD model, but what we do
 here is just to check if the core field ->eclbase and ->data_file
 have been set. If they are both equal to NULL we assume that
 ECLIPSE is not active and return false, otherwise we return true.
 */

bool ecl_config_active(const ecl_config_type * config)
{
  if ((config->eclbase == NULL )&& (config->data_file == NULL))
    return false;
  else
    return true;
}

  /**
   Could look up the sched_file instance directly - because the
   ecl_config will never be the owner of a file with predictions.
   */

int ecl_config_get_last_history_restart(const ecl_config_type * ecl_config)
{
  return ecl_config->last_history_restart;
}

bool ecl_config_can_restart(const ecl_config_type * ecl_config)
{
  return ecl_config->can_restart;
}

void ecl_config_assert_restart(const ecl_config_type * ecl_config)
{
  if (!ecl_config_can_restart(ecl_config))
  {
    fprintf(stderr, "** Warning - tried to restart case which is not properly set up for restart.\n");
    fprintf(stderr, "** Need <INIT> in datafile and INIT_SECTION keyword in config file.\n");
    util_exit("%s: exiting \n", __func__);
  }
}

ui_return_type * ecl_config_validate_data_file(const ecl_config_type * ecl_config, const char * data_file) {
  if (util_file_exists(data_file)) 
    return ui_return_alloc(UI_RETURN_OK);
  else {
    ui_return_type * ui_return = ui_return_alloc(UI_RETURN_FAIL);
    char * error_msg = util_alloc_sprintf("File not found:%s" , data_file);
    ui_return_add_error(ui_return , error_msg);
    free( error_msg );
    return ui_return;
  }
}


void ecl_config_set_data_file(ecl_config_type * ecl_config, const char * data_file) {
  ecl_config->data_file = util_realloc_string_copy(ecl_config->data_file, data_file);
  {
    FILE * stream = util_fopen(ecl_config->data_file, "r");
    basic_parser_type * parser = basic_parser_alloc(NULL, NULL, NULL, NULL, "--", "\n");
    char * init_tag = enkf_util_alloc_tagged_string("INIT");
    
    ecl_config->can_restart = basic_parser_fseek_string(parser, stream, init_tag, false, true);
    
    free(init_tag);
    basic_parser_free(parser);
    fclose(stream);
  }
  ecl_config->start_date = ecl_util_get_start_date(ecl_config->data_file);
  ecl_config->num_cpu = ecl_util_get_num_cpu(ecl_config->data_file);
  ecl_config->unit_system = ecl_util_get_unit_set(ecl_config->data_file);
}


const char * ecl_config_get_data_file(const ecl_config_type * ecl_config)
{
  return ecl_config->data_file;
}

time_t ecl_config_get_start_date(const ecl_config_type * ecl_config)
{
  return ecl_config->start_date;
}

time_t ecl_config_get_end_date(const ecl_config_type * ecl_config)
{
  return ecl_config->end_date;
}

static void ecl_config_set_end_date(ecl_config_type * ecl_config, time_t end_date)
{
  ecl_config->end_date = end_date;
}

int ecl_config_get_num_cpu(const ecl_config_type * ecl_config)
{
  return ecl_config->num_cpu;
}

const char * ecl_config_get_schedule_prediction_file(const ecl_config_type * ecl_config)
{
  return ecl_config->schedule_prediction_file;
}

/**
   Observe: The real schedule prediction functionality is implemented
   as a special GEN_KW node in ensemble_config.
 */

void ecl_config_set_schedule_prediction_file(ecl_config_type * ecl_config, const char * schedule_prediction_file)
{
  ecl_config->schedule_prediction_file = util_realloc_string_copy(ecl_config->schedule_prediction_file, schedule_prediction_file);
}

const char * ecl_config_get_schedule_file(const ecl_config_type * ecl_config)
{
  if (ecl_config->sched_file != NULL )
    return sched_file_iget_filename(ecl_config->sched_file, 0);
  else
    return NULL ;
}

bool ecl_config_has_schedule(const ecl_config_type * ecl_config)
{
  if (ecl_config->sched_file == NULL )
    return false;
  else
    return true;
}

bool ecl_config_has_init_section(const ecl_config_type * ecl_config)
{
  if (ecl_config->init_section == NULL )
    return false;
  else
    return true;
}



ui_return_type * ecl_config_validate_schedule_file(const ecl_config_type * ecl_config , const char * schedule_file) {
  if ((ecl_config->start_date != -1) && (util_file_exists(schedule_file)))
    return ui_return_alloc(UI_RETURN_OK);
  else {
    ui_return_type * ui_return = ui_return_alloc(UI_RETURN_FAIL);
    
    if (ecl_config->start_date == -1)
      ui_return_add_error(ui_return, "You must set the ECLIPSE datafile before you can set the SCHEDULE file.");
    
    if (!util_file_exists(schedule_file)) {
      char * error_msg = util_alloc_sprintf("SCHEDULE file:%s not found" , schedule_file);
      ui_return_add_error(ui_return , error_msg);
      free(error_msg);
    }
    
    return ui_return;
  }
}


/**
   Observe: This function makes a hard assumption that the
   ecl_config->start_date has already been set. (And should not be
   changed either ...)
*/

void ecl_config_set_schedule_file(ecl_config_type * ecl_config, const char * schedule_file, const char * schedule_target_file)
{
  if (schedule_target_file)
      ecl_config->schedule_target_file = util_alloc_string_copy(schedule_target_file);
  else {
    char * base; /* The schedule target file will be without any path component */
    char * ext;
    util_alloc_file_components(schedule_file, NULL, &base, &ext);
    ecl_config->schedule_target_file = util_alloc_filename(NULL, base, ext);
    free(ext);
    free(base);
  }
  
  ecl_config->sched_file = sched_file_alloc(ecl_config->start_date);
  sched_file_parse(ecl_config->sched_file, schedule_file);
  ecl_config->last_history_restart = sched_file_get_num_restart_files(ecl_config->sched_file) - 1; /* We keep track of this - so we can stop assimilation at the end of history */

  {
    hash_iter_type * iter = hash_iter_alloc(ecl_config->fixed_length_kw);
    while (!hash_iter_is_complete(iter)) {
      const char * key = hash_iter_get_next_key(iter);
      int length = hash_get_int(ecl_config->fixed_length_kw, key);
      
      sched_file_add_fixed_length_kw(ecl_config->sched_file, key, length);
    }
    hash_iter_free(iter);
  }
}



void ecl_config_add_fixed_length_schedule_kw(ecl_config_type * ecl_config, const char * kw, int length)
{
  hash_insert_int(ecl_config->fixed_length_kw, kw, length);
  if (ecl_config->sched_file != NULL )
    sched_file_add_fixed_length_kw(ecl_config->sched_file, kw, length);

}


ui_return_type * ecl_config_validate_eclbase(const ecl_config_type * ecl_config, const char * eclbase_fmt) {
  if (ecl_util_valid_basename_fmt(eclbase_fmt)) 
    return ui_return_alloc(UI_RETURN_OK);
  else {
    ui_return_type * ui_return = ui_return_alloc(UI_RETURN_FAIL);
    {
      char * error_msg = util_alloc_sprintf("The format string: %s was invalid as ECLBASE format", eclbase_fmt);
      ui_return_add_error(ui_return, error_msg);
      free(error_msg);
    }
    ui_return_add_help(ui_return , "The eclbase format must have all characters in the same case,");
    ui_return_add_help(ui_return , "in addition it can contain a %d specifier which will be");
    ui_return_add_help(ui_return , "with the realization number.");

    return ui_return;
  }
}

/**
 The value of eclbase is in addition internalized in each enkf_state
 object, i.e. the _set routine must be called from enkf_main, and
 call enkf_state_update_eclbase() afterwards.
 */

void ecl_config_set_eclbase(ecl_config_type * ecl_config, const char * eclbase_fmt)
{
  if (ecl_config->eclbase != NULL )
    path_fmt_free(ecl_config->eclbase);
  ecl_config->eclbase = path_fmt_alloc_path_fmt(eclbase_fmt);
}


/**
 Observe that this function returns a (char *) - corresponding to
 the argument used when calling the ecl_config_set_eclbase()
 function, and not a path_fmt instance.
 */

const char * ecl_config_get_eclbase(const ecl_config_type * ecl_config)
{
  return path_fmt_get_fmt(ecl_config->eclbase);
}

/**
 Can be called with @refcase == NULL - which amounts to clearing the
 current refcase.
*/
bool ecl_config_load_refcase(ecl_config_type * ecl_config, const char * refcase)
{
  return ecl_refcase_list_set_default(ecl_config->refcase_list, refcase);
}


ui_return_type * ecl_config_validate_refcase( const ecl_config_type * ecl_config , const char * refcase) {
  if (ecl_sum_case_exists( refcase ))
    return ui_return_alloc( UI_RETURN_OK );
  else {
    ui_return_type * ui_return = ui_return_alloc( UI_RETURN_FAIL );
    char * error_msg = util_alloc_sprintf( "Could not load summary case from:%s \n",refcase);
    ui_return_add_error( ui_return , error_msg );
    free( error_msg );
    return ui_return;
  }
}


/**
 Will return NULL if no refcase is set.
 */
const char * ecl_config_get_refcase_name(const ecl_config_type * ecl_config)
{
  const ecl_sum_type * refcase = ecl_refcase_list_get_default(ecl_config->refcase_list);
  if (refcase == NULL )
    return NULL ;
  else
    return ecl_sum_get_case(refcase);

}

/**
 This function will clear the list of static keywords supplied by
 the user. The default built in keywords are not touched.
 */

void ecl_config_clear_static_kw(ecl_config_type * ecl_config)
{
  ecl_config->include_all_static_kw = false;
  stringlist_clear(ecl_config->user_static_kw);
}

/**
 Returns a stringlist of the user-defined static keywords.
 */
stringlist_type * ecl_config_get_static_kw_list(const ecl_config_type * ecl_config)
{
  return ecl_config->user_static_kw;
}

  /* The semantic regarding INIT_SECTION is as follows:

   1. If the INIT_SECTION points to an existing file - the
   ecl_config->input_init_section is set to the absolute path of
   this file.

   2. If the INIT_SECTION points to a not existing file:

   a. We assert that INIT_SECTION points to a pure filename,
   i.e. /some/path/which/does/not/exist is NOT accepted. In
   the case the input argument contain a path a error message
   will be printed on stderr and the ->init_section will not
   be set.
   b. The ecl_config->input_init_section is set to point to this
   file.
   c. WE TRUST THE USER TO SUPPLY CONTENT (THROUGH SOME FUNKY
   FORWARD MODEL) IN THE RUNPATH. This can unfortunately not
   be checked/verified before the ECLIPSE simulation fails.


   The INIT_SECTION keyword and <INIT> in the datafile (checked with
   ecl_config->can_restart) interplay as follows:


   CASE   |   INIT_SECTION  |  <INIT>    | OK ?
   ---------------------------------------------
   0      |   Present       | Present    |  Yes
   1      |   Not Present   | Present    |  No
   2      |   Present       | Not present|  No
   3      |   Not Present   | Not present|  Yes
   ---------------------------------------------


   Case 0: This is the most flexible case, which can do arbitrary
   restart.

   Case 1: In this case the datafile will contain a <INIT> tag, we
   we do not have the info to replace that tag with for
   initialisation, and ECLIPSE will fail. Strictly speaking this
   case can actually restart, but that is not enough - we let
   this case fail hard.

   Case 2: We have some INIT_SECTION infor, but no tag in he
   datafile to update. If the datafile has embedded
   initialisation info this case will work for init; but it is
   logically flawed, and not accepted. Currently only a warning.

   Case 3: This case has just the right amount of information for
   initialisation, but it is 'consistently unable' to restart.

*/

ui_return_type * ecl_config_validate_init_section(const ecl_config_type * ecl_config, const char * input_init_section) {
  if (ecl_config->can_restart)
    return ui_return_alloc( UI_RETURN_OK );
  else {
    ui_return_type * ui_return = ui_return_alloc(UI_RETURN_FAIL);
    ui_return_add_error( ui_return , "The <INIT> tag was not found in datafile - can not set INIT_SECTION keyword\n");
    return ui_return;
  }
}


void ecl_config_set_init_section(ecl_config_type * ecl_config, const char * input_init_section) {
  if (ecl_config->can_restart)  { 
    /* The <INIT> tag is set. */
    ecl_config->input_init_section = util_realloc_string_copy(ecl_config->input_init_section, input_init_section); /* input_init_section = path/to/init_section         */
    if (util_file_exists(ecl_config->input_init_section))
    { /* init_section       = $CWD/path/to/init_section */
      util_safe_free(ecl_config->init_section);
      ecl_config->init_section = util_alloc_realpath(input_init_section);
    }
    else
    {
      char * path;

      util_alloc_file_components(ecl_config->input_init_section, &path, NULL, NULL );
      if (path != NULL )
        fprintf(stderr,
            "** Warning: %s: When INIT_SECTION:%s points to a non-existing file - you can not have any path components.\n",
            __func__, input_init_section);
      else
        ecl_config->init_section = util_alloc_string_copy(input_init_section);

      util_safe_free(path);
    }
  }
  else
    /* 
       The <INIT> tag is not set - we can not utilize the
       input_init_section info, and we just ignore it.
     */
    fprintf(stderr,
        "** Warning: <INIT> tag was not found in datafile - can not utilize INIT_SECTION keyword - ignored.\n");
}


/**
 This just returns the string which has been set with the
 ecl_config_set_init_section() function, whereas the
 ecl_config_get_equil_init_file() function will return the absolute
 path to the init_section (if it exists).
 */

const char * ecl_config_get_init_section(const ecl_config_type * ecl_config)
{
  return ecl_config->input_init_section;
}

const char * ecl_config_get_equil_init_file(const ecl_config_type * ecl_config)
{
  return ecl_config->init_section;
}


static void ecl_config_init_static_kw(ecl_config_type * ecl_config)
{
  int i;
  for (i = 0; i < NUM_STATIC_KW; i++)
    set_add_key(ecl_config->static_kw_set, DEFAULT_STATIC_KW
    [i]);
}

ecl_config_type * ecl_config_alloc()
{
  ecl_config_type * ecl_config = util_malloc(sizeof *ecl_config);

  ecl_config->io_config = ecl_io_config_alloc(DEFAULT_FORMATTED, DEFAULT_UNIFIED, DEFAULT_UNIFIED);
  ecl_config->fixed_length_kw = hash_alloc();
  ecl_config->eclbase = NULL;
  ecl_config->include_all_static_kw = false;
  ecl_config->static_kw_set = set_alloc_empty();
  ecl_config->user_static_kw = stringlist_alloc_new();
  ecl_config->num_cpu = 1; /* This must get a valid default in case no ECLIPSE datafile is provided. */
  ecl_config->unit_system = ERT_ECL_METRIC_UNITS;
  ecl_config->data_file = NULL;
  ecl_config->input_init_section = NULL;
  ecl_config->init_section = NULL;
  ecl_config->grid = NULL;
  ecl_config->can_restart = false;
  ecl_config->start_date = -1;
  ecl_config->end_date = -1;
  ecl_config->sched_file = NULL;
  ecl_config->schedule_prediction_file = NULL;
  ecl_config->schedule_target_file = NULL;
  ecl_config->refcase_list = ecl_refcase_list_alloc();

  ecl_config_init_static_kw(ecl_config);

  return ecl_config;
}

void ecl_config_init(ecl_config_type * ecl_config, const config_content_type * config)
{
  if (config_content_has_item(config, ECLBASE_KEY)) {
    ui_return_type * ui_return = ecl_config_validate_eclbase(ecl_config, config_content_iget(config, ECLBASE_KEY, 0, 0));
    if (ui_return_get_status(ui_return) == UI_RETURN_OK)
      ecl_config_set_eclbase(ecl_config, config_content_iget(config, ECLBASE_KEY, 0, 0));
    else
      util_abort("%s: failed to set eclbase format. Error:%s\n", __func__ , ui_return_get_last_error(ui_return));
    ui_return_free(ui_return);
  }

  if (config_content_has_item(config, DATA_FILE_KEY))
  {
    ui_return_type * ui_return = ecl_config_validate_data_file(ecl_config, config_content_iget(config, DATA_FILE_KEY, 0, 0));
    if (ui_return_get_status( ui_return ) == UI_RETURN_OK)
      ecl_config_set_data_file( ecl_config, config_content_iget(config, DATA_FILE_KEY, 0, 0) );
    else
      util_abort("%s: problem setting ECLIPSE data file\n",__func__ , ui_return_get_last_error(ui_return));

    ui_return_free(ui_return);
  }

  if (config_content_has_item(config, SCHEDULE_FILE_KEY)) {
    const char * schedule_target_file = config_content_safe_iget(config, SCHEDULE_FILE_KEY, 0, 1);
    if (schedule_target_file) {
      ui_return_type * ui_return_sched_target_file = ecl_config_validate_schedule_file(ecl_config, schedule_target_file);
      if (!ui_return_get_status(ui_return_sched_target_file) == UI_RETURN_OK) {
         util_abort("%s: failed to set target schedule file. Error:%s\n",__func__ , ui_return_get_last_error(ui_return_sched_target_file));
      }
      ui_return_free(ui_return_sched_target_file);
    }

    ui_return_type * ui_return = ecl_config_validate_schedule_file(ecl_config, config_content_iget(config, SCHEDULE_FILE_KEY, 0, 0));
    if (ui_return_get_status(ui_return) == UI_RETURN_OK)
      ecl_config_set_schedule_file(ecl_config, config_content_iget(config, SCHEDULE_FILE_KEY, 0, 0), schedule_target_file);
    else
      util_abort("%s: failed to set schedule file. Error:%s\n",__func__ , ui_return_get_last_error(ui_return));

    ui_return_free(ui_return);
  }

  if (config_content_has_item(config, GRID_KEY)) {
    const char * grid_file = config_content_iget(config, GRID_KEY, 0, 0);
    ui_return_type * ui_return = ecl_config_validate_grid( ecl_config , grid_file);
    if (ui_return_get_status(ui_return) == UI_RETURN_OK)
      ecl_config_set_grid(ecl_config, grid_file );
    else
      util_abort("%s: failed to set grid file:%s  Error:%s \n",__func__ , grid_file , ui_return_get_last_error(ui_return));

    ui_return_free( ui_return );
  }


  if (config_content_has_item(config, ADD_FIXED_LENGTH_SCHEDULE_KW_KEY))
  {
    int iocc;
    for (iocc = 0; iocc < config_content_get_occurences(config, ADD_FIXED_LENGTH_SCHEDULE_KW_KEY); iocc++)
      ecl_config_add_fixed_length_schedule_kw(ecl_config,
                                              config_content_iget(config, ADD_FIXED_LENGTH_SCHEDULE_KW_KEY, iocc, 0),
                                              config_content_iget_as_int(config, ADD_FIXED_LENGTH_SCHEDULE_KW_KEY, iocc, 1));
  }

  if (config_content_has_item(config, REFCASE_KEY))
  {
    const char * refcase_path = config_content_get_value_as_path(config, REFCASE_KEY);
    if (!ecl_config_load_refcase(ecl_config, refcase_path))
      fprintf(stderr, "** Warning: loading refcase:%s failed \n", refcase_path);
  }

  if (config_content_has_item(config, REFCASE_LIST_KEY))
  {
    config_content_item_type * item = config_content_get_item(config, REFCASE_LIST_KEY);
    int i;
    for (i = 0; i < config_content_item_get_size(item); i++)
    {
      config_content_node_type * node = config_content_item_iget_node(item, i);
      int j;
      for (j = 0; j < config_content_node_get_size(node); j++)
      {
        const char * case_glob = config_content_node_iget_as_path(node, j);
        ecl_refcase_list_add_matching(ecl_config->refcase_list, case_glob);
      }
    }
  }

  if (config_content_has_item(config, INIT_SECTION_KEY))
    ecl_config_set_init_section(ecl_config, config_content_get_value(config, INIT_SECTION_KEY));
  else if (ecl_config->can_restart)
    /**
     This is a hard error - the datafile contains <INIT>, however
     the config file does NOT contain INIT_SECTION, i.e. we have
     no information to fill in for the <INIT> section. This case
     will not be able to initialize an ECLIPSE model, and that is
     broken behaviour.
     */
    util_exit("Sorry: when the datafile contains <INIT> the config file MUST have the INIT_SECTION keyword. \n");

  /*
   The user has not supplied a INIT_SECTION keyword whatsoever,
   this essentially means that we can not restart - because:

   1. The EQUIL section must be inlined in the DATAFILE without any
   special markup.

   2. ECLIPSE will fail hard if the datafile contains both an EQUIL
   section and a restart statement, and when we have not marked
   the EQUIL section specially with the INIT_SECTION keyword it
   is impossible for ERT to dynamically change between a
   datafile with initialisation and a datafile for restart.

   IFF the user has no intentitions of any form of restart, this is
   perfectly legitemate.
   */
  if (config_content_has_item(config, END_DATE_KEY))
  {
    const char * date_string = config_content_get_value(config, END_DATE_KEY);
    time_t end_date;
    if (util_sscanf_date_utc(date_string, &end_date))
      ecl_config_set_end_date(ecl_config, end_date);
    else
      fprintf(stderr, "** WARNING **: Failed to parse %s as a date - should be in format dd/mm/yyyy \n", date_string);
  }
}

void ecl_config_free(ecl_config_type * ecl_config)
{
  ecl_io_config_free(ecl_config->io_config);
  if (ecl_config->eclbase != NULL )
    path_fmt_free(ecl_config->eclbase);

  set_free(ecl_config->static_kw_set);
  stringlist_free(ecl_config->user_static_kw);
  util_safe_free(ecl_config->data_file);
  if (ecl_config->sched_file != NULL )
    sched_file_free(ecl_config->sched_file);

  util_safe_free(ecl_config->schedule_target_file);
  hash_free(ecl_config->fixed_length_kw);

  util_safe_free(ecl_config->input_init_section);
  util_safe_free(ecl_config->init_section);
  util_safe_free(ecl_config->schedule_prediction_file);

  if (ecl_config->grid != NULL )
    ecl_grid_free(ecl_config->grid);

  ecl_refcase_list_free(ecl_config->refcase_list);

  free(ecl_config);
}

/**
 This function adds a keyword to the list of restart keywords wich
 are included. Observe that ecl_util_escape_kw() is called prior to
 adding it.

 The kw __ALL__ is magic; and will result in a request to store all
 static kewyords. This wastes disk-space, but might be beneficial
 when debugging.
 */

void ecl_config_add_static_kw(ecl_config_type * ecl_config, const char * _kw)
{
  if (strcmp(_kw, DEFAULT_ALL_STATIC_KW) == 0)
    ecl_config->include_all_static_kw = true;
  else
  {
    char * kw = util_alloc_string_copy(_kw);
    ecl_util_escape_kw(kw);
    if (!stringlist_contains(ecl_config->user_static_kw, kw))
      stringlist_append_owned_ref(ecl_config->user_static_kw, kw);
  }
}

/**
 This function checks whether the static kw should be
 included. Observe that it is __assumed__ that ecl_util_escape_kw()
 has already been called on the kw.
 */

bool ecl_config_include_static_kw(const ecl_config_type * ecl_config, const char * kw)
{
  if (ecl_config->include_all_static_kw)
    return true;
  else
  {
    if (set_has_key(ecl_config->static_kw_set, kw))
      return true;
    else
      return stringlist_contains(ecl_config->user_static_kw, kw);
  }
}

ecl_grid_type * ecl_config_get_grid(const ecl_config_type * ecl_config)
{
  return ecl_config->grid;
}

const char * ecl_config_get_gridfile(const ecl_config_type * ecl_config)
{
  if (ecl_config->grid == NULL )
    return NULL ;
  else
    return ecl_grid_get_name(ecl_config->grid);
}

/**
   The ecl_config object isolated supports run-time changing of the
   grid, however this does not (in general) apply to the system as a
   whole. Other objects which internalize pointers (i.e. field_config
   objects) to an ecl_grid_type instance will be left with dangling
   pointers; and things will probably die an ugly death. So - changing
   grid runtime should be done with extreme care.
*/

void ecl_config_set_grid(ecl_config_type * ecl_config, const char * grid_file)
{
  if (ecl_config->grid != NULL )
    ecl_grid_free(ecl_config->grid);
  ecl_config->grid = ecl_grid_alloc(grid_file);
}

ui_return_type * ecl_config_validate_grid( const ecl_config_type * ecl_config , const char * grid_file ) {
  ui_return_type * ui_return;
  if (util_file_exists( grid_file )) {
    ecl_file_enum file_type = ecl_util_get_file_type( grid_file , NULL , NULL );
    if ((file_type == ECL_EGRID_FILE) || (file_type == ECL_GRID_FILE))
      ui_return =  ui_return_alloc( UI_RETURN_OK );
    else {
      ui_return =  ui_return_alloc( UI_RETURN_FAIL );
      ui_return_add_error( ui_return , "Input argument is not a GRID/EGRID file");
    }
  } else {
    ui_return =  ui_return_alloc( UI_RETURN_FAIL );
    ui_return_add_error( ui_return , "Input argument does not exist.");
  }
  return ui_return;
}



ecl_refcase_list_type * ecl_config_get_refcase_list(const ecl_config_type * ecl_config)
{
  return ecl_config->refcase_list;
}

const ecl_sum_type * ecl_config_get_refcase(const ecl_config_type * ecl_config)
{
  return ecl_refcase_list_get_default(ecl_config->refcase_list);
}

bool ecl_config_has_refcase(const ecl_config_type * ecl_config)
{
  const ecl_sum_type * refcase = ecl_config_get_refcase(ecl_config);
  if (refcase)
    return true;
  else
    return false;
}

ecl_io_config_type * ecl_config_get_io_config(const ecl_config_type * ecl_config)
{
  return ecl_config->io_config;
}

const path_fmt_type * ecl_config_get_eclbase_fmt(const ecl_config_type * ecl_config)
{
  return ecl_config->eclbase;
}

sched_file_type * ecl_config_get_sched_file(const ecl_config_type * ecl_config)
{
  return ecl_config->sched_file;
}


const char * ecl_config_get_schedule_target(const ecl_config_type * ecl_config)
{
  return ecl_config->schedule_target_file;
}

int ecl_config_get_num_restart_files(const ecl_config_type * ecl_config)
{
  return sched_file_get_num_restart_files(ecl_config->sched_file);
}

bool ecl_config_get_formatted(const ecl_config_type * ecl_config)
{
  return ecl_io_config_get_formatted(ecl_config->io_config);
}
bool ecl_config_get_unified_restart(const ecl_config_type * ecl_config)
{
  return ecl_io_config_get_unified_restart(ecl_config->io_config);
}
bool ecl_config_get_unified_summary(const ecl_config_type * ecl_config)
{
  return ecl_io_config_get_unified_summary(ecl_config->io_config);
}

void ecl_config_static_kw_init(ecl_config_type * ecl_config, const config_content_type * config)
{
  if (config_content_has_item( config , STATIC_KW_KEY)) {
    const config_content_item_type * content_item = config_content_get_item(config, STATIC_KW_KEY);
    int j;
    for (j = 0; j < config_content_item_get_size(content_item); j++) {
      const config_content_node_type * content_node = config_content_item_iget_node(content_item, j);
      int k;
      for (k = 0; k < config_content_node_get_size(content_node); k++)
        ecl_config_add_static_kw(ecl_config, config_content_node_iget(content_node, k));
    }
  }
}

void ecl_config_add_config_items(config_parser_type * config)
{
  config_schema_item_type * item;

  item = config_add_schema_item(config, SCHEDULE_FILE_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 2);
  config_schema_item_iset_type(item, 0, CONFIG_EXISTING_PATH);
  /*
   Observe that SCHEDULE_PREDICTION_FILE - which is implemented as a
   GEN_KW is added in ensemble_config.c
   */

  item = config_add_schema_item(config, IGNORE_SCHEDULE_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);
  config_schema_item_iset_type(item, 0, CONFIG_BOOL);

  item = config_add_schema_item(config, ECLBASE_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);

  item = config_add_schema_item(config, DATA_FILE_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);
  config_schema_item_iset_type(item, 0, CONFIG_EXISTING_PATH);

  item = config_add_schema_item(config, STATIC_KW_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item(config, ADD_FIXED_LENGTH_SCHEDULE_KW_KEY, false);
  config_schema_item_set_argc_minmax(item, 2, 2);
  config_schema_item_iset_type(item, 1, CONFIG_INT);

  item = config_add_schema_item(config, REFCASE_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);
  config_schema_item_iset_type(item, 0, CONFIG_PATH);

  item = config_add_schema_item(config, REFCASE_LIST_KEY, false);
  config_schema_item_set_default_type(item, CONFIG_PATH);

  item = config_add_schema_item(config, GRID_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);
  config_schema_item_iset_type(item, 0, CONFIG_EXISTING_PATH);

  item = config_add_schema_item(config, INIT_SECTION_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);
  config_schema_item_iset_type(item, 0, CONFIG_PATH);
  config_add_alias(config, INIT_SECTION_KEY, "EQUIL_INIT_FILE");

  item = config_add_schema_item(config, END_DATE_KEY, false);
  config_schema_item_set_argc_minmax(item, 1, 1);
}

void ecl_config_fprintf_config(const ecl_config_type * ecl_config, FILE * stream)
{
  fprintf(stream, CONFIG_COMMENTLINE_FORMAT);
  fprintf(stream, CONFIG_COMMENT_FORMAT, "Here comes configuration information related to the ECLIPSE model.");

  fprintf(stream, CONFIG_KEY_FORMAT, DATA_FILE_KEY);
  fprintf(stream, CONFIG_ENDVALUE_FORMAT, ecl_config->data_file);

  fprintf(stream, CONFIG_KEY_FORMAT, SCHEDULE_FILE_KEY);
  fprintf(stream, CONFIG_ENDVALUE_FORMAT, sched_file_iget_filename(ecl_config->sched_file, 0));

  fprintf(stream, CONFIG_KEY_FORMAT, ECLBASE_KEY);
  fprintf(stream, CONFIG_ENDVALUE_FORMAT, path_fmt_get_fmt(ecl_config->eclbase));

  if (ecl_config->include_all_static_kw)
  {
    fprintf(stream, CONFIG_KEY_FORMAT, STATIC_KW_KEY);
    fprintf(stream, CONFIG_ENDVALUE_FORMAT, DEFAULT_ALL_STATIC_KW);
  }
  {
    int size = stringlist_get_size(ecl_config->user_static_kw);
    if (size > 0)
    {
      int i;
      fprintf(stream, CONFIG_KEY_FORMAT, STATIC_KW_KEY);
      for (i = 0; i < size; i++)
        if (i < (size - 1))
          fprintf(stream, CONFIG_VALUE_FORMAT, stringlist_iget(ecl_config->user_static_kw, i));
        else
          fprintf(stream, CONFIG_ENDVALUE_FORMAT, stringlist_iget(ecl_config->user_static_kw, i));
    }
  }

  /*
   if (ecl_config->refcase != NULL) {
   fprintf( stream , CONFIG_KEY_FORMAT      , REFCASE_KEY );
   fprintf( stream , CONFIG_ENDVALUE_FORMAT , ecl_config_get_refcase_name( ecl_config ));
   }
   */

  if (ecl_config->grid != NULL )
  {
    fprintf(stream, CONFIG_KEY_FORMAT, GRID_KEY);
    fprintf(stream, CONFIG_ENDVALUE_FORMAT, ecl_config_get_gridfile(ecl_config));
  }

  if (ecl_config->schedule_prediction_file != NULL )
  {
    fprintf(stream, CONFIG_KEY_FORMAT, SCHEDULE_PREDICTION_FILE_KEY);
    fprintf(stream, CONFIG_ENDVALUE_FORMAT, ecl_config_get_schedule_prediction_file(ecl_config));
  }

  if (ecl_config->init_section != NULL )
  {
    fprintf(stream, CONFIG_KEY_FORMAT, INIT_SECTION_KEY);
    fprintf(stream, CONFIG_ENDVALUE_FORMAT, ecl_config_get_init_section(ecl_config));
  }

  {
    hash_iter_type * iter = hash_iter_alloc(ecl_config->fixed_length_kw);
    while (!hash_iter_is_complete(iter))
    {
      const char * kw = hash_iter_get_next_key(iter);
      int length = hash_get_int(ecl_config->fixed_length_kw, kw);

      fprintf(stream, CONFIG_KEY_FORMAT, ADD_FIXED_LENGTH_SCHEDULE_KW_KEY);
      fprintf(stream, CONFIG_VALUE_FORMAT, kw);
      fprintf(stream, CONFIG_INT_FORMAT, length);
      fprintf(stream, "\n");

    }
    hash_iter_free(iter);
  }

  fprintf(stream, "\n\n");
}

/* Units as specified in the ECLIPSE technical manual */
const char * ecl_config_get_depth_unit(const ecl_config_type * ecl_config)
{
  switch(ecl_config->unit_system) {
  case ERT_ECL_METRIC_UNITS:
    return "M";
  case ERT_ECL_FIELD_UNITS:
    return "FT";
  case ERT_ECL_LAB_UNITS:
    return "CM";
  default:
    util_abort("%s: unit system enum value:%d not recognized \n",__func__ , ecl_config->unit_system);
    return NULL;
  }
}


const char * ecl_config_get_pressure_unit(const ecl_config_type * ecl_config)
{
  switch(ecl_config->unit_system) {
  case ERT_ECL_METRIC_UNITS:
    return "BARSA";
  case ERT_ECL_FIELD_UNITS:
    return "PSIA";
  case ERT_ECL_LAB_UNITS:
    return "ATMA";
  default:
    util_abort("%s: unit system enum value:%d not recognized \n",__func__ , ecl_config->unit_system);
    return NULL;
  }
}
