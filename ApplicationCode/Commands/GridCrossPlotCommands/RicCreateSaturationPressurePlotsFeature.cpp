/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicCreateSaturationPressurePlotsFeature.h"
#include "RicSaturationPressureUi.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"
#include "RimSaturationPressurePlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSaturationPressurePlotsFeature, "RicCreateSaturationPressurePlotsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSaturationPressurePlot*>
    RicCreateSaturationPressurePlotsFeature::createPlots( RimEclipseResultCase* eclipseResultCase, int timeStep )
{
    std::vector<RimSaturationPressurePlot*> plots;

    if ( !eclipseResultCase )
    {
        RiaLogging::error( "CreateSaturationPressurePlots:: No case specified" );
        return plots;
    }

    RimProject* project = RimProject::current();

    RimSaturationPressurePlotCollection* collection = project->mainPlotCollection()->saturationPressurePlotCollection();

    if ( eclipseResultCase && eclipseResultCase->ensureReservoirCaseIsOpen() )
    {
        eclipseResultCase->ensureDeckIsParsedForEquilData();

        RigEclipseCaseData* eclipseCaseData = eclipseResultCase->eclipseCaseData();

        if ( eclipseCaseData->equilData().empty() )
        {
            RiaLogging::error( "CreateSaturationPressurePlots:: No EQUIL data available" );
            return plots;
        }

        if ( eclipseCaseData && eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
        {
            RigCaseCellResultsData* resultData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

            if ( !resultData->hasResultEntry(
                     RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" ) ) )
            {
                RiaLogging::error( "CreateSaturationPressurePlots : PRESSURE is not available " );
                return plots;
            }

            if ( !resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PDEW" ) ) )
            {
                RiaLogging::error( "CreateSaturationPressurePlots : PDEW is not available " );
                return plots;
            }

            if ( !resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PBUB" ) ) )
            {
                RiaLogging::error( "CreateSaturationPressurePlots : PBUB is not available " );
                return plots;
            }
        }
    }

    plots = collection->createSaturationPressurePlots( eclipseResultCase, timeStep );
    for ( auto plot : plots )
    {
        plot->loadDataAndUpdate();
        plot->zoomAll();
        plot->updateConnectedEditors();
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateSaturationPressurePlotsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSaturationPressurePlotsFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();

    RimSaturationPressurePlotCollection* collection = project->mainPlotCollection()->saturationPressurePlotCollection();

    std::vector<RimEclipseResultCase*> eclipseCases;
    {
        RiaApplication*       app = RiaApplication::instance();
        std::vector<RimCase*> cases;
        app->project()->allCases( cases );
        for ( auto* rimCase : cases )
        {
            auto erc = dynamic_cast<RimEclipseResultCase*>( rimCase );
            if ( erc )
            {
                eclipseCases.push_back( erc );
            }
        }
    }

    RimEclipseResultCase* eclipseResultCase = nullptr;
    int                   timeStep          = 0;

    if ( !eclipseCases.empty() )
    {
        if ( eclipseCases.size() == 1 )
        {
            eclipseResultCase = eclipseCases[0];
        }

        if ( !eclipseResultCase || eclipseResultCase->timeStepDates().size() > 1 )
        {
            RicSaturationPressureUi saturationPressureUi;
            saturationPressureUi.setSelectedCase( eclipseCases[0] );

            RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();

            caf::PdmUiPropertyViewDialog propertyDialog( plotwindow,
                                                         &saturationPressureUi,
                                                         "Select Case to create Pressure Saturation plots",
                                                         "" );

            if ( propertyDialog.exec() == QDialog::Accepted )
            {
                eclipseResultCase = dynamic_cast<RimEclipseResultCase*>( saturationPressureUi.selectedCase() );
                timeStep          = saturationPressureUi.selectedTimeStep();
            }
        }
    }

    caf::PdmObject* objectToSelect = nullptr;

    std::vector<RimSaturationPressurePlot*> plots = createPlots( eclipseResultCase, timeStep );
    if ( plots.empty() )
    {
        QString text = "No plots generated.\n\n";
        text += "Data required to generate saturation/pressure plots:\n";
        text += " - EQUIL property defining at least one region\n";
        text += " - EQLNUM property defining at least one region\n";
        text += " - Dynamic properties PRESSURE, PBUB and PDEW\n\n";
        text += "Make sure to add 'PBPD' to the RPTRST keyword in the SOLUTION selection. ";
        text += "If this is a two phase run (Oil/water or Gas/Water) or if both VAPOIL ";
        text += "and DISGAS are disabled, saturation pressure are not valid.\n\n";
        text += "See error log for more details.";

        RiaLogging::errorInMessageBox( nullptr, "Saturation Pressure Plots", text );
    }
    else
    {
        objectToSelect = plots.front();
    }

    collection->updateAllRequiredEditors();
    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();

    if ( objectToSelect )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSaturationPressurePlotsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Saturation Pressure Plots" );
    actionToSetup->setIcon( QIcon( ":/SummaryXPlotsLight16x16.png" ) );
}
