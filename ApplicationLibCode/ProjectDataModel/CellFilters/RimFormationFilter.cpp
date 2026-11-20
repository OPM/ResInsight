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
#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RigEclipseCaseData.h"

#include "Formations/RimFormationNamesCollection.h"
#include "Formations/RimFormationTools.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectScriptingCapability.h"
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
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Formation Filter",
                                                    ":/Formations16x16.png",
                                                    "",
                                                    "",
                                                    "FormationFilter",
                                                    "A cell filter selecting cells in the chosen formations" );

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
std::vector<QString> RimFormationFilter::availableFormationNames() const
{
    // Formation categories use the formation index as value, see RimPropertyFilter::setCategoryNames
    std::vector<QString> names( m_categoryNames.size() );
    for ( size_t i = 0; i < m_categoryNames.size() && i < m_categoryValues.size(); i++ )
    {
        const int value = m_categoryValues[i];
        if ( value >= 0 && value < static_cast<int>( names.size() ) ) names[value] = m_categoryNames[i];
    }
    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimFormationFilter::setSelectedFormationNames( const std::vector<QString>& formationNames )
{
    if ( m_categoryNames.empty() )
    {
        // Categories are not computed until the case data is loaded, see RimDataFilterCollection::loadAndInitializeFilters
        m_resultDefinition->loadResult();
        computeResultValueRange();
    }

    const std::vector<QString> available = availableFormationNames();
    if ( available.empty() ) return std::unexpected( QString( "No formation names are loaded for the filter case" ) );

    auto indices = RimFormationTools::formationIndicesFromNames( available, formationNames );
    if ( !indices ) return std::unexpected( indices.error() );

    m_selectedCategoryValues = indices.value();
    updateFilterName();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFormationFilter::selectedFormationNames() const
{
    return RimFormationTools::formationNamesFromIndices( availableFormationNames(), m_selectedCategoryValues() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsemble* RimFormationFilter::parentEnsemble() const
{
    return firstAncestorOrThisOfType<RimReservoirGridEnsemble>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimFormationFilter::realizationsWithMismatchedFormations() const
{
    RimReservoirGridEnsemble* ensemble = parentEnsemble();
    if ( !ensemble ) return {};

    return RimFormationTools::casesWithDifferentFormations( ensemble->mainCase(), ensemble->cases() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationFilter::warnIfRealizationsHaveMismatchedFormations() const
{
    RimReservoirGridEnsemble* ensemble = parentEnsemble();
    if ( !ensemble ) return;

    const std::vector<RimEclipseCase*> mismatched = realizationsWithMismatchedFormations();
    if ( mismatched.empty() ) return;

    QStringList caseNames;
    for ( RimEclipseCase* eclipseCase : mismatched )
    {
        caseNames.push_back( eclipseCase->caseUserDescription() );
    }

    const QString mainCaseName = ensemble->mainCase() ? ensemble->mainCase()->caseUserDescription() : QString();

    RiaLogging::warning( QString( "Formation filter '%1': realizations %2 have formation names that differ from main case '%3'; "
                                  "filter is resolved against the main case" )
                             .arg( name() )
                             .arg( caseNames.join( ", " ) )
                             .arg( mainCaseName )
                             .toStdString() );
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

        // A formation split over several K ranges occurs more than once, and is listed once
        const size_t index = std::distance( m_categoryValues.begin(), it );
        if ( index < m_categoryNames.size() && !names.contains( m_categoryNames[index] ) ) names.push_back( m_categoryNames[index] );
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
