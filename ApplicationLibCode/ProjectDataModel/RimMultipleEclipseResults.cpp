/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RimMultipleEclipseResults.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimMultipleEclipseResults, "RimMultipleEclipseResults" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultipleEclipseResults::RimMultipleEclipseResults()
{
    CAF_PDM_InitObject( "Multiple Result Info", ":/TextAnnotation16x16.png" );

    CAF_PDM_InitField( &m_showCenterCoordinates, "showCenterCoordinates", false, "Show Center Coordinates" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showCenterCoordinates );

    CAF_PDM_InitField( &m_showCornerCoordinates, "showCornerCoordinates", false, "Show Corner Coordinates" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showCornerCoordinates );

    CAF_PDM_InitFieldNoDefault( &m_selectedKeywords, "SelectedProperties", "Properties" );
    m_selectedKeywords.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_selectedKeywords.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleEclipseResults::setEclipseView( RimEclipseView* eclipseView )
{
    m_eclipseView = eclipseView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseResultAddress> RimMultipleEclipseResults::additionalResultAddresses() const
{
    if ( !m_eclipseView || !m_eclipseView->currentGridCellResults() || !m_isChecked ) return {};

    std::set<QString> selectedResults;
    for ( const auto& result : m_selectedKeywords() )
    {
        selectedResults.insert( result );
    }

    std::vector<RigEclipseResultAddress> resultAddresses;

    auto gridCellResult = m_eclipseView->currentGridCellResults();
    for ( const auto& res : gridCellResult->existingResults() )
    {
        auto candidate = res.resultName();
        if ( selectedResults.count( candidate ) > 0 )
        {
            resultAddresses.emplace_back( res );
        }
    }

    return resultAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultipleEclipseResults::showCenterCoordinates() const
{
    return m_showCenterCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultipleEclipseResults::showCornerCoordinates() const
{
    return m_showCornerCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultipleEclipseResults::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( fieldNeedingOptions == &m_selectedKeywords )
    {
        if ( !m_eclipseView || !m_eclipseView->currentGridCellResults() ) return {};

        QList<caf::PdmOptionItemInfo> options;

        RigCaseCellResultsData*                resultData       = m_eclipseView->currentGridCellResults();
        std::vector<RiaDefines::ResultCatType> resultCategories = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                    RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                    RiaDefines::ResultCatType::INPUT_PROPERTY };
        for ( auto catType : resultCategories )
        {
            QList<caf::PdmOptionItemInfo> allOptions = RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard( catType, resultData );

            bool isFirstOfCategory = true;
            for ( const caf::PdmOptionItemInfo& option : allOptions )
            {
                if ( resultData->hasResultEntry( RigEclipseResultAddress( catType, option.optionUiText() ) ) )
                {
                    if ( isFirstOfCategory )
                    {
                        // Add the category title only when there is at least one valid result
                        options.push_back(
                            caf::PdmOptionItemInfo::createHeader( caf::AppEnum<RiaDefines::ResultCatType>::uiText( catType ), true ) );
                        isFirstOfCategory = false;
                    }

                    options.push_back( option );
                }
            }
        }

        return options;
    }

    return {};
}
