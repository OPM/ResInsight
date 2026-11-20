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

#include "RimcFormationFilter.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RigEclipseCaseData.h"

#include "Formations/RimFormationTools.h"
#include "RimCellFilter.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFormationFilter.h"
#include "RimReservoirGridEnsemble.h"

#include "RimcDataContainerString.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QStringList>

#include <algorithm>
#include <memory>

namespace
{
//--------------------------------------------------------------------------------------------------
/// The case used to configure filters owned by the given object. For an ensemble, this is the main case.
//--------------------------------------------------------------------------------------------------
RimEclipseCase* filterCase( caf::PdmObject* filterOwner )
{
    if ( !filterOwner ) return nullptr;

    if ( auto* ensemble = filterOwner->firstAncestorOrThisOfType<RimReservoirGridEnsemble>() ) return ensemble->mainCase();

    if ( auto* propertyFilter = dynamic_cast<RimEclipsePropertyFilter*>( filterOwner ) )
    {
        if ( auto* eclipseCase = propertyFilter->resultDefinition()->eclipseCase() ) return eclipseCase;
    }

    if ( auto* collection = filterOwner->firstAncestorOrThisOfType<RimDataFilterCollection>() )
    {
        if ( auto* eclipseCase = dynamic_cast<RimEclipseCase*>( collection->ownerCase() ) ) return eclipseCase;
    }

    if ( auto* view = filterOwner->firstAncestorOrThisOfType<RimEclipseView>() ) return view->eclipseCase();

    return filterOwner->firstAncestorOrThisOfType<RimEclipseCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> formationNamesOfLoadedCase( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return {};
    return eclipseCase->eclipseCaseData()->formationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimCellFilter::FilterModeType, QString> filterModeFromText( const QString& text )
{
    const QString mode = text.trimmed().toUpper();
    if ( mode.isEmpty() || mode == "INCLUDE" ) return RimCellFilter::INCLUDE;
    if ( mode == "EXCLUDE" ) return RimCellFilter::EXCLUDE;

    return std::unexpected( QString( "Invalid filter mode '%1'. Use INCLUDE or EXCLUDE" ).arg( text ) );
}

//--------------------------------------------------------------------------------------------------
/// Apply the requested selection and mode to a newly created formation filter, and notify listeners
//--------------------------------------------------------------------------------------------------
std::expected<RimFormationFilter*, QString>
    configureFormationFilter( RimFormationFilter* filter, const std::vector<QString>& formationNames, RimCellFilter::FilterModeType mode )
{
    if ( !filter ) return std::unexpected( QString( "Failed to create formation filter" ) );

    filter->setFilterMode( mode );

    // The names are validated before the filter is created, so this is not expected to fail
    if ( auto result = filter->setSelectedFormationNames( formationNames ); !result ) return std::unexpected( result.error() );

    filter->updateFilterName();
    filter->updateConnectedEditors();

    // Notify the parent after the final name and selection are set, to refresh derived names and views
    filter->triggerFilterChanged();

    return filter;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimcFormationFilterTools::ensureFormationNamesAreLoaded( caf::PdmObject* filterOwner )
{
    if ( auto* ensemble = filterOwner ? filterOwner->firstAncestorOrThisOfType<RimReservoirGridEnsemble>() : nullptr )
    {
        // The grids of an ensemble are loaded on demand. The formation names result requires the grid.
        ensemble->loadGridDataFromFiles();
    }

    RimEclipseCase* eclipseCase = filterCase( filterOwner );
    if ( !eclipseCase ) return std::unexpected( QString( "No Eclipse case found for the formation filter" ) );

    if ( !eclipseCase->ensureReservoirCaseIsOpen() )
    {
        return std::unexpected( QString( "Unable to open case '%1'" ).arg( eclipseCase->caseUserDescription() ) );
    }

    if ( formationNamesOfLoadedCase( eclipseCase ).empty() )
    {
        return std::unexpected(
            QString( "No formation names are loaded for case '%1'. Import formation names before creating a formation filter" )
                .arg( eclipseCase->caseUserDescription() ) );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimcFormationFilterTools::validateFormationNames( caf::PdmObject*             filterOwner,
                                                                               const std::vector<QString>& formationNames )
{
    RimEclipseCase*            eclipseCase = filterCase( filterOwner );
    const std::vector<QString> available   = formationNamesOfLoadedCase( eclipseCase );

    auto indices = RimFormationTools::formationIndicesFromNames( available, formationNames );
    if ( indices ) return {};

    QString message = indices.error();

    // For an ensemble, point out names that exist in other realizations than the main case
    if ( auto* ensemble = filterOwner->firstAncestorOrThisOfType<RimReservoirGridEnsemble>() )
    {
        for ( const QString& name : formationNames )
        {
            if ( std::find( available.begin(), available.end(), name ) != available.end() ) continue;

            QStringList realizations;
            for ( RimEclipseCase* realization : ensemble->cases() )
            {
                const auto names = RimFormationTools::formationNames( realization );
                if ( std::find( names.begin(), names.end(), name ) != names.end() )
                {
                    realizations.push_back( realization->caseUserDescription() );
                }
            }

            if ( !realizations.isEmpty() )
            {
                message += QString( ". '%1' exists in realizations %2 but not in main case '%3'" )
                               .arg( name )
                               .arg( realizations.join( ", " ) )
                               .arg( eclipseCase ? eclipseCase->caseUserDescription() : QString() );
            }
        }
    }

    return std::unexpected( message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimFormationFilter*, QString> RimcFormationFilterTools::addFormationFilter( RimCombinedFilter*          combinedFilter,
                                                                                          const std::vector<QString>& formationNames,
                                                                                          const QString&              filterMode )
{
    if ( !combinedFilter ) return std::unexpected( QString( "Self is not a RimCombinedFilter" ) );

    auto mode = filterModeFromText( filterMode );
    if ( !mode ) return std::unexpected( mode.error() );

    if ( auto result = ensureFormationNamesAreLoaded( combinedFilter ); !result ) return std::unexpected( result.error() );
    if ( auto result = validateFormationNames( combinedFilter, formationNames ); !result ) return std::unexpected( result.error() );

    RimFormationFilter* filter = nullptr;
    if ( combinedFilter->firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>() )
    {
        // Combined filter in a view: use the view property filter setup, as the GUI command does
        filter = dynamic_cast<RimFormationFilter*>(
            RicEclipsePropertyFilterFeatureImpl::addPropertyFilterToCombinedFilter( combinedFilter, true ) );
    }
    else
    {
        filter = combinedFilter->addNewFilter<RimFormationFilter>( []( RimFormationFilter* f ) { f->setToFormationNames(); } );
        combinedFilter->updateConnectedEditors();
    }

    return configureFormationFilter( filter, formationNames, mode.value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimFormationFilter*, QString> RimcFormationFilterTools::addFormationFilter( RimDataFilterCollection*    collection,
                                                                                          const std::vector<QString>& formationNames,
                                                                                          const QString&              filterMode )
{
    if ( !collection ) return std::unexpected( QString( "Self is not a RimDataFilterCollection" ) );

    auto mode = filterModeFromText( filterMode );
    if ( !mode ) return std::unexpected( mode.error() );

    if ( auto result = ensureFormationNamesAreLoaded( collection ); !result ) return std::unexpected( result.error() );
    if ( auto result = validateFormationNames( collection, formationNames ); !result ) return std::unexpected( result.error() );

    RimFormationFilter* filter = collection->addNewFormationFilter();
    collection->updateConnectedEditors();

    return configureFormationFilter( filter, formationNames, mode.value() );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCombinedFilter, RimcCombinedFilter_addFormationFilter, "AddFormationFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCombinedFilter_addFormationFilter::RimcCombinedFilter_addFormationFilter( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Formation Filter",
                        "",
                        "",
                        "Add a new formation filter as a child of this combined filter. Formation names are resolved against "
                        "the filter case, which is the main case for an ensemble" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_formationNames, "FormationNames", "", "", "", "Formations to select. Empty selects all" );
    CAF_PDM_InitScriptableField( &m_filterMode, "FilterMode", QString( "INCLUDE" ), "", "", "", "INCLUDE or EXCLUDE" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcCombinedFilter_addFormationFilter::execute()
{
    auto result = RimcFormationFilterTools::addFormationFilter( self<RimCombinedFilter>(), m_formationNames(), m_filterMode() );
    if ( !result ) return std::unexpected( result.error() );

    return result.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcCombinedFilter_addFormationFilter::classKeywordReturnedType() const
{
    return RimFormationFilter::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimDataFilterCollection, RimcDataFilterCollection_addFormationFilter, "AddFormationFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcDataFilterCollection_addFormationFilter::RimcDataFilterCollection_addFormationFilter( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Formation Filter",
                        "",
                        "",
                        "Add a new formation filter to the data filter collection. Formation names are resolved against the "
                        "filter case, which is the main case for an ensemble" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_formationNames, "FormationNames", "", "", "", "Formations to select. Empty selects all" );
    CAF_PDM_InitScriptableField( &m_filterMode, "FilterMode", QString( "INCLUDE" ), "", "", "", "INCLUDE or EXCLUDE" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcDataFilterCollection_addFormationFilter::execute()
{
    auto result = RimcFormationFilterTools::addFormationFilter( self<RimDataFilterCollection>(), m_formationNames(), m_filterMode() );
    if ( !result ) return std::unexpected( result.error() );

    return result.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcDataFilterCollection_addFormationFilter::classKeywordReturnedType() const
{
    return RimFormationFilter::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFormationFilter, RimcFormationFilter_selectedFormationNames, "SelectedFormationNamesInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFormationFilter_selectedFormationNames::RimcFormationFilter_selectedFormationNames( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Selected Formation Names", "", "", "Names of the selected formations" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFormationFilter_selectedFormationNames::execute()
{
    auto* filter = self<RimFormationFilter>();
    if ( !filter ) return std::unexpected( QString( "Self is not a RimFormationFilter" ) );

    auto result            = std::make_unique<RimcDataContainerString>();
    result->m_stringValues = filter->selectedFormationNames();
    return result.release();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcFormationFilter_selectedFormationNames::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFormationFilter, RimcFormationFilter_setSelectedFormationNames, "SetSelectedFormationNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFormationFilter_setSelectedFormationNames::RimcFormationFilter_setSelectedFormationNames( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Selected Formation Names", "", "", "Select formations by name. Empty selects all" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_formationNames, "FormationNames", "", "", "", "Formations to select. Empty selects all" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFormationFilter_setSelectedFormationNames::execute()
{
    auto* filter = self<RimFormationFilter>();
    if ( !filter ) return std::unexpected( QString( "Self is not a RimFormationFilter" ) );

    if ( auto result = RimcFormationFilterTools::ensureFormationNamesAreLoaded( filter ); !result )
        return std::unexpected( result.error() );
    if ( auto result = RimcFormationFilterTools::validateFormationNames( filter, m_formationNames() ); !result )
    {
        return std::unexpected( result.error() );
    }

    if ( auto result = filter->setSelectedFormationNames( m_formationNames() ); !result ) return std::unexpected( result.error() );

    filter->updateConnectedEditors();
    filter->triggerFilterChanged();

    return nullptr;
}
