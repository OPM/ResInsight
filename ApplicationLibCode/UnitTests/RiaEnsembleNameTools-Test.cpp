/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaEnsembleNameTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaEnsembleNameToolsTests, commonBaseName )
{
    QStringList fileNames = { "/base/realization-0/iter-0/stimplan/output/B-H3_StimPlanModel_01_job-01/mainfrac/data_vs_time.csv",
                              "/base/realization-0/iter-0/stimplan/output/B-H3_StimPlanModel_02_job-02/mainfrac/data_vs_time.csv" };

    ASSERT_EQ( "data_vs_time", RiaEnsembleNameTools::findCommonBaseName( fileNames ).toStdString() );
}
