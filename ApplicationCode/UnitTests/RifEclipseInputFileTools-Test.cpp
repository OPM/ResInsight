#include "gtest/gtest.h"

#include "RigEclipseCaseData.h"

#include "RifEclipseInputFileTools.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
/*
TEST(RifEclipseInputFileToolsTest, PathsKeyword)
{
    QString filename = "d:/Models/Statoil/troll_Ref2014/T07-4A-W2014-06.DATA";
    //QString filename = "d:/Models/Statoil/!myTestWithWellLog/TEST10K_FLT_LGR_NNC.DATA";

    std::vector<std::pair<QString, QString>> pathEntries;

    RifEclipseInputFileTools::parseAndReadPathAliasKeyword(filename, &pathEntries);

    for (auto entry : pathEntries)
    {
        qDebug() << entry.first << " " << entry.second;
    }

    std::vector<QString> filenamesWithFaults;
    cvf::Collection<RigFault> faults;
    RifEclipseInputFileTools::readFaultsInGridSection(filename, &faults, &filenamesWithFaults);

}
*/

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseInputFileToolsTest, FaultFaces)
{
    {
        QStringList faceTexts;
        faceTexts << "X" << "X+" << "I" << "I+" << "x" << "x+" << "i" << "i+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach (QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_I, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "X-" << "I-" << "x-" << "i-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NEG_I, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y" << "Y+" << "J" << "J+" << "y" << "y+" << "j" << "j+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_J, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y-" << "J-" << "y-" << "j-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NEG_J, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z" << "Z+" << "K" << "k+" << "z" << "z+" << "k" << "k+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_K, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z-" << "K-" << "z-" << "k-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NEG_K, faceType);
        }
    }


    // Improved parsing handling some special cases
    {
        QStringList faceTexts;
        faceTexts << "Z--" << "z--" << "z/" << " y /";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_NE(cvf::StructGridInterface::NO_FACE, faceType);
        }
    }

    //Invalid faces
    {
        QStringList faceTexts;
        faceTexts << "-k-" << " -k " << "   +k-  ";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NO_FACE, faceType);
        }
    }

    // Valid cases with whitespace
    {
        QStringList faceTexts;
        faceTexts << " X" << " X+ " << " I " << " i+  ";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_I, faceType);
        }
    }

}
