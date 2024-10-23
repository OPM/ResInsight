/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimStatisticsContourMap.h"

#include "RiaLogging.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimProject.h"

#include "RimEclipseResultDefinition.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafProgressInfo.h"

CAF_PDM_SOURCE_INIT( RimStatisticsContourMap, "RimStatisticalContourMap" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMap::RimStatisticsContourMap()
{
    CAF_PDM_InitObject( "StatisticsContourMap", ":/Histogram16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "" );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Result" );
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_resultDefinition->setResultVariable( "SOIL" );

    CAF_PDM_InitFieldNoDefault( &m_computeStatisticsButton, "ComputeStatisticsButton", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_computeStatisticsButton );
    m_computeStatisticsButton = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* resultDefinitionGroup = uiOrdering.addNewGroup( "Result Definition" );
    m_resultDefinition->uiOrdering( uiConfigName, *resultDefinitionGroup );

    uiOrdering.add( &m_computeStatisticsButton );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_resultDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_computeStatisticsButton == changedField )
    {
        computeStatistics();
        m_computeStatisticsButton = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_computeStatisticsButton == field )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Compute";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::initAfterRead()
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    if ( ensemble->cases().empty() ) return;

    RimEclipseCase* eclipseCase = ensemble->cases().front();
    setEclipseCase( eclipseCase );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::computeStatistics()
{
    RiaLogging::info( "Computing statistics" );
}
