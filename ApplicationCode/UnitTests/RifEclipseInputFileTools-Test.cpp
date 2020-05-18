#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifEclipseInputFileTools.h"
#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include <QDebug>
#include <QFile>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, FaultFaces )
{
    {
        QStringList faceTexts;
        faceTexts << "X"
                  << "X+"
                  << "I"
                  << "I+"
                  << "x"
                  << "x+"
                  << "i"
                  << "i+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_I, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "X-"
                  << "I-"
                  << "x-"
                  << "i-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NEG_I, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y"
                  << "Y+"
                  << "J"
                  << "J+"
                  << "y"
                  << "y+"
                  << "j"
                  << "j+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_J, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y-"
                  << "J-"
                  << "y-"
                  << "j-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NEG_J, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z"
                  << "Z+"
                  << "K"
                  << "k+"
                  << "z"
                  << "z+"
                  << "k"
                  << "k+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_K, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z-"
                  << "K-"
                  << "z-"
                  << "k-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NEG_K, faceType );
        }
    }

    // Improved parsing handling some special cases
    {
        QStringList faceTexts;
        faceTexts << "Z--"
                  << "z--"
                  << "z/"
                  << " y /";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_NE( cvf::StructGridInterface::NO_FACE, faceType );
        }
    }

    // Invalid faces
    {
        QStringList faceTexts;
        faceTexts << "-k-"
                  << " -k "
                  << "   +k-  ";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NO_FACE, faceType );
        }
    }

    // Valid cases with whitespace
    {
        QStringList faceTexts;
        faceTexts << " X"
                  << " X+ "
                  << " I "
                  << " i+  ";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach ( QString text, faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_I, faceType );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, EquilData )
{
    static const QString testDataRootFolder = QString( "%1/ParsingOfDataKeywords/" ).arg( TEST_DATA_DIR );

    {
        QString fileName = testDataRootFolder + "simulation/MY_CASE.DATA";

        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        RifEclipseInputFileTools::parseAndReadPathAliasKeyword( fileName, &pathAliasDefinitions );

        const QString        keyword( "EQUIL" );
        const QString        keywordToStopParsing;
        const qint64         startPositionInFile = 0;
        QStringList          keywordContent;
        std::vector<QString> fileNamesContainingKeyword;
        bool                 isStopParsingKeywordDetected = false;
        const QString        includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   pathAliasDefinitions,
                                                                                   &keywordContent,
                                                                                   &fileNamesContainingKeyword,
                                                                                   &isStopParsingKeywordDetected,
                                                                                   includeStatementAbsolutePathPrefix );
        EXPECT_EQ( (int)10, keywordContent.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, FaultData )
{
    static const QString testDataRootFolder = QString( "%1/ParsingOfDataKeywords/" ).arg( TEST_DATA_DIR );

    {
        QString fileName = testDataRootFolder + "simulation/MY_CASE.DATA";

        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        RifEclipseInputFileTools::parseAndReadPathAliasKeyword( fileName, &pathAliasDefinitions );

        const QString        keyword( "FAULTS" );
        const QString        keywordToStopParsing;
        const qint64         startPositionInFile = 0;
        QStringList          keywordContent;
        std::vector<QString> fileNamesContainingKeyword;
        bool                 isStopParsingKeywordDetected = false;
        const QString        includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   pathAliasDefinitions,
                                                                                   &keywordContent,
                                                                                   &fileNamesContainingKeyword,
                                                                                   &isStopParsingKeywordDetected,
                                                                                   includeStatementAbsolutePathPrefix );

        EXPECT_EQ( (int)977, keywordContent.size() );

        /*
                for (const auto& s : keywordContent)
                {
                    qDebug() << s;
                }
        */
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, StopAtKeyword )
{
    static const QString testDataRootFolder = QString( "%1/ParsingOfDataKeywords/" ).arg( TEST_DATA_DIR );
    QString              fileName           = testDataRootFolder + "simulation/MY_CASE_2.DATA";

    QFile data( fileName );
    if ( !data.open( QFile::ReadOnly ) )
    {
        return;
    }

    {
        const QString                            keyword( "FAULTS" );
        const QString                            keywordToStopParsing( "EDIT" );
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   pathAliasDefinitions,
                                                                                   &keywordContent,
                                                                                   &fileNamesContainingKeyword,
                                                                                   &isStopParsingKeywordDetected,
                                                                                   includeStatementAbsolutePathPrefix );

        EXPECT_TRUE( isStopParsingKeywordDetected );
        EXPECT_TRUE( keywordContent.isEmpty() );
    }

    {
        const QString                            keyword( "EQUIL" );
        const QString                            keywordToStopParsing( "SCHEDULE" );
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   pathAliasDefinitions,
                                                                                   &keywordContent,
                                                                                   &fileNamesContainingKeyword,
                                                                                   &isStopParsingKeywordDetected,
                                                                                   includeStatementAbsolutePathPrefix );
        EXPECT_TRUE( isStopParsingKeywordDetected );
        EXPECT_TRUE( keywordContent.isEmpty() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, DISABLED_FindFilesWithVfp )
{
    QString fileName = "d:/gitroot-ceesol/ResInsight-regression-test/ModelData/norne/NORNE_ATW2013.DATA";

    {
        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        const QString                            keyword( "VFPPROD" );
        const QString                            keywordToStopParsing;
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   pathAliasDefinitions,
                                                                                   &keywordContent,
                                                                                   &fileNamesContainingKeyword,
                                                                                   &isStopParsingKeywordDetected,
                                                                                   includeStatementAbsolutePathPrefix );

        //        EXPECT_TRUE( isStopParsingKeywordDetected );
        //        EXPECT_TRUE( keywordContent.isEmpty() );
    }

    {
        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        const QString                            keyword( "VFPINJ" );
        const QString                            keywordToStopParsing;
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   pathAliasDefinitions,
                                                                                   &keywordContent,
                                                                                   &fileNamesContainingKeyword,
                                                                                   &isStopParsingKeywordDetected,
                                                                                   includeStatementAbsolutePathPrefix );

        //      EXPECT_TRUE( isStopParsingKeywordDetected );
        //        EXPECT_TRUE( keywordContent.isEmpty() );
    }
}
