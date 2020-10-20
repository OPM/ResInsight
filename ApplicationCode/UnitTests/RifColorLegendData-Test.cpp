#include "gtest/gtest.h"

#include "RifColorLegendData.h"
#include "RigFormationNames.h"

#include "QDir"
#include "RiaTestDataDirectory.h"
#include <QString>
#include <QStringList>

#include "cvfColor3.h"

TEST( RifColorLegendData, ReadLYRFileWithoutColor )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifColorLegendData/Norne_ATW2013.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QString errormessage;

    cvf::ref<RigFormationNames> fm = RifColorLegendData::readFormationNamesFile( filePath, &errormessage );
    EXPECT_TRUE( errormessage.isEmpty() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 0 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    EXPECT_TRUE( formationName_K1 == "Garn 3" );
    EXPECT_EQ( 1, formationIndex );
}

TEST( RifColorLegendData, ReadLYRFileWithColorName )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifColorLegendData/Norne_ATW2013ColorName.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QString errormessage;

    cvf::ref<RigFormationNames> fm = RifColorLegendData::readFormationNamesFile( filePath, &errormessage );
    EXPECT_TRUE( errormessage.isEmpty() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 1 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    cvf::Color3f formationColor;
    bool         colorPresent = fm->formationColorFromKLayerIdx( 1, &formationColor );
    EXPECT_TRUE( colorPresent );

    EXPECT_TRUE( formationName_K1 == "Garn 2" );
    EXPECT_EQ( 1, formationIndex );
    EXPECT_EQ( 1.0f, formationColor.r() );
    EXPECT_EQ( 0.0f, formationColor.g() );
    EXPECT_EQ( 0.0f, formationColor.b() );
}

TEST( RifColorLegendData, ReadLYRFileWithColorHTML )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifColorLegendData/Norne_ATW2013ColorHTML.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QString errormessage;

    cvf::ref<RigFormationNames> fm = RifColorLegendData::readFormationNamesFile( filePath, &errormessage );
    EXPECT_TRUE( errormessage.isEmpty() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 1 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    cvf::Color3f formationColor;
    bool         colorPresent = fm->formationColorFromKLayerIdx( 1, &formationColor );
    EXPECT_TRUE( colorPresent );

    EXPECT_TRUE( formationName_K1 == "Garn 2" );
    EXPECT_EQ( 1, formationIndex );
    EXPECT_EQ( 1.0f, formationColor.r() );
    EXPECT_EQ( 0.0f, formationColor.g() );
    EXPECT_EQ( 0.0f, formationColor.b() );
}
