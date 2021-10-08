/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifEclipseTextFileReader.h"

#include "mio/single_include/mio/mio.hpp"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseTextFileReader, ReadKeywordsAndValues )
{
    QString baseFolder = QString( "%1/RifReaderEclipseOutput/TEST10K_FLT_LGR_NNC_OUT.GRDECL" ).arg( TEST_DATA_DIR );

    std::string filename = baseFolder.toStdString();

    //    "e:/gitroot-ceesol/ResInsight-regression-test/ModelData/TestCase_Ascii_no_map_axis/geocell.grdecl";
    // filename = "d:/scratch/R5_H25_C1_aug_grid.grdecl";

    std::error_code error;

    mio::mmap_sink   rw_mmap    = mio::make_mmap_sink( filename, 0, mio::map_entire_file, error );
    std::string_view stringData = rw_mmap.data();

    std::string         keywordName;
    std::string         line;
    std::vector<double> values;
    bool                endOfKeyword = false;

    size_t offset    = 0;
    size_t bytesRead = 0;

    while ( offset < stringData.size() )
    {
        RifEclipseTextFileReader reader;
        auto [keyword, values] = reader.readKeywordAndValues( stringData, offset, bytesRead );
        offset += bytesRead;

        std::cout << keyword << " : " << values.size() << "\n";

        if ( !values.empty() )
        {
            std::cout << values.front() << " " << values.back();
            std::cout << "\n\n";
        }
    }
}
