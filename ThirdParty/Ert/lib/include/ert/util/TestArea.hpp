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

#ifndef ERT_TEST_AREA_CXX
#define ERT_TEST_AREA_CXX

#include <string>

#include <ert/util/test_work_area.hpp>

#include <ert/util/ert_unique_ptr.hpp>

/*
  The TestArea class will create a random temporary and call chdir()
  into this directory. When the TestArea instance goes out of scope
  the directory will be removed.

  In addition to the cleanup the main feature of the TestArea
  implementation are in the copyXXX() methods, these methods all
  interpret the input argument relative to the original cwd. This is
  quite convenient when you need to copy input files to the test
  directory.
*/


namespace ERT {

    class TestArea {
    public:
      TestArea( );
      TestArea( const std::string& name );
      void enter(const std::string& name );

      /* Will copy one file into work area. */
      void copyFile( const std::string& filename) const;

      /* Will copy a directory with all content recursively into the
	 work area. */
      void copyDirectory( const std::string& directory) const;

      /*
	 Will copy all the content of a directory into the work area, i.e.
	 if your directory looks like:

	    path0/file1.txt
	    path0/file2.txt
	    path0/subdir/file3.txt

         copyDirectoryContet( "path0" ) will copy the files file1.txt,
         file2.txt and the directory subdir into work area.
      */
      void copyDirectoryContent( const std::string& directory) const;

      /*
	Will determine the directory holding the input entry, and copy
	the content of that directory, i.e. the following are
	equivalent:

           copyDirectoryContent("path0") <=> copyParentContent("path0/file1.txt")

      */
      void copyParentContent( const std::string& entry ) const;

      /*
	Will call copyDircetory( ) with the directory holding this
	entry:

	    copyDirectory("path0") <=> copyParentDirectory("path0/file1.txt")
      */
      void copyParentDirectory( const std::string& entry ) const;

      std::string getCwd() const;
      std::string getOriginalCwd() const;
      void setStore(bool store);

    private:
      std::string inputPath( const std::string& path) const;
      void assertOpen( ) const;
      void assertFileExists( const std::string& ) const;
      void assertDirectoryExists( const std::string& ) const;
      void assertEntryExists( const std::string& ) const;
      ert_unique_ptr<test_work_area_type , test_work_area_free> c_ptr;
    };
}

#endif
