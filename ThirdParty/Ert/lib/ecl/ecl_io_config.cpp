/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_io_config.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <stdbool.h>
#include <stdlib.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_io_config.hpp>
#include <ert/ecl/ecl_util.hpp>


/**
   This file implements a pathetically small struct which is used to
   pack three booleans representing eclipse IO configuration. The
   three config items which are stored are:

    * formatted   : whether to use formatted files.
    * unified     : whether unified summary || restart files should be used.

   All types are implemented by an internal enum which supports a
   undefined type. The rationale for this is to provide functionality
   to 'guess' type based on arbitrary input. If for instance the input
   file is formatted, it is impossible to infer whether we should flip
   endian ness.
*/


typedef enum {  UNIFIED        = 0,
                MULTIPLE       = 1,
                UNIF_UNDEFINED = 2 } unified_type;

typedef enum {  FORMATTED     = 0,
                UNFORMATTED   = 1,
                FMT_UNDEFINED = 2 } formatted_type;


struct ecl_io_config_struct {
  formatted_type     formatted;
  unified_type       unified_restart;
  unified_type       unified_summary;
};


/*****************************************************************/

static ecl_io_config_type * ecl_io_config_alloc__() {
  ecl_io_config_type * ecl_io_config = (ecl_io_config_type*)util_malloc(sizeof * ecl_io_config );

  ecl_io_config->formatted       = FMT_UNDEFINED;
  ecl_io_config->unified_restart = UNIF_UNDEFINED;
  ecl_io_config->unified_summary = UNIF_UNDEFINED;

  return ecl_io_config;
}



void ecl_io_config_set_formatted(ecl_io_config_type * io_config, bool formatted) {
  if (formatted)
    io_config->formatted = FORMATTED;
  else
    io_config->formatted = UNFORMATTED;
}



void ecl_io_config_set_unified_restart(ecl_io_config_type * io_config, bool unified) {
  if (unified)
    io_config->unified_restart = UNIFIED;
  else
    io_config->unified_restart = MULTIPLE;
}


void ecl_io_config_set_unified_summary(ecl_io_config_type * io_config, bool unified) {
  if (unified)
    io_config->unified_summary = UNIFIED;
  else
    io_config->unified_summary = MULTIPLE;
}


bool ecl_io_config_get_formatted(ecl_io_config_type * io_config) {
  if (io_config->formatted == FORMATTED)
    return true;
  else if (io_config->formatted == UNFORMATTED)
    return false;
  else {
    util_abort("%s: formatted_state == undefined - sorry \n",__func__);
    return false; /* Compiler shut up */
  }
}


bool ecl_io_config_get_unified_summary(ecl_io_config_type * io_config) {
  if (io_config->unified_summary == UNIFIED)
    return true;
  else if (io_config->unified_summary == MULTIPLE)
    return false;
  else {
    util_abort("%s: unified_state == undefined - sorry \n",__func__);
    return false; /* Compiler shut up */
  }
}

bool ecl_io_config_get_unified_restart(ecl_io_config_type * io_config) {
  if (io_config->unified_restart == UNIFIED)
    return true;
  else if (io_config->unified_restart == MULTIPLE)
    return false;
  else {
    util_abort("%s: formatted_state == undefined - sorry \n",__func__);
    return false; /* Compiler shut up */
  }
}



ecl_io_config_type * ecl_io_config_alloc(bool formatted ,  bool unified_summary , bool unified_restart) {
  ecl_io_config_type * ecl_io_config = ecl_io_config_alloc__();

  ecl_io_config_set_formatted( ecl_io_config , formatted );
  ecl_io_config_set_unified_restart( ecl_io_config , unified_restart );
  ecl_io_config_set_unified_summary( ecl_io_config , unified_summary );

  return ecl_io_config;
}




void ecl_io_config_free(ecl_io_config_type * io_config) {
  free(io_config);
}

