//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//

// The one and only version info for the application
// The application should use STRPRODUCTVER to display product version information
//
// The PRODUCTVER define is used in VersionInfo block fields FILEVERSION, PRODUCTVERSION and "FileVersion"
// The STRPRODUCTVER define is used in VersionInfo block fields "ProductVersion"
// See file "*.rc2"
//
// PRODUCTVER define contains : <major version>, <minor version>, <unused>, <build number>
//
// Externally shipped versions should have even build numbers
#define PRODUCTVER     "@PRODUCTVER@"
#define STRPRODUCTVER  "@STRPRODUCTVER@"

#define RESINSIGHT_MAJOR_VERSION "@RESINSIGHT_MAJOR_VERSION@"
#define RESINSIGHT_MINOR_VERSION "@RESINSIGHT_MINOR_VERSION@"
#define RESINSIGHT_PATCH_VERSION "@RESINSIGHT_PATCH_VERSION@"

#define RESINSIGHT_OCTAVE_VERSION "@OCTAVE_VERSION_STRING@"
