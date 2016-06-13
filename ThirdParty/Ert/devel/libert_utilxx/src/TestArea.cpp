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

#include <ert/util/util.h>
#include <ert/util/TestArea.hpp>

namespace ERT {

  TestArea::TestArea( )
  {
  }

  TestArea::TestArea( const std::string& name )
  {
    enter( name );
  }

  void TestArea::enter( const std::string& name ) {
    c_ptr.reset( test_work_area_alloc( name.c_str() ));
  }


  void TestArea::assertOpen() const {
    if (!c_ptr)
      throw std::runtime_error("Must call TestArea::enter( \"name\" ) before invoking copy operations");
  }

  void TestArea::assertFileExists( const std::string& filename ) const {
    assertOpen();
    if (!util_is_file( filename.c_str() ))
      throw std::invalid_argument("File " + filename + " does not exist ");
  }

  void TestArea::assertDirectoryExists( const std::string& directory ) const {
    assertOpen();
    if (!util_is_directory( directory.c_str() ))
      throw std::invalid_argument("Directory " + directory  + " does not exist ");
  }

  void TestArea::assertEntryExists( const std::string& entry ) const {
    assertOpen();
    if (!util_entry_exists( entry.c_str() ))
      throw std::invalid_argument("Entry " + entry+ " does not exist ");
  }


  void TestArea::copyFile( const std::string& filename ) const {
    assertFileExists( filename );
    test_work_area_copy_file( c_ptr.get() , filename.c_str() );
  }


  void TestArea::copyDirectory( const std::string& directory) const {
    assertDirectoryExists( directory );
    test_work_area_copy_directory( c_ptr.get() , directory.c_str() );
  }

  void TestArea::copyDirectoryContent( const std::string& directory) const {
    assertDirectoryExists( directory );
    test_work_area_copy_directory_content( c_ptr.get() , directory.c_str() );
  }

  void TestArea::copyParentContent( const std::string& entry ) const {
    assertEntryExists( entry );
    test_work_area_copy_parent_content( c_ptr.get() , entry.c_str() );
  }

  void TestArea::copyParentDirectory( const std::string& entry ) const {
    assertEntryExists( entry );
    test_work_area_copy_parent_directory( c_ptr.get() , entry.c_str() );
  }

}

