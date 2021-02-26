/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicWellPathFractureTextReportFeatureImpl.h"

#include "RiaRegressionTestRunner.h"

#include "RicExportFractureCompletionsImpl.h"
#include "RicWellPathFractureReportItem.h"

#include "RifTextDataTableFormatter.h"

#include "RigCompletionData.h"
#include "RigTransmissibilityEquations.h"

#include "RimEclipseCase.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFileWellPath.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString orientationText( RimFractureTemplate::FracOrientationEnum orientation )
{
    return caf::AppEnum<RimFractureTemplate::FracOrientationEnum>::uiText( orientation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableColumn floatNumberColumn( const QString& text )
{
    return RifTextDataTableColumn( text, RifTextDataTableDoubleFormatting( RIF_FLOAT, 3 ), RIGHT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::wellPathFractureReport(
    RimEclipseCase*                                   sourceCase,
    const std::vector<RimWellPath*>&                  wellPaths,
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems ) const
{
    QString lineStart = "--";

    QString     text;
    QTextStream textStream( &text );

    textStream
        << lineStart
        << "========================================================================================================\n";

    textStream << lineStart << " RESINSIGHT DATA\n";

    textStream << lineStart << "\n";

    std::vector<RimStimPlanFractureTemplate*> stimPlanTemplates;
    std::vector<RimEllipseFractureTemplate*>  ellipseTemplates;

    {
        auto proj              = RimProject::current();
        auto fractureTemplates = proj->activeOilField()->fractureDefinitionCollection()->fractureTemplates();

        std::set<QString> usedFractureTemplateNames;
        for ( const auto& item : wellPathFractureReportItems )
        {
            usedFractureTemplateNames.insert( item.fractureTemplateName() );
        }

        for ( const auto fracTemplate : fractureTemplates )
        {
            if ( usedFractureTemplateNames.find( fracTemplate->name() ) == usedFractureTemplateNames.end() )
            {
                continue;
            }

            auto stimPlanTemplate = dynamic_cast<RimStimPlanFractureTemplate*>( fracTemplate );
            if ( stimPlanTemplate )
            {
                stimPlanTemplates.push_back( stimPlanTemplate );
            }

            auto ellipseTemplate = dynamic_cast<RimEllipseFractureTemplate*>( fracTemplate );
            if ( ellipseTemplate )
            {
                ellipseTemplates.push_back( ellipseTemplate );
            }
        }
    }

    if ( !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        if ( sourceCase )
        {
            textStream << lineStart << " Grid Model:\n";
            textStream << lineStart << " " << sourceCase->gridFileName() << "\n";
            textStream << lineStart << "\n";
        }

        {
            QString tableText = createWellFileLocationText( wellPaths );
            textStream << tableText;
            textStream << lineStart << "\n";
        }

        {
            QString tableText = createStimPlanFileLocationText( stimPlanTemplates );
            textStream << tableText;
            textStream << lineStart << "\n";
        }
    }

    {
        QString tableText = createEllipseFractureText( ellipseTemplates );
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        QString tableText = createStimPlanFractureText( stimPlanTemplates );
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        std::vector<RimFractureTemplate*> fracTemplates;
        fracTemplates.insert( fracTemplates.end(), ellipseTemplates.begin(), ellipseTemplates.end() );
        fracTemplates.insert( fracTemplates.end(), stimPlanTemplates.begin(), stimPlanTemplates.end() );

        QString tableText = createFractureText( fracTemplates );
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        std::vector<RimWellPathFracture*> wellPathFractures;
        for ( const auto& w : wellPaths )
        {
            for ( const auto& frac : w->fractureCollection()->activeFractures() )
            {
                wellPathFractures.push_back( frac );
            }
        }

        std::sort( wellPathFractures.begin(), wellPathFractures.end(), RimWellPathFracture::compareByWellPathNameAndMD );

        {
            QString tableText = createFractureInstancesText( wellPathFractures );
            textStream << tableText;
            textStream << lineStart << "\n";
        }

        {
            QString tableText = createFractureCompletionSummaryText( wellPathFractureReportItems );
            textStream << tableText;
            textStream << lineStart << "\n";
        }

        {
            QString tableText = createFracturePressureDepletionSummaryText( wellPathFractureReportItems );
            textStream << tableText;
            textStream << lineStart << "\n";
        }

        {
            textStream << lineStart << " Maximum number of connections per well\n";
            textStream << lineStart << "\n";

            QString tableText = createConnectionsPerWellText( wellPathFractureReportItems );
            textStream << tableText;
            textStream << lineStart << "\n";
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathFractureTextReportFeatureImpl::wellPathsWithActiveFractures()
{
    std::vector<RimWellPath*> wellPaths;

    auto* wellPathColl = RimTools::wellPathCollection();
    if ( wellPathColl )
    {
        for ( const auto& wellPath : wellPathColl->allWellPaths() )
        {
            if ( !wellPath->fractureCollection()->activeFractures().empty() )
            {
                wellPaths.push_back( wellPath );
            }
        }
    }

    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createWellFileLocationText( const std::vector<RimWellPath*>& wellPaths ) const
{
    if ( wellPaths.empty() ) return "";

    QString tableText;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "Well" ),
        RifTextDataTableColumn( "Location" ),
    };

    formatter.header( header );

    formatter.addHorizontalLine( '-' );

    if ( !wellPaths.empty() )
    {
        for ( const auto& wellPath : wellPaths )
        {
            auto fileWellPath = dynamic_cast<RimFileWellPath*>( wellPath );
            if ( fileWellPath )
            {
                formatter.add( wellPath->completionSettings()->wellNameForExport() );
                formatter.add( fileWellPath->filePath() );
                formatter.rowCompleted();
            }
        }
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createStimPlanFileLocationText(
    const std::vector<RimStimPlanFractureTemplate*>& stimPlanTemplates ) const
{
    if ( stimPlanTemplates.empty() ) return "";

    QString tableText;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "StimPlan Name" ),
        RifTextDataTableColumn( "Location" ),
    };

    formatter.header( header );

    formatter.addHorizontalLine( '-' );

    if ( !stimPlanTemplates.empty() )
    {
        for ( const auto& stimPlanTemplate : stimPlanTemplates )
        {
            formatter.add( stimPlanTemplate->name() );
            formatter.add( stimPlanTemplate->fileName() );
            formatter.rowCompleted();
        }
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createStimPlanFractureText(
    const std::vector<RimStimPlanFractureTemplate*>& stimPlanTemplates ) const
{
    if ( stimPlanTemplates.empty() ) return "";

    QString tableText;

    RiaDefines::EclipseUnitSystem unitSystem   = stimPlanTemplates.front()->fractureTemplateUnit();
    bool                          isFieldUnits = unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "StimPlan" ),
        RifTextDataTableColumn( " " ),
        floatNumberColumn( "WDiam" ),
        floatNumberColumn( "Skin" ),
    };

    formatter.header( header );

    // Second header line
    {
        formatter.add( "Template" ); // Template
        formatter.add( "Orientation" ); // Orientation
        formatter.add( isFieldUnits ? "[in]" : "[m]" ); // WDiam
        formatter.add( "[] " ); // Skin
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine( '-' );

    for ( const auto& stimPlanTemplate : stimPlanTemplates )
    {
        formatter.add( stimPlanTemplate->name() );
        formatter.add( orientationText( stimPlanTemplate->orientationType() ) );
        formatter.add( stimPlanTemplate->wellDiameter() );
        formatter.add( stimPlanTemplate->skinFactor() );

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createEllipseFractureText(
    const std::vector<RimEllipseFractureTemplate*>& ellipseTemplates ) const
{
    if ( ellipseTemplates.empty() ) return "";

    QString tableText;

    RiaDefines::EclipseUnitSystem unitSystem   = ellipseTemplates.front()->fractureTemplateUnit();
    bool                          isFieldUnits = unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "Ellipse" ),
        RifTextDataTableColumn( " " ),
        floatNumberColumn( "Xf" ),
        floatNumberColumn( "Height" ),
        floatNumberColumn( "Kf" ),
        floatNumberColumn( "Wf" ),
        floatNumberColumn( "WDiam" ),
        floatNumberColumn( "Skin" ),
    };

    formatter.header( header );

    // Second header line
    {
        formatter.add( "Template" ); // Template
        formatter.add( "Orientation" ); // Orientation
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // Xf
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // Height
        formatter.add( "[mD]" ); // Kf
        formatter.add( isFieldUnits ? "[in]" : "[m]" ); // Wf
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // WDiam
        formatter.add( "[] " ); // Skin
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine( '-' );

    for ( const auto& ellipseTemplate : ellipseTemplates )
    {
        formatter.add( ellipseTemplate->name() );
        formatter.add( orientationText( ellipseTemplate->orientationType() ) );

        formatter.add( ellipseTemplate->halfLength() );
        formatter.add( ellipseTemplate->height() );

        formatter.add(
            RigTransmissibilityEquations::permeability( ellipseTemplate->conductivity(), ellipseTemplate->width() ) );
        formatter.add( ellipseTemplate->width() );

        formatter.add( ellipseTemplate->wellDiameter() );
        formatter.add( ellipseTemplate->skinFactor() );

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createFractureText(
    const std::vector<RimFractureTemplate*>& fractureTemplates ) const
{
    if ( fractureTemplates.empty() ) return "";

    QString tableText;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( " " ),
        floatNumberColumn( "Top" ),
        floatNumberColumn( "Bot" ),
        floatNumberColumn( "Fault" ),
        floatNumberColumn( "Height" ),
        floatNumberColumn( "Half Length" ),
        floatNumberColumn( "DFac" ),
        floatNumberColumn( "Conductivity" ),
    };

    formatter.header( header );

    // Second header line
    {
        formatter.add( "Template" );
        formatter.add( "Cont" );
        formatter.add( "Cont" );
        formatter.add( "Truncation" );
        formatter.add( "Scale" );
        formatter.add( "Scale" );
        formatter.add( "Scale" );
        formatter.add( "Scale" );
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine( '-' );

    for ( const auto& fracTemplate : fractureTemplates )
    {
        formatter.add( fracTemplate->name() );

        if ( fracTemplate->fractureContainment()->isEnabled() )
        {
            formatter.add( fracTemplate->fractureContainment()->topKLayer() );
            formatter.add( fracTemplate->fractureContainment()->baseKLayer() );
        }
        else
        {
            formatter.add( "N/A" );
            formatter.add( "N/A" );
        }

        if ( fracTemplate->fractureContainment()->minimumFaultThrow() >= 0.0 )
        {
            formatter.add( fracTemplate->fractureContainment()->minimumFaultThrow() );
        }
        else
        {
            formatter.add( "N/A" );
        }

        double halfLengthScale, heightScale, dfactorScale, conductivityScale;
        fracTemplate->scaleFactors( &halfLengthScale, &heightScale, &dfactorScale, &conductivityScale );
        formatter.add( heightScale );
        formatter.add( halfLengthScale );
        formatter.add( dfactorScale );
        formatter.add( conductivityScale );

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createFractureInstancesText(
    const std::vector<RimWellPathFracture*>& fractures ) const
{
    if ( fractures.empty() ) return "";

    RiaDefines::EclipseUnitSystem unitSystem   = fractures.front()->fractureUnit(); // Fix
    bool                          isFieldUnits = unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD;

    QString tableText;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "" ),
        RifTextDataTableColumn( "" ),
        RifTextDataTableColumn( "" ),
        floatNumberColumn( "MD" ),
        floatNumberColumn( "Dip" ),
        floatNumberColumn( "Tilt" ),
        floatNumberColumn( "LPerf" ),
        floatNumberColumn( "PerfEff" ),
        floatNumberColumn( "Wdia" ),
        RifTextDataTableColumn( "Dfac", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ), RIGHT ),
    };

    formatter.header( header );

    // Second header line
    {
        formatter.add( "Well" );
        formatter.add( "Fracture" );
        formatter.add( "Template" );
        formatter.add( "" ); // MD
        formatter.add( "" ); // Dip
        formatter.add( "" ); // Tilt
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // LPerf
        formatter.add( "[]" ); // PerfEff
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // WDia
        formatter.add( "[...]" ); // Dfac

        formatter.rowCompleted();
    }

    formatter.addHorizontalLine( '-' );

    for ( const auto& fracture : fractures )
    {
        fracture->ensureValidNonDarcyProperties();

        QString wellName;

        RimWellPath* wellPath = nullptr;
        fracture->firstAncestorOrThisOfType( wellPath );
        if ( wellPath )
        {
            wellName = wellPath->completionSettings()->wellNameForExport();
        }

        formatter.add( wellName );
        formatter.add( fracture->name() );

        if ( fracture->fractureTemplate() )
        {
            formatter.add( fracture->fractureTemplate()->name() );
        }
        else
        {
            formatter.add( "N/A" );
        }

        formatter.add( fracture->fractureMD() );
        formatter.add( fracture->dip() );
        formatter.add( fracture->tilt() );

        if ( fracture->fractureTemplate() &&
             fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
        {
            formatter.add( fracture->perforationLength() );
        }
        else
        {
            formatter.add( "N/A" );
        }

        formatter.add( fracture->perforationEfficiency() );
        formatter.add( fracture->wellRadius() * 2.0 );

        if ( fracture->fractureTemplate() && fracture->fractureTemplate()->isNonDarcyFlowEnabled() )
        {
            formatter.add( fracture->nonDarcyProperties().dFactor );
        }
        else
        {
            formatter.add( "N/A" );
        }

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createFractureCompletionSummaryText(
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems ) const
{
    QString tableText;

    RiaDefines::EclipseUnitSystem unitSystem   = wellPathFractureReportItems.front().unitSystem();
    bool                          isFieldUnits = unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    const QString meanText = "Mean";

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "" ), // Well
        RifTextDataTableColumn( "" ), // Fracture
        RifTextDataTableColumn( "" ), // Template
        floatNumberColumn( "" ), // Tr
        floatNumberColumn( "" ), //#con
        floatNumberColumn( "" ), // Fcd
        RifTextDataTableColumn( "", RifTextDataTableDoubleFormatting( RIF_FLOAT, 1 ), RIGHT ), // Area
        RifTextDataTableColumn( meanText, RifTextDataTableDoubleFormatting( RIF_FLOAT, 1 ), RIGHT ), // KfWf
        RifTextDataTableColumn( meanText, RifTextDataTableDoubleFormatting( RIF_FLOAT, 1 ), RIGHT ), // Kf
        floatNumberColumn( meanText ), // wf
        RifTextDataTableColumn( meanText, RifTextDataTableDoubleFormatting( RIF_FLOAT, 1 ), RIGHT ), // xf
        RifTextDataTableColumn( meanText, RifTextDataTableDoubleFormatting( RIF_FLOAT, 1 ), RIGHT ), // H
        floatNumberColumn( meanText ), // Km
    };

    formatter.header( header );

    // Second header line
    {
        formatter.add( "" );
        formatter.add( "" );
        formatter.add( "" );
        formatter.add( "Tr" ); // Tr
        formatter.add( "#con" ); // #con
        formatter.add( "Fcd" ); // Fcd
        formatter.add( "Area" ); // Area
        formatter.add( "KfWf" ); // KfWf
        formatter.add( "Kf" ); // Kf
        formatter.add( "wf" ); // wf
        formatter.add( "Xf" ); // Xf
        formatter.add( "H" ); // H
        formatter.add( "Km" ); // Km
        formatter.rowCompleted();
    }

    // Third header line
    {
        formatter.add( "Well" );
        formatter.add( "Fracture" );
        formatter.add( "Template" );
        formatter.add( isFieldUnits ? "[cP.rb/day/psi]" : "[cP.rm3/day/bars]" ); // Tr
        formatter.add( "" ); // #con
        formatter.add( "[]" ); // Fcd
        formatter.add( isFieldUnits ? "[ft2]" : "[m2]" ); // Area
        formatter.add( isFieldUnits ? "[mDft]" : "[mDm]" ); // KfWf
        formatter.add( "[mD]" ); // Kf
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // wf
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // Xf
        formatter.add( isFieldUnits ? "[ft]" : "[m]" ); // H
        formatter.add( "[mD]" ); // Km
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine( '-' );

    for ( const auto& reportItem : wellPathFractureReportItems )
    {
        formatter.add( reportItem.wellPathNameForExport() );
        formatter.add( reportItem.fractureName() );
        formatter.add( reportItem.fractureTemplateName() );

        formatter.add( reportItem.transmissibility() );
        formatter.add( reportItem.connectionCount() );
        formatter.add( reportItem.fcd() );
        formatter.add( reportItem.area() );

        formatter.add( reportItem.kfwf() ); // KfWf
        formatter.add( reportItem.kf() ); // Kf
        formatter.add( reportItem.wf() ); // wf

        formatter.add( reportItem.xf() ); // Xf
        formatter.add( reportItem.h() ); // H
        formatter.add( reportItem.km() ); // Km

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createFracturePressureDepletionSummaryText(
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems ) const
{
    QString tableText;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "Well" ),
                                                   RifTextDataTableColumn( "Fracture" ),
                                                   RifTextDataTableColumn( "Actual WBHP" ),
                                                   RifTextDataTableColumn( "Min Pressure Drop" ),
                                                   RifTextDataTableColumn( "Max Pressure Drop" ) };

    bool createdTable = false;

    for ( const auto& reportItem : wellPathFractureReportItems )
    {
        if ( reportItem.performPressureDepletionScaling() )
        {
            if ( !createdTable )
            {
                formatter.comment(
                    QString( "Pressure Depletion Time step: %1" ).arg( reportItem.pressureDepletionTimeStepString() ) );
                formatter.comment( QString( "WBHP Source: %1" ).arg( reportItem.pressureDepletionWBHPString() ) );
                formatter.comment( QString( "User Defined WBHP: %1" ).arg( reportItem.pressureDepletionUserWBHP() ) );

                formatter.header( header );
                formatter.addHorizontalLine( '-' );
                createdTable = true;
            }
            formatter.add( reportItem.wellPathNameForExport() );
            formatter.add( reportItem.fractureName() );
            formatter.add( reportItem.pressureDepletionActualWBHP() );
            formatter.add( reportItem.pressureDepletionMinPressureDrop() );
            formatter.add( reportItem.pressureDepletionMaxPressureDrop() );
            formatter.rowCompleted();
        }
    }
    if ( createdTable )
    {
        formatter.tableCompleted();
    }

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createConnectionsPerWellText(
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems ) const
{
    QString tableText;

    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );
    configureFormatter( &formatter );

    std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "Well" ), floatNumberColumn( "ConnCount" ) };

    formatter.header( header );
    formatter.addHorizontalLine( '-' );

    std::map<QString /*Well*/, size_t> wellConnectionCounts;
    for ( const auto& reportItem : wellPathFractureReportItems )
    {
        QString wellPathName = reportItem.wellPathNameForExport();
        if ( wellConnectionCounts.find( wellPathName ) == wellConnectionCounts.end() )
        {
            wellConnectionCounts.insert( std::make_pair( wellPathName, 0 ) );
        }

        wellConnectionCounts[wellPathName] += reportItem.connectionCount();
    }

    for ( const auto& connCount : wellConnectionCounts )
    {
        formatter.add( connCount.first );
        formatter.add( connCount.second );

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureTextReportFeatureImpl::configureFormatter( RifTextDataTableFormatter* formatter ) const
{
    if ( !formatter ) return;

    formatter->setColumnSpacing( 3 );
    formatter->setTableRowPrependText( "-- " );
    formatter->setTableRowLineAppendText( "" );
}
