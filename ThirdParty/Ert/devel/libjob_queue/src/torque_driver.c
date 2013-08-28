/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'torque_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/job_queue/torque_driver.h>


#define TORQUE_DRIVER_TYPE_ID 34873653
#define TORQUE_JOB_TYPE_ID    12312312

struct torque_driver_struct {
  UTIL_TYPE_ID_DECLARATION;
  char * queue_name;
  char * qsub_cmd;
  char * qstat_cmd;
  char * qdel_cmd;
  char * num_cpus_per_node_char;
  char * num_nodes_char;
  bool keep_qsub_output;
  int num_cpus_per_node;
  int num_nodes;

};

struct torque_job_struct {
  UTIL_TYPE_ID_DECLARATION;
  long int torque_jobnr;
  char * torque_jobnr_char;
};

UTIL_SAFE_CAST_FUNCTION(torque_driver, TORQUE_DRIVER_TYPE_ID);

static UTIL_SAFE_CAST_FUNCTION_CONST(torque_driver, TORQUE_DRIVER_TYPE_ID)
static UTIL_SAFE_CAST_FUNCTION(torque_job, TORQUE_JOB_TYPE_ID)

void * torque_driver_alloc() {
  torque_driver_type * torque_driver = util_malloc(sizeof * torque_driver);
  UTIL_TYPE_ID_INIT(torque_driver, TORQUE_DRIVER_TYPE_ID);

  torque_driver->queue_name = NULL;
  torque_driver->qsub_cmd = NULL;
  torque_driver->qstat_cmd = NULL;
  torque_driver->qdel_cmd = NULL;
  torque_driver->num_cpus_per_node_char = NULL;
  torque_driver->num_nodes_char = NULL;
  torque_driver->keep_qsub_output = false;
  torque_driver->num_cpus_per_node = 1;
  torque_driver->num_nodes = 1;

  torque_driver_set_option(torque_driver, TORQUE_QSUB_CMD, TORQUE_DEFAULT_QSUB_CMD);
  torque_driver_set_option(torque_driver, TORQUE_QSTAT_CMD, TORQUE_DEFAULT_QSTAT_CMD);
  torque_driver_set_option(torque_driver, TORQUE_QDEL_CMD, TORQUE_DEFAULT_QDEL_CMD);
  torque_driver_set_option(torque_driver, TORQUE_NUM_CPUS_PER_NODE, "1");
  torque_driver_set_option(torque_driver, TORQUE_NUM_NODES, "1");

  return torque_driver;
}

static void torque_driver_set_qsub_cmd(torque_driver_type * driver, const char * qsub_cmd) {
  driver->qsub_cmd = util_realloc_string_copy(driver->qsub_cmd, qsub_cmd);
}

static void torque_driver_set_qstat_cmd(torque_driver_type * driver, const char * qstat_cmd) {
  driver->qstat_cmd = util_realloc_string_copy(driver->qstat_cmd, qstat_cmd);
}

static void torque_driver_set_qdel_cmd(torque_driver_type * driver, const char * qdel_cmd) {
  driver->qdel_cmd = util_realloc_string_copy(driver->qdel_cmd, qdel_cmd);
}

static void torque_driver_set_queue_name(torque_driver_type * driver, const char * queue_name) {
  driver->queue_name = util_realloc_string_copy(driver->queue_name, queue_name);
}

static bool torque_driver_set_num_nodes(torque_driver_type * driver, const char* num_nodes_char) {
  int num_nodes = 0;
  if (util_sscanf_int(num_nodes_char, &num_nodes)) {
    driver->num_nodes = num_nodes;
    driver->num_nodes_char = util_realloc_string_copy(driver->num_nodes_char, num_nodes_char);
    return true;
  } else {
    return false;
  }
}

static bool torque_driver_set_keep_qsub_output(torque_driver_type * driver, const char* keep_output_bool_as_char) {
  bool keep_output_parsed;

  if (util_sscanf_bool(keep_output_bool_as_char, &keep_output_parsed)) {
    driver->keep_qsub_output = keep_output_parsed;
    return true;
  } else {
    return false;
  }
}

static bool torque_driver_set_num_cpus_per_node(torque_driver_type * driver, const char* num_cpus_per_node_char) {
  int num_cpus_per_node = 0;
  if (util_sscanf_int(num_cpus_per_node_char, &num_cpus_per_node)) {
    driver->num_cpus_per_node = num_cpus_per_node;
    driver->num_cpus_per_node_char = util_realloc_string_copy(driver->num_cpus_per_node_char, num_cpus_per_node_char);
    return true;
  } else {
    return false;
  }
}

bool torque_driver_set_option(void * __driver, const char * option_key, const void * value) {
  torque_driver_type * driver = torque_driver_safe_cast(__driver);
  bool option_set = true;
  {
    if (strcmp(TORQUE_QSUB_CMD, option_key) == 0)
      torque_driver_set_qsub_cmd(driver, value);
    else if (strcmp(TORQUE_QSTAT_CMD, option_key) == 0)
      torque_driver_set_qstat_cmd(driver, value);
    else if (strcmp(TORQUE_QDEL_CMD, option_key) == 0)
      torque_driver_set_qdel_cmd(driver, value);
    else if (strcmp(TORQUE_QUEUE, option_key) == 0)
      torque_driver_set_queue_name(driver, value);
    else if (strcmp(TORQUE_NUM_CPUS_PER_NODE, option_key) == 0)
      option_set = torque_driver_set_num_cpus_per_node(driver, value);
    else if (strcmp(TORQUE_NUM_NODES, option_key) == 0)
      option_set = torque_driver_set_num_nodes(driver, value);
    else if (strcmp(TORQUE_KEEP_QSUB_OUTPUT, option_key) == 0)
      option_set = torque_driver_set_keep_qsub_output(driver, value);
    else
      option_set = false;
  }
  return option_set;
}

const void * torque_driver_get_option(const void * __driver, const char * option_key) {
  const torque_driver_type * driver = torque_driver_safe_cast_const(__driver);
  {
    if (strcmp(TORQUE_QSUB_CMD, option_key) == 0)
      return driver->qsub_cmd;
    else if (strcmp(TORQUE_QSTAT_CMD, option_key) == 0)
      return driver->qstat_cmd;
    else if (strcmp(TORQUE_QDEL_CMD, option_key) == 0)
      return driver->qdel_cmd;
    else if (strcmp(TORQUE_QUEUE, option_key) == 0)
      return driver->queue_name;
    else if (strcmp(TORQUE_NUM_CPUS_PER_NODE, option_key) == 0)
      return driver->num_cpus_per_node_char;
    else if (strcmp(TORQUE_NUM_NODES, option_key) == 0)
      return driver->num_nodes_char;
    else if (strcmp(TORQUE_KEEP_QSUB_OUTPUT, option_key) == 0)
      return driver->keep_qsub_output ? "1" : "0";
    else {
      util_abort("%s: option_id:%s not recognized for TORQUE driver \n", __func__, option_key);
      return NULL;
    }
  }
}

void torque_driver_init_option_list(stringlist_type * option_list) {
  stringlist_append_ref(option_list, TORQUE_QSUB_CMD);
  stringlist_append_ref(option_list, TORQUE_QSTAT_CMD);
  stringlist_append_ref(option_list, TORQUE_QDEL_CMD);
  stringlist_append_ref(option_list, TORQUE_QUEUE);
  stringlist_append_ref(option_list, TORQUE_NUM_CPUS_PER_NODE);
  stringlist_append_ref(option_list, TORQUE_NUM_NODES);
  stringlist_append_ref(option_list, TORQUE_KEEP_QSUB_OUTPUT);
}

torque_job_type * torque_job_alloc() {
  torque_job_type * job;
  job = util_malloc(sizeof * job);
  job->torque_jobnr_char = NULL;
  job->torque_jobnr = 0;
  UTIL_TYPE_ID_INIT(job, TORQUE_JOB_TYPE_ID);

  return job;
}

stringlist_type * torque_driver_alloc_cmd(torque_driver_type * driver,
        const char * job_name,
        const char * submit_script) {


  stringlist_type * argv = stringlist_alloc_new();

  if (driver->keep_qsub_output) {
    stringlist_append_ref(argv, "-k");
    stringlist_append_ref(argv, "oe");
  }

  {
    char * resource_string = util_alloc_sprintf("nodes=%d:ppn=%d", driver->num_nodes, driver->num_cpus_per_node);
    stringlist_append_ref(argv, "-l");
    stringlist_append_copy(argv, resource_string);
    free(resource_string);
  }

  if (driver->queue_name != NULL) {
    stringlist_append_ref(argv, "-q");
    stringlist_append_ref(argv, driver->queue_name);
  }

  if (job_name != NULL) {
    stringlist_append_ref(argv, "-N");
    stringlist_append_ref(argv, job_name);
  }

  stringlist_append_ref(argv, submit_script);

  return argv;
}

static int torque_job_parse_qsub_stdout(const torque_driver_type * driver, const char * stdout_file) {
  int jobid;
  {
    FILE * stream = util_fopen(stdout_file, "r");
    char * jobid_string = util_fscanf_alloc_upto(stream, ".", false);

    if (jobid_string == NULL || !util_sscanf_int(jobid_string, &jobid)) {

      char * file_content = util_fread_alloc_file_content(stdout_file, NULL);
      fprintf(stderr, "Failed to get torque job id from file: %s \n", stdout_file);
      fprintf(stderr, "qsub command                      : %s \n", driver->qsub_cmd);
      fprintf(stderr, "File content: [%s]\n", file_content);
      free(file_content);
      util_exit("%s: \n", __func__);
    }
    free(jobid_string);
    fclose(stream);
  }
  return jobid;
}

void torque_job_create_submit_script(const char * script_filename, const char * submit_cmd, int argc, const char ** job_argv) {
  if (submit_cmd == NULL) {
    util_abort("%s: cannot create submit script, because there is no executing commmand specified.", __func__);
  }
  
  FILE* script_file = util_fopen(script_filename, "w");
  fprintf(script_file, "#!/bin/sh\n");
  
  fprintf(script_file, "%s", submit_cmd);
  for (int i = 0; i < argc; i++) {

    fprintf(script_file, " %s", job_argv[i]);
  }

  util_fclose(script_file);
}

static int torque_driver_submit_shell_job(torque_driver_type * driver,
        const char * run_path,
        const char * job_name,
        const char * submit_cmd,
        int num_cpu,
        int job_argc,
        const char ** job_argv) {
  int job_id;
  char * tmp_file = util_alloc_tmp_file("/tmp", "enkf-submit", true);
  char * script_filename = util_alloc_filename(run_path, "qsub_script", "sh");
  torque_job_create_submit_script(script_filename, submit_cmd, job_argc, job_argv);
  {
    int p_units_from_driver = driver->num_cpus_per_node * driver->num_nodes;
    if (num_cpu != p_units_from_driver) {
      util_abort("%s: Error in config, job's config requires %d processing units, but config says %s: %d, and %s: %d, which multiplied becomes: %d \n",
              __func__, num_cpu, TORQUE_NUM_CPUS_PER_NODE, driver->num_cpus_per_node, TORQUE_NUM_NODES, driver->num_nodes, p_units_from_driver);
    }
    stringlist_type * remote_argv = torque_driver_alloc_cmd(driver, job_name, script_filename);
    char ** argv = stringlist_alloc_char_ref(remote_argv);
    util_fork_exec(driver->qsub_cmd, stringlist_get_size(remote_argv), (const char **) argv, true, NULL, NULL, NULL, tmp_file, NULL);

    free(argv);
    stringlist_free(remote_argv);
  }

  job_id = torque_job_parse_qsub_stdout(driver, tmp_file);

  util_unlink_existing(tmp_file);
  free(tmp_file);

  return job_id;
}

void torque_job_free(torque_job_type * job) {

  util_safe_free(job->torque_jobnr_char);
  free(job);
}

void torque_driver_free_job(void * __job) {

  torque_job_type * job = torque_job_safe_cast(__job);
  torque_job_free(job);
}

void * torque_driver_submit_job(void * __driver,
        const char * submit_cmd,
        int num_cpu,
        const char * run_path,
        const char * job_name,
        int argc,
        const char ** argv) {
  torque_driver_type * driver = torque_driver_safe_cast(__driver);
  torque_job_type * job = torque_job_alloc();
  {
    job->torque_jobnr = torque_driver_submit_shell_job(driver, run_path, job_name, submit_cmd, num_cpu, argc, argv);
    job->torque_jobnr_char = util_alloc_sprintf("%ld", job->torque_jobnr);
  }

  if (job->torque_jobnr > 0)
    return job;
  else {
    /*
      The submit failed - the queue system shall handle
      NULL return values.
     */
    torque_job_free(job);

    return NULL;
  }
}

static char* torque_driver_get_qstat_status(torque_driver_type * driver, char * jobnr_char) {
  char * status = util_malloc(sizeof (char)*2);
  char * tmp_file = util_alloc_tmp_file("/tmp", "enkf-qstat", true);

  {
    char ** argv = util_calloc(1, sizeof * argv);
    argv[0] = jobnr_char;

    util_fork_exec(driver->qstat_cmd, 1, (const char **) argv, true, NULL, NULL, NULL, tmp_file, NULL);
    free(argv);
  }

  FILE *stream = util_fopen(tmp_file, "r");
  bool at_eof = false;
  util_fskip_lines(stream, 2);
  char * line = util_fscanf_alloc_line(stream, &at_eof);
  fclose(stream);

  if (line != NULL) {
    char job_id_full_string[32];
    if (sscanf(line, "%s %*s %*s %*s %s %*s", job_id_full_string, status) == 2) {
      char *dotPtr = strchr(job_id_full_string, '.');
      int dotPosition = dotPtr - job_id_full_string;
      char* job_id_as_char_ptr = util_alloc_substring_copy(job_id_full_string, 0, dotPosition);
      if (strcmp(job_id_as_char_ptr, jobnr_char) != 0) {
        util_abort("%s: Job id input (%d) does not match the one found by qstat (%d)\n", __func__, jobnr_char, job_id_as_char_ptr);
      }
      free(job_id_as_char_ptr);
    }
    free(line);
  } else {
    util_abort("%s: Unable to read qstat's output line number 3 from file: %s", __func__, tmp_file);
  }

  util_unlink_existing(tmp_file);
  free(tmp_file);

  return status;
}

job_status_type torque_driver_get_job_status(void * __driver, void * __job) {
  torque_driver_type * driver = torque_driver_safe_cast(__driver);
  torque_job_type * job = torque_job_safe_cast(__job);
  char * status = torque_driver_get_qstat_status(driver, job->torque_jobnr_char);
  int result = JOB_QUEUE_FAILED;
  if (strcmp(status, "R") == 0) {
    result = JOB_QUEUE_RUNNING;
  } else if (strcmp(status, "E") == 0) {
    result = JOB_QUEUE_DONE;
  } else if (strcmp(status, "C") == 0) {
    result = JOB_QUEUE_DONE;
  } else if (strcmp(status, "Q") == 0) {
    result = JOB_QUEUE_PENDING;
  } else {
    util_abort("%s: Unknown status found (%s), expecting one of R, E, C and Q.\n", __func__, status);
  }
  free(status);

  return result;
}

void torque_driver_kill_job(void * __driver, void * __job) {

  torque_driver_type * driver = torque_driver_safe_cast(__driver);
  torque_job_type * job = torque_job_safe_cast(__job);
  util_fork_exec(driver->qdel_cmd, 1, (const char **) &job->torque_jobnr_char, true, NULL, NULL, NULL, NULL, NULL);
}

void torque_driver_free(torque_driver_type * driver) {

  util_safe_free(driver->queue_name);
  free(driver->qdel_cmd);
  free(driver->qstat_cmd);
  free(driver->qsub_cmd);
  free(driver->num_cpus_per_node_char);
  free(driver->num_nodes_char);

  free(driver);
  driver = NULL;
}

void torque_driver_free__(void * __driver) {
  torque_driver_type * driver = torque_driver_safe_cast(__driver);
  torque_driver_free(driver);
}

