/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

/*
#include "ert/ecl/ecl_file.h"
#include "gtest/gtest.h"


//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, WellTestErt)
{

    {
        char filename[1024] =
"d:/Models/Statoil_multipleRealisations/MultipleRealisations/Case_with_10_timesteps/Real0/BRUGGE_0000.X0000";
ecl_file_type* ecl_file = ecl_file_open(filename, ECL_FILE_CLOSE_STREAM); ecl_file_close(ecl_file);
    }

    {
        char filename[1024] = "d:/Models/Statoil/kapoCoulibaly/data/15K_080910_AS1.X0000";
        ecl_file_type* ecl_file = ecl_file_open(filename, ECL_FILE_CLOSE_STREAM);
        if (ecl_file) ecl_file_close(ecl_file);
    }

    {
        char filename[1024] = "d:/Models/Statoil/kapoCoulibaly/data/15K_080910_AS1.X0000";
        ecl_file_type* ecl_file = ecl_file_open(filename, ECL_FILE_WRITABLE);
        if (ecl_file) ecl_file_close(ecl_file);
    }
}
*/

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

#endif

#if 0
//--------------------------------------------------------------------------------------------------
/// This file contains test code taken from the test cases in ERT source code.
//  There is a typedef issue (center) between ERT and QTextStream, so this file does not include any 
//  Qt files.
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, ElipseInputGridFile)
{
    RigCaseData res;
    RifReaderEclipseInput inputReader;
    bool result = inputReader.open("TEST10K_FLT_LGR_NNC.grdecl", &res);
    EXPECT_TRUE(result);
}


TEST(RigReservoirTest, ReadFaults)
{
//     QString filename("d:/Models/Statoil/testcase_juli_2011/data/grid_local.grdecl");
// 
//     std::vector< RifKeywordAndFilePos > fileKeywords;
//     RifEclipseInputFileTools::findKeywordsOnFile(filename, fileKeywords);
//    
//     cvf::Collection<RigFault> faults;
//     
//     RifEclipseInputFileTools::readFaults(filename, faults, fileKeywords);

//     for (size_t j = 0; j < faults.size(); j++)
//     {
//         printf(faults.at(j)->name().toLatin1());
//         printf("\n");
//     }

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, ReadFaultsRecursively)
{
    //TODO: Establish a way to define location of test model files

    //QString filename("d:/Models/Statoil/TEST_RKMFAULTS/TEST_RKMFAULTS.DATA");
    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.DATA");
    
//    QString filename("d:/gitroot/ResInsight/TestModels/fault_test/regular27cell.DATA");

    QString outFilename = "d:/tmp/msj_faults.txt";
    QFile outputFile(outFilename);
    {
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            return;
        }
    }

    QTextStream outStream(&outputFile);

    cvf::Collection<RigFault> faults;

    std::vector<QString> filenamesWithFaults;
    RifEclipseInputFileTools::readFaultsInGridSection(filename, faults, filenamesWithFaults);

//    EXPECT_EQ(4, faults.size());

    for (size_t j = 0; j < faults.size(); j++)
    {
        const RigFault* rigFault = faults.at(j);
        
        printf(rigFault->name().toLatin1());

        for (size_t faceType = 0; faceType < 6; faceType++)
        {
            cvf::StructGridInterface::FaceType faceEnum = cvf::StructGridInterface::FaceType(faceType);
//             const std::vector<cvf::CellRange>& cellRanges = rigFault->cellRangeForFace(faceEnum);
// 
//             for (size_t i = 0; i < cellRanges.size(); i++)
//             {
//                 cvf::Vec3st min, max;
//                 cellRanges[i].range(min, max);
// 
//                 QString tmp;
//                 tmp = tmp.sprintf("min i=%3d  j=%3d  k=%3d  -  max  i=%3d  j=%3d  k=%3d  \n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
//                 
//                 outStream << tmp;
// 
// //                 printf("min i=%3d  j=%3d  k=%3d  -  max  i=%3d  j=%3d  k=%3d  ", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
// //                 printf("\n");
//             }
        }
    }
}

#endif
