/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'hook_manager.h' is part of ERT - Ensemble based Reservoir Tool.

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
#ifndef ERT_HOOK_MANAGER_H
#define ERT_HOOK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>

#include <ert/enkf/hook_workflow.h>
#include <ert/enkf/ert_workflow_list.h>
#include <ert/enkf/runpath_list.h>

  typedef struct hook_manager_struct hook_manager_type;

  hook_manager_type   * hook_manager_alloc(ert_workflow_list_type * workflow_list);
  void                  hook_manager_free();

  void                  hook_manager_init( hook_manager_type * hook_manager , const config_content_type * config);
  void                  hook_manager_add_config_items( config_parser_type * config );

  runpath_list_type   * hook_manager_get_runpath_list( hook_manager_type * hook_manager );
  void                  hook_manager_export_runpath_list( const hook_manager_type * hook_manager );
  void                  hook_manager_set_runpath_list_file( hook_manager_type * hook_manager , const char * path, const char * filename);
  const char          * hook_manager_get_runpath_list_file(const hook_manager_type * hook_manager);
  void                  hook_manager_run_workflows( const hook_manager_type * hook_manager , hook_run_mode_enum run_mode , void * self);

  const hook_workflow_type   * hook_manager_iget_hook_workflow(const hook_manager_type * hook_manager, int index);
  int                          hook_manager_get_size(const hook_manager_type * hook_manager);

  /*****************************************************************/
  /* Deprecated stuff                                              */
  /*****************************************************************/
  bool                  hook_manager_run_post_hook_workflow( const hook_manager_type * hook_manager , void * self);

  void                  hook_manager_set_path( hook_manager_type * hook_manager , const char * path);
  bool                  hook_manager_has_post_hook_workflow( const hook_manager_type * hook_manager );
  const hook_workflow_type * hook_manager_get_post_hook_workflow( const hook_manager_type * hook_manager );


#ifdef __cplusplus
}
#endif
#endif
