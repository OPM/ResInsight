#include "gtest/gtest.h"

#include "CompletionExportCommands/RicCompletionDataExportTools.h"
#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"

#include "RifTextDataTableFormatter.h"

#include "RigCompletionData.h"

#include <QRegularExpression>
#include <QString>
#include <QTextStream>

#include <cmath>

namespace
{
RigCompletionData
    makePerforation( double transmissibility, double diameter, double skin, double dFactor, double startMD = 100.0, double endMD = 110.0 )
{
    RigCompletionData completion( "W", RigCompletionDataGridCell( 0, nullptr ), startMD );
    completion.setTransAndWPImultBackgroundDataFromPerforation( transmissibility,
                                                                skin,
                                                                diameter,
                                                                dFactor,
                                                                RigCompletionData::defaultValue(),
                                                                RigCompletionData::CellDirection::DIR_K );
    completion.setDepthRange( startMD, endMD );
    return completion;
}

const RicExportCompletionDataSettingsUi& explicitTransSettings()
{
    static RicExportCompletionDataSettingsUi settings;
    settings.compdatExport = RicExportCompletionDataSettingsUi::CompdatExport::TRANSMISSIBILITIES;
    return settings;
}

QString exportTable( void ( *exporter )( RifTextDataTableFormatter&, const QString&, const std::vector<RigCompletionData>& ),
                     const std::vector<RigCompletionData>& completions )
{
    QString                   text;
    QTextStream               stream( &text );
    RifTextDataTableFormatter formatter( stream );
    exporter( formatter, QString(), completions );
    return text;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CombineSumsTransmissibilityAndWeightsValues )
{
    const auto combined = RicCompletionDataExportTools::combineEclipseCellCompletions( { makePerforation( 1.0, 0.2, 1.0, 2.0 ),
                                                                                         makePerforation( 3.0, 0.4, 5.0, 4.0 ) },
                                                                                       explicitTransSettings() );

    EXPECT_DOUBLE_EQ( 4.0, combined.transmissibility() );
    EXPECT_DOUBLE_EQ( ( 0.2 * 1.0 + 0.4 * 3.0 ) / 4.0, combined.diameter() );
    EXPECT_DOUBLE_EQ( ( 1.0 * 1.0 + 5.0 * 3.0 ) / 4.0, combined.skinFactor() );
    EXPECT_EQ( RigCompletionData::CompletionType::PERFORATION, combined.completionType() );
}

//--------------------------------------------------------------------------------------------------
/// The combined D-factor is the transmissibility weighted mean divided by the number of completions
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CombineDFactorIsWeightedMeanDividedByCount )
{
    const auto combined = RicCompletionDataExportTools::combineEclipseCellCompletions( { makePerforation( 1.0, 0.2, 0.0, 2.0 ),
                                                                                         makePerforation( 3.0, 0.2, 0.0, 4.0 ) },
                                                                                       explicitTransSettings() );

    EXPECT_DOUBLE_EQ( ( 2.0 * 1.0 + 4.0 * 3.0 ) / 4.0 / 2.0, combined.dFactor() );

    const auto single =
        RicCompletionDataExportTools::combineEclipseCellCompletions( { makePerforation( 2.0, 0.2, 0.0, 3.0 ) }, explicitTransSettings() );
    EXPECT_DOUBLE_EQ( 3.0, single.dFactor() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CombineMergesMeasuredDepthRange )
{
    const auto combined = RicCompletionDataExportTools::combineEclipseCellCompletions( { makePerforation( 1.0, 0.2, 0.0, 1.0, 100.0, 110.0 ),
                                                                                         makePerforation( 1.0, 0.2, 0.0, 1.0, 105.0, 130.0 ) },
                                                                                       explicitTransSettings() );

    ASSERT_TRUE( combined.startMD().has_value() );
    ASSERT_TRUE( combined.endMD().has_value() );
    EXPECT_DOUBLE_EQ( 100.0, combined.startMD().value() );
    EXPECT_DOUBLE_EQ( 130.0, combined.endMD().value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CombineIgnoresNegativeTransmissibility )
{
    const auto combined = RicCompletionDataExportTools::combineEclipseCellCompletions( { makePerforation( 2.0, 0.2, 0.0, 1.0 ),
                                                                                         makePerforation( -1.0, 0.2, 0.0, 1.0 ) },
                                                                                       explicitTransSettings() );

    EXPECT_DOUBLE_EQ( 2.0, combined.transmissibility() );
    ASSERT_FALSE( combined.metadata().empty() );
    EXPECT_EQ( "ERROR", combined.metadata().front().name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CombineDifferentWellsGivesUnexportedCompletion )
{
    RigCompletionData other = makePerforation( 1.0, 0.2, 0.0, 1.0 );
    RigCompletionData otherWell( "OTHER", RigCompletionDataGridCell( 0, nullptr ), 100.0 );
    otherWell.setTransAndWPImultBackgroundDataFromPerforation( 1.0, 0.0, 0.2, 1.0, 1.0, RigCompletionData::CellDirection::DIR_K );

    const auto combined = RicCompletionDataExportTools::combineEclipseCellCompletions( { other, otherWell }, explicitTransSettings() );

    EXPECT_TRUE( RigCompletionData::isDefaultValue( combined.transmissibility() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CombineHugeTransmissibilityGivesUnexportedCompletion )
{
    const auto combined = RicCompletionDataExportTools::combineEclipseCellCompletions( { makePerforation( HUGE_VAL, 0.2, 0.0, 1.0 ) },
                                                                                       explicitTransSettings() );

    EXPECT_DOUBLE_EQ( 0.0, combined.transmissibility() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, MainGridCompletionsKeepsCellsWithoutLgr )
{
    std::vector<RigCompletionData> completions = { makePerforation( 1.0, 0.2, 0.0, 1.0 ), makePerforation( 2.0, 0.2, 0.0, 1.0 ) };

    EXPECT_EQ( 2u, RicCompletionDataExportTools::mainGridCompletions( completions ).size() );
    EXPECT_TRUE( RicCompletionDataExportTools::subGridsCompletions( completions ).empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, AppendCompletionDataGroupsByGlobalCellIndex )
{
    std::map<size_t, std::vector<RigCompletionData>> grouped;

    RicCompletionDataExportTools::appendCompletionData( &grouped,
                                                        { makePerforation( 1.0, 0.2, 0.0, 1.0 ), makePerforation( 2.0, 0.2, 0.0, 1.0 ) } );
    RicCompletionDataExportTools::appendCompletionData( &grouped, { makePerforation( 3.0, 0.2, 0.0, 1.0 ) } );

    ASSERT_EQ( 1u, grouped.size() );
    EXPECT_EQ( 3u, grouped.begin()->second.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CompdatUsesDefaultMarkerWithoutDFactor )
{
    const QString text = exportTable( RicCompletionDataExportTools::exportCompdatTableUsingFormatter,
                                      { makePerforation( 1.5, 0.2, 0.0, RigCompletionData::defaultValue() ) } );

    EXPECT_TRUE( text.contains( "COMPDAT" ) ) << text.toStdString();
    EXPECT_TRUE( text.contains( "'Z'" ) ) << text.toStdString();
    // The D-factor column is the last one before the direction and is defaulted
    EXPECT_TRUE( text.contains( QRegularExpression( "1\\*\\s+'Z'" ) ) ) << text.toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CompdatWritesDFactorWhenSet )
{
    auto completion = makePerforation( 1.5, 0.2, 0.0, 2.5 );
    completion.setPerConnectionDfactor();

    const QString text = exportTable( RicCompletionDataExportTools::exportCompdatTableUsingFormatter, { completion } );

    EXPECT_FALSE( text.contains( QRegularExpression( "1\\*\\s+'Z'" ) ) ) << text.toStdString();
    EXPECT_TRUE( text.contains( "-2.5" ) ) << text.toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, CompdatSkipsCompletionsWithoutTransmissibility )
{
    const QString text =
        exportTable( RicCompletionDataExportTools::exportCompdatTableUsingFormatter, { makePerforation( 0.0, 0.2, 0.0, 1.0 ) } );

    EXPECT_FALSE( text.contains( "'Z'" ) ) << text.toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicCompletionDataExportTools, ComplumpOnlyExportsCompletionsWithNumber )
{
    auto withNumber = makePerforation( 1.5, 0.2, 0.0, 1.0 );
    withNumber.setCompletionNumber( 7 );
    auto withoutNumber = makePerforation( 1.5, 0.2, 0.0, 1.0 );

    const QString text = exportTable( RicCompletionDataExportTools::exportComplumpTableUsingFormatter, { withoutNumber } );
    EXPECT_FALSE( text.contains( "7" ) ) << text.toStdString();

    const QString textWithNumber = exportTable( RicCompletionDataExportTools::exportComplumpTableUsingFormatter, { withNumber } );
    EXPECT_TRUE( textWithNumber.contains( "COMPLUMP" ) );
    EXPECT_TRUE( textWithNumber.contains( QRegularExpression( "\\s7\\s" ) ) ) << textWithNumber.toStdString();
}
