/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////



#include <time.h>
#include <stdbool.h>

#include <util.h>
#include <int_vector.h>
#include <ecl_intehead.h>
#include <ecl_file.h>
#include <ecl_kw.h>
#include <ecl_kw_magic.h>
#include <ecl_util.h>

#include <well_state.h>
#include <well_info.h>
#include <well_conn.h>
#include <well_ts.h>


#include "gtest/gtest.h"

#include "RigCaseData.h"
#include "RifReaderEclipseInput.h"

#if 0
//--------------------------------------------------------------------------------------------------
/// This file contains test code taken from the test cases in ERT source code.
//  There is a typedef issue (center) between ERT and QTextStream, so this file does not include any 
//  Qt files.
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, WellTestErt)
{
    char filename[1024] = "TEST10K_FLT_LGR_NNC.UNRST";

    well_info_type * well_info = well_info_alloc( NULL );
    well_info_load_rstfile( well_info , filename);

    // List all wells:
    {
        int iwell;
        for (iwell = 0; iwell < well_info_get_num_wells( well_info ); iwell++)
        {
            printf("Well[%02d] : %s \n",iwell , well_info_iget_well_name( well_info , iwell));
        }
    }

    // Look at the timeseries for one well:
    {
        well_ts_type * well_ts = well_info_get_ts( well_info , well_info_iget_well_name( well_info , 0));
        for (int i =0; i < well_ts_get_size( well_ts ); i++)
        {
            well_state_type * well_state = well_ts_iget_state( well_ts , i );

            printf("Well:%s  report:%04d  state:",well_state_get_name( well_state ), well_state_get_report_nr( well_state ));
            if (well_state_is_open( well_state ))
                printf("OPEN\n");
            else
                printf("CLOSED\n");
        }
    }

    // Look at one well_state:
    {
        well_state_type * well_state = well_info_iiget_state( well_info , 0 , 0 );
        printf("Well:%s  report:%04d \n",well_state_get_name( well_state ), well_state_get_report_nr( well_state ));
        {
            int branchCount = well_state_get_num_branches(well_state);
            for (int ibranch = 0 ; ibranch < branchCount; ++ibranch)
            {
                printf("Branch: %d", ibranch);
                for (int iconn = 0; iconn < well_state_get_num_connections( well_state, ibranch ); iconn++)
                {
                    const well_conn_type * conn = well_state_get_connections( well_state , ibranch)[iconn];
                    printf("Connection:%02d   i=%3d  j=%3d  k=%3d  State:",iconn , conn->i, conn->j , conn->k);
                    if (conn->open)
                        printf("Open\n");
                    else
                        printf("Closed\n");
                }
            }
        }
    }

    well_info_free( well_info );
}
//--------------------------------------------------------------------------------------------------
/// This file contains test code taken from the test cases in ERT source code.
//  There is a typedef issue (center) between ERT and QTextStream, so this file does not include any 
//  Qt files.
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, ElipseInputGridFile)
{
    RigReservoir res;
    RifReaderEclipseInput inputReader;
    bool result = inputReader.open("TEST10K_FLT_LGR_NNC.grdecl", &res);
    EXPECT_TRUE(result);
    EXPECT_EQ(size_t(1), res.mainGrid()->cells().size());
    EXPECT_EQ(size_t(1), res.mainGrid()->globalMatrixModelActiveCellCount());

}
#endif
