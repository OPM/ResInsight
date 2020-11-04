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

#include "RimCompletionTemplateCollection.h"

#include "RimStimPlanModelTemplateCollection.h"
#include "RimFractureTemplateCollection.h"
#include "RimValveTemplateCollection.h"

#include "cafPdmUiTreeOrdering.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT( RimCompletionTemplateCollection, "CompletionTemplateCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCompletionTemplateCollection::RimCompletionTemplateCollection()
{
    CAF_PDM_InitObject( "Completion Templates", ":/CompletionsSymbol16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureTemplates, "FractureTemplates", "", "", "", "" );
    m_fractureTemplates = new RimFractureTemplateCollection;
    m_fractureTemplates->addDefaultEllipseTemplate();

    CAF_PDM_InitFieldNoDefault( &m_stimPlanModelTemplates, "StimPlanModelTemplates", "", "", "", "" );
    m_stimPlanModelTemplates = new RimStimPlanModelTemplateCollection;

    CAF_PDM_InitFieldNoDefault( &m_valveTemplates, "ValveTemplates", "", "", "", "" );
    m_valveTemplates = new RimValveTemplateCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCompletionTemplateCollection::~RimCompletionTemplateCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection* RimCompletionTemplateCollection::fractureTemplateCollection()
{
    return m_fractureTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimFractureTemplateCollection* RimCompletionTemplateCollection::fractureTemplateCollection() const
{
    return m_fractureTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplateCollection* RimCompletionTemplateCollection::valveTemplateCollection()
{
    return m_valveTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimValveTemplateCollection* RimCompletionTemplateCollection::valveTemplateCollection() const
{
    return m_valveTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionTemplateCollection::setDefaultUnitSystemBasedOnLoadedCases()
{
    m_fractureTemplates->setDefaultUnitSystemBasedOnLoadedCases();
    m_valveTemplates->setDefaultUnitSystemBasedOnLoadedCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionTemplateCollection::setFractureTemplateCollection( RimFractureTemplateCollection* fractureTemplateCollection )
{
    CVF_ASSERT( !m_fractureTemplates->fractureTemplates().empty() );
    m_fractureTemplates = fractureTemplateCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplateCollection* RimCompletionTemplateCollection::stimPlanModelTemplateCollection()
{
    return m_stimPlanModelTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimStimPlanModelTemplateCollection* RimCompletionTemplateCollection::stimPlanModelTemplateCollection() const
{
    return m_stimPlanModelTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionTemplateCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                            QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_fractureTemplates );
    uiTreeOrdering.add( m_stimPlanModelTemplates );
    uiTreeOrdering.add( m_valveTemplates );
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionTemplateCollection::loadAndUpdateData()
{
    m_fractureTemplates->loadAndUpdateData();
    m_stimPlanModelTemplates->loadAndUpdateData();
}
