/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimFormationFilter.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaResultNames.h"

#include "RigEclipseCaseData.h"

#include "Formations/RimFormationNamesCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLabel.h"

#include <QStringList>

#include <algorithm>
#include <iterator>

CAF_PDM_SOURCE_INIT( RimFormationFilter, "FormationFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationFilter::RimFormationFilter()
{
    CAF_PDM_InitObject( "Formation Filter", ":/Formations16x16.png" );

    // Formation filters always select formations
    m_useCategorySelection = true;

    // The base class sets the icon before this class is constructed
    if ( RiaGuiApplication::isRunning() )
    {
        updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationFilter::convertPropertyFiltersToFormationFilters( caf::PdmChildArrayField<RimCellFilter*>& filters )
{
    for ( size_t i = 0; i < filters.size(); i++ )
    {
        auto* propertyFilter = dynamic_cast<RimEclipsePropertyFilter*>( filters[i] );
        if ( !propertyFilter || dynamic_cast<RimFormationFilter*>( propertyFilter ) ) continue;
        if ( propertyFilter->resultDefinition()->resultType() != RiaDefines::ResultCatType::FORMATION_NAMES ) continue;

        auto* formationFilter = dynamic_cast<RimFormationFilter*>(
            propertyFilter->xmlCapability()->copyAndCastByXmlSerialization( classKeywordStatic(),
                                                                            propertyFilter->classKeyword(),
                                                                            caf::PdmDefaultObjectFactory::instance() ) );
        if ( !formationFilter ) continue;

        filters.set( i, formationFilter );
        delete propertyFilter;
    }
}

//--------------------------------------------------------------------------------------------------
/// Configure the filter as a category property filter on the active formation names, with all
/// formations initially selected. The eclipse case must be set before calling.
//--------------------------------------------------------------------------------------------------
void RimFormationFilter::setToFormationNames()
{
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::FORMATION_NAMES );
    m_resultDefinition->setResultVariable( RiaResultNames::activeFormationNamesResultName() );
    m_resultDefinition->loadResult();

    setToDefaultValues();
    updateFilterName();
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationFilter::iconResourceString() const
{
    return ":/Formations16x16.png";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Filter Settings" );
    if ( !hasFormationNames() )
    {
        auto* label = group->addNewLabel( "No formations loaded. Import formation names in <a href=\"dummy\">Formations</a>." );
        label->setLinkActivatedCallback(
            []( const QString& link )
            {
                auto* project = RimProject::current();
                if ( project && project->activeOilField() && project->activeOilField()->formationNamesCollection() )
                {
                    Riu3DMainWindowTools::selectAsCurrentItem( project->activeOilField()->formationNamesCollection() );
                }
            } );
    }
    group->add( &m_filterMode );
    group->add( &m_selectedCategoryValues );

    uiOrdering.skipRemainingFields( true );

    updateReadOnlyStateOfAllFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFormationFilter::hasFormationNames() const
{
    RimEclipseCase* ec = m_resultDefinition->eclipseCase();
    if ( !ec ) ec = eclipseCase();
    if ( ec && ec->eclipseCaseData() ) return !ec->eclipseCaseData()->formationNames().empty();

    return !m_categoryNames.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationFilter::updateFilterName()
{
    if ( m_hasUserDefinedName() ) return;

    if ( m_categoryNames.empty() )
    {
        RimEclipsePropertyFilter::updateFilterName();
        return;
    }

    QString newFiltername = "Formation Names";
    if ( filterMode() == EXCLUDE ) newFiltername += " excl.";
    newFiltername += " (" + compressedSelectedCategoryNames( 50 ) + ")";

    setName( newFiltername );
}

//--------------------------------------------------------------------------------------------------
/// Comma separated names of the selected categories. At most maxLength characters are used for
/// the names; remaining selected categories are summarized as "+N".
//--------------------------------------------------------------------------------------------------
QString RimFormationFilter::compressedSelectedCategoryNames( int maxLength ) const
{
    if ( m_selectedCategoryValues().empty() ) return "none";
    if ( m_selectedCategoryValues().size() == m_categoryValues.size() ) return "all";

    QStringList names;
    for ( int value : m_selectedCategoryValues() )
    {
        auto it = std::find( m_categoryValues.begin(), m_categoryValues.end(), value );
        if ( it == m_categoryValues.end() ) continue;

        const size_t index = std::distance( m_categoryValues.begin(), it );
        if ( index < m_categoryNames.size() ) names.push_back( m_categoryNames[index] );
    }

    QString text;
    int     numIncluded = 0;
    for ( const QString& name : names )
    {
        const QString candidate = text.isEmpty() ? name : text + ", " + name;
        if ( candidate.size() > maxLength )
        {
            if ( numIncluded == 0 ) text = name.left( maxLength - 3 ) + "...";
            break;
        }
        text = candidate;
        numIncluded++;
    }

    if ( numIncluded < names.size() && numIncluded > 0 ) text += QString( ", +%1" ).arg( names.size() - numIncluded );
    if ( numIncluded == 0 && !names.isEmpty() && names.size() > 1 ) text += QString( ", +%1" ).arg( names.size() - 1 );

    return text;
}
