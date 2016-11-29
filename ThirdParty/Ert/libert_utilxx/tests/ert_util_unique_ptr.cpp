/*
   Copyright (C) 2016 Statoil ASA, Norway.

   This is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or1
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <ert/util/ert_unique_ptr.hpp>
#include <ert/util/stringlist.h>


void test_stringlist() {
    ERT::ert_unique_ptr<stringlist_type , stringlist_free> stringlist(stringlist_alloc_new( ));
    stringlist_append_copy( stringlist.get() , "Hello");
}


int main(int argc , char **argv) {
    test_stringlist();
}
