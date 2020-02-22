/*
   Copyright (C) 2018  Equinor ASA, Norway.

   The file 'vector_util.h' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdio.h>

#include <vector>
#include <algorithm>

template <class T>
int vector_util_index(const std::vector<T>& vec, T value) {

  int index;
  auto iter = find(vec.begin(), vec.end(), value);
  if (iter == vec.end())
    index = -1;
  else
    index = iter - vec.begin();
  return index;
}


template <class T>
void vector_util_fprintf(const std::vector<T>& vec , FILE * stream , const char * name , const char * fmt) {
  size_t i;
  if (name != NULL)
    fprintf(stream , "%s = [" , name);
  else
    fprintf(stream , "[");

  for (i = 0; i < vec.size(); i++) {
    fprintf(stream , fmt , vec[i]);
    if (i < (vec.size() - 1))
      fprintf(stream , ", ");
  }

  fprintf(stream , "]\n");
}
