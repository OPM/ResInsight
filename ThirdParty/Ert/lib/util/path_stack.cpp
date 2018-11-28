/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'path_stack.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <string>
#include <stack>

#include <ert/util/stringlist.hpp>
#include <ert/util/util.h>
#include <ert/util/path_stack.hpp>

/**
   This file implements the structure path_stack which is vaguely
   inspired by the emacs-lisp scecial form (save-excursion ...). The
   intentiation with the 'class' is to push directories onto a stack,
   and then each time you pop from the stack the the process will
   chdir() back to directory which is popped.

   Observe that the constructor path_stack_alloc() and destructor
   path_stack_free() do *not* alter the stack or the cwd of the
   process.
*/


struct path_stack_struct {
  std::stack<std::string, std::vector<std::string>> stack;
};


/**
   This will create a new path_stack instance; it will push anything
   on the current stack of paths.
*/

path_stack_type * path_stack_alloc() {
  path_stack_type * path_stack = new path_stack_type();
  return path_stack;
}

void path_stack_free( path_stack_type * path_stack ) {
  delete path_stack;
}


/**
   This will push a path on to the stack. The function will start by
   chdir() to the input path. If the chdir() call fails the function
   will return false, and the stack will be unmodified; if the chdir()
   succeeds the input @path will be pushed onto the stack.

   If path is NULL that is interpreted as cwd.
*/

bool path_stack_push( path_stack_type * path_stack , const char * path ) {
  if (path != NULL)
    if (util_chdir( path ) != 0)
      return false;

  path_stack_push_cwd( path_stack );
  return true;
}


void path_stack_push_cwd( path_stack_type * path_stack ) {
  char * cwd = util_alloc_cwd();
  path_stack->stack.push(cwd);
  free(cwd);
}

void path_stack_pop( path_stack_type * path_stack ) {
  const std::string& path = path_stack->stack.top();

  if (util_chdir( path.c_str() ) != 0)
    // The directory has become inaccesible ...
    util_abort("%s: could not pop back to directory:%s Error:%s\n", __func__ , path.c_str() , strerror( errno ));

  path_stack->stack.pop();
}


int path_stack_size( const path_stack_type * path_stack ) {
  return path_stack->stack.size();
}



