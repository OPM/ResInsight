/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicNewGridTimeHistoryCurveFeature.h"

#include "RiaGuiApplication.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RigFemResultAddress.h"

#include "RicNewSummaryCurveFeature.h"
#include "RicSelectSummaryPlotUI.h"
#include "RicWellLogTools.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimProject.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimTools.h"
#include "Tools/RimAutomationSettings.h"

#include "Riu3dSelectionManager.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmReferenceHelper.h"
#include "cafPdmUiPropertyViewDialog.h"

#include "cvfAssert.h"
#include "cvfColor3.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewGridTimeHistoryCurveFeature, "RicNewGridTimeHistoryCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewGridTimeHistoryCurveFeature::userSelectedSummaryPlot()
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    const QString lastUsedSummaryPlotKey( "lastUsedSummaryPlotKey" );

    RimSummaryPlot* defaultSelectedPlot = nullptr;
    {
        QString         lastUsedPlotRef = app->cacheDataObject( lastUsedSummaryPlotKey ).toString();
        RimSummaryPlot* lastUsedPlot =
            dynamic_cast<RimSummaryPlot*>( caf::PdmReferenceHelper::objectFromReference( app->project(), lastUsedPlotRef ) );

        if ( lastUsedPlot )
        {
            defaultSelectedPlot = lastUsedPlot;
        }
        else
        {
            auto getFirstSummaryPlot = []() -> RimSummaryPlot*
            {
                auto summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();
                for ( auto multiPlot : summaryPlotColl->multiPlots() )
                {
                    for ( auto summaryPlot : multiPlot->summaryPlots() )
                    {
                        return summaryPlot;
                    }
                }
                return nullptr;
            };

            defaultSelectedPlot = getFirstSummaryPlot();
        }
    }

    RicSelectSummaryPlotUI featureUi;
    if ( defaultSelectedPlot )
    {
        featureUi.setDefaultSummaryPlot( defaultSelectedPlot );
    }

    QString newPlotName = RicNewGridTimeHistoryCurveFeature::suggestedNewPlotName();
    featureUi.setSuggestedPlotName( newPlotName );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr, &featureUi, "Select Destination Plot", "" );
    propertyDialog.resize( QSize( 400, 200 ) );

    if ( propertyDialog.exec() != QDialog::Accepted ) return nullptr;

    RimSummaryPlot* summaryPlot = nullptr;
    if ( featureUi.isCreateNewPlotChecked() )
    {
        RimSummaryPlot* plot = new RimSummaryPlot();
        plot->setUiName( featureUi.newPlotName() );
        RiaSummaryPlotTools::createAndAppendSingleSummaryMultiPlot( plot );
        plot->loadDataAndUpdate();

        summaryPlot = plot;
    }
    else
    {
        summaryPlot = featureUi.selectedSummaryPlot();
    }

    QString refFromProjectToView = caf::PdmReferenceHelper::referenceFromRootToObject( app->project(), summaryPlot );
    app->setCacheDataObject( lastUsedSummaryPlotKey, refFromProjectToView );

    auto automationSettings = RimTools::automationSettings();
    automationSettings->setDestinationPlot( summaryPlot );

    return summaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewGridTimeHistoryCurveFeature::suggestedNewPlotName()
{
    QString resultName;
    {
        Rim3dView*      activeView = RiaApplication::instance()->activeMainOrComparisonGridView();
        RimEclipseView* eclView    = dynamic_cast<RimEclipseView*>( activeView );
        if ( eclView )
        {
            RimEclipseResultDefinition* resDef = eclView->cellResult();
            resultName                         = resDef->resultVariableUiShortName();
        }

        RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>( activeView );
        if ( geoView )
        {
            // NOTE: See also RimGeoMechProertyFilter for generation of result name

            RimGeoMechResultDefinition* resultDefinition = geoView->cellResultResultDefinition();

            RigFemResultAddress resultAddress = resultDefinition->resultAddress();

            if ( resultAddress.resultPosType == RIG_FORMATION_NAMES )
            {
                resultName = resultDefinition->resultFieldName();
            }
            else
            {
                QString posName;

                switch ( resultAddress.resultPosType )
                {
                    case RIG_NODAL:
                        posName = "N";
                        break;
                    case RIG_ELEMENT_NODAL:
                        posName = "EN";
                        break;
                    case RIG_INTEGRATION_POINT:
                        posName = "IP";
                        break;
                    case RIG_ELEMENT:
                        posName = "E";
                        break;
                    case RIG_ELEMENT_NODAL_FACE:
                        break;
                    case RIG_FORMATION_NAMES:
                        break;
                    case RIG_WELLPATH_DERIVED:
                        break;
                    case RIG_DIFFERENTIALS:
                        break;
                    default:
                        break;
                }

                QString fieldUiName = resultDefinition->resultFieldUiName();
                QString compoUiName = resultDefinition->resultComponentUiName();

                resultName = posName + ", " + fieldUiName + ", " + compoUiName;
            }
        }
    }

    QString plotName = "New Plot Name";
    if ( !resultName.isEmpty() )
    {
        plotName = QString( "Cell Result - %1" ).arg( resultName );
    }

    return plotName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGridTimeHistoryCurveFeature::isCommandEnabled() const
{
    if ( RicWellLogTools::isWellPathOrSimWellSelectedInView() ) return false;

    std::vector<RiuSelectionItem*> items;
    Riu3dSelectionManager::instance()->selectedItems( items );

    if ( !items.empty() )
    {
        const RiuEclipseSelectionItem* eclSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>( items[0] );
        if ( eclSelectionItem && eclSelectionItem->m_resultDefinition )
        {
            if ( eclSelectionItem->m_resultDefinition->isFlowDiagOrInjectionFlooding() &&
                 eclSelectionItem->m_resultDefinition->resultVariable() != RIG_NUM_FLOODED_PV )
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::onActionTriggered( bool isChecked )
{
    RimSummaryPlot* summaryPlot = RicNewGridTimeHistoryCurveFeature::userSelectedSummaryPlot();
    if ( !summaryPlot ) return;

    std::vector<RiuSelectionItem*> items;
    Riu3dSelectionManager::instance()->selectedItems( items );
    CVF_ASSERT( !items.empty() );

    for ( auto item : items )
    {
        RimGridTimeHistoryCurve::createCurveFromSelectionItem( item, summaryPlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Plot Time History for Selected Cells" );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}
