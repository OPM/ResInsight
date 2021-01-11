#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifColorLegendData.h"
#include "RigFormationNames.h"
#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimRegularLegendConfig.h"

#include "cvfColor3.h"

#include <QDir>
#include <QString>
#include <QStringList>

TEST( RifColorLegendData, ReadLYRFileWithoutColor )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifColorLegendData/Norne_ATW2013.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QString errormessage;

    auto [fm, colorLegend] = RifColorLegendData::readFormationNamesFile( filePath, &errormessage );
    EXPECT_TRUE( errormessage.isEmpty() );
    EXPECT_TRUE( colorLegend.notNull() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 0 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    EXPECT_TRUE( formationName_K1 == "Garn 3" );
    EXPECT_EQ( 1, formationIndex );

    EXPECT_EQ( "Norne_ATW2013", colorLegend->colorLegendName().toStdString()  );
    EXPECT_EQ( 22u, colorLegend->colorLegendItems().size());

    EXPECT_EQ( "Garn 3", colorLegend->colorLegendItems()[0]->categoryName().toStdString());
}

TEST( RifColorLegendData, ReadLYRFileWithColorName )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifColorLegendData/Norne_ATW2013ColorName.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QString errormessage;

    auto [fm, colorLegend] = RifColorLegendData::readFormationNamesFile( filePath, &errormessage );
    EXPECT_TRUE( errormessage.isEmpty() );
    EXPECT_TRUE( colorLegend.notNull() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 1 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    EXPECT_TRUE( formationName_K1 == "Garn 2" );
    EXPECT_EQ( 1, formationIndex );

    EXPECT_EQ( "Garn 2", colorLegend->colorLegendItems()[1]->categoryName().toStdString());
    const cvf::Color3f& formationColor = colorLegend->colorLegendItems()[1]->color();
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

    auto [fm, colorLegend] = RifColorLegendData::readFormationNamesFile( filePath, &errormessage );
    EXPECT_TRUE( errormessage.isEmpty() );
    EXPECT_TRUE( colorLegend.notNull() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 1 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    EXPECT_TRUE( formationName_K1 == "Garn 2" );
    EXPECT_EQ( 1, formationIndex );
    EXPECT_EQ( "Garn 2", colorLegend->colorLegendItems()[1]->categoryName().toStdString());

    const cvf::Color3f& formationColor = colorLegend->colorLegendItems()[1]->color();
    EXPECT_EQ( 1.0f, formationColor.r() );
    EXPECT_EQ( 0.0f, formationColor.g() );
    EXPECT_EQ( 0.0f, formationColor.b() );
}

TEST( RimRegularLegendConfig, LogTenFunctions )
{
    {
        // Negative values will return zero
        double value = -0.0015;

        auto exponentCeil = RimRegularLegendConfig::computeTenExponentCeil( value );
        EXPECT_EQ( 0.0f, exponentCeil );

        auto exponentFloor = RimRegularLegendConfig::computeTenExponentFloor( value );
        EXPECT_EQ( 0.0f, exponentFloor );
    }

    {
        double value = 0.15;

        auto exponentCeil = RimRegularLegendConfig::computeTenExponentCeil( value );
        EXPECT_EQ( 0.0f, exponentCeil );

        auto exponentFloor = RimRegularLegendConfig::computeTenExponentFloor( value );
        EXPECT_EQ( -1.0f, exponentFloor );
    }

    {
        double value = 1.5;

        auto exponentCeil = RimRegularLegendConfig::computeTenExponentCeil( value );
        EXPECT_EQ( 1.0f, exponentCeil );

        auto exponentFloor = RimRegularLegendConfig::computeTenExponentFloor( value );
        EXPECT_EQ( 0.0f, exponentFloor );
    }

    {
        double value = 15;

        auto exponentCeil = RimRegularLegendConfig::computeTenExponentCeil( value );
        EXPECT_EQ( 2.0f, exponentCeil );

        auto exponentFloor = RimRegularLegendConfig::computeTenExponentFloor( value );
        EXPECT_EQ( 1.0f, exponentFloor );
    }
}
