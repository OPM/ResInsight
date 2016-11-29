
#include "opm/parser/eclipse/Parser/Parser.hpp"
#include "gtest/gtest.h"

#include "RigCaseData.h"
#include "RifEclipseInputFileTools.h"
#include "RifReaderOpmParserInput.h"

#include <QString>
#include <QTime>
#include <QDebug>


using namespace Opm;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(opm_parser_test, smallCase)
{
/*
   // QString filename = "d:/Models/Statoil/small_ascii/10K_BOX_MSW.GRDECL";
    QString filename = "d:/Models/Statoil/testcase_juli_2011/data/grid_local.grdecl";



    size_t iterationCount = 5;

    qDebug() << "ERT reading\n";

    for (size_t i = 0; i < iterationCount; i++)
    {
        QTime time;
        time.start();

        RigCaseData caseData;

        RifEclipseInputFileTools::openGridFile(filename, &caseData, false);

        qDebug() << time.elapsed();
    }


    qDebug() << "OPM reading\n";

    for (size_t i = 0; i < iterationCount; i++)
    {
        QTime time;
        time.start();

        RigCaseData caseData;

        RifReaderOpmParserInput::openGridFile(filename, false, &caseData, false);

        qDebug() << time.elapsed();
    }
*/

}

