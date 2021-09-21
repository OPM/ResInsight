/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicCreateEnsembleWellLogUi.h"

#include "RiaApplication.h"

#include "RiaDefines.h"
#include "RigEclipseCaseData.h"

#include "RigEclipseResultAddress.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "cafAppEnum.h"
#include "cafPdmObject.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RicCreateEnsembleWellLogUi, "RicCreateEnsembleWellLogUi" );

namespace caf
{
template <>
void caf::AppEnum<RicCreateEnsembleWellLogUi::WellPathSource>::setUp()
{
    addItem( RicCreateEnsembleWellLogUi::WellPathSource::FILE, "FILE", "From file" );
    addItem( RicCreateEnsembleWellLogUi::WellPathSource::PROJECT_WELLS, "PROJECT_WELLS", "From Project Wells" );
    setDefault( RicCreateEnsembleWellLogUi::WellPathSource::FILE );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateEnsembleWellLogUi::RicCreateEnsembleWellLogUi()
{
    CAF_PDM_InitObject( "Create Ensemble Well Log", "", "", "" );

    CAF_PDM_InitField( &m_autoCreateEnsembleWellLogs,
                       "AutoCreateEnsembleWellLogs",
                       false,
                       "Create Ensemble Well Logs From Exported Files",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_autoCreateEnsembleWellLogs );

    CAF_PDM_InitField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathSource, "WellPathSource", "Well Path Source", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_well, "Well", "Well", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_selectedKeywords, "SelectedProperties", "Selected Properties", "", "", "" );
    m_selectedKeywords.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    m_tabNames << "Well"
               << "Properties";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateEnsembleWellLogUi::~RicCreateEnsembleWellLogUi()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RicCreateEnsembleWellLogUi::tabNames() const
{
    return m_tabNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( uiConfigName == m_tabNames[0] )
    {
        uiOrdering.add( &m_wellPathSource );

        bool fileSource = ( m_wellPathSource == RicCreateEnsembleWellLogUi::WellPathSource::FILE );
        uiOrdering.add( &m_well );
        uiOrdering.add( &m_wellPath );
        m_well.uiCapability()->setUiHidden( !fileSource );
        m_wellPath.uiCapability()->setUiHidden( fileSource );
        uiOrdering.add( &m_autoCreateEnsembleWellLogs );
    }
    else if ( uiConfigName == m_tabNames[1] )
    {
        uiOrdering.add( &m_selectedKeywords );
        uiOrdering.add( &m_timeStep );
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicCreateEnsembleWellLogUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedKeywords )
    {
        RigCaseCellResultsData* resultData = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

        std::vector<RiaDefines::ResultCatType> resultCategories = validResultCategories();
        for ( auto catType : resultCategories )
        {
            QList<caf::PdmOptionItemInfo> allOptions =
                RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard( catType, resultData );

            bool isFirstOfCategory = true;
            for ( caf::PdmOptionItemInfo option : allOptions )
            {
                if ( resultData->hasResultEntry( RigEclipseResultAddress( catType, option.optionUiText() ) ) )
                {
                    if ( isFirstOfCategory )
                    {
                        // Add the category title only when there is at least one valid result
                        options.push_back(
                            caf::PdmOptionItemInfo::createHeader( caf::AppEnum<RiaDefines::ResultCatType>::uiText( catType ),
                                                                  true ) );
                        isFirstOfCategory = false;
                    }

                    options.push_back( option );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_caseData->ownerCase(), &options );
    }
    else if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateEnsembleWellLogUi::autoCreateEnsembleWellLogs() const
{
    return m_autoCreateEnsembleWellLogs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, RiaDefines::ResultCatType>> RicCreateEnsembleWellLogUi::properties() const
{
    std::vector<QString> selectedKeyWords = m_selectedKeywords();

    auto findResultCategory = []( const QString&                                keyword,
                                  const std::vector<RiaDefines::ResultCatType>& categories,
                                  RigEclipseCaseData*                           caseData ) {
        // Find the result category for a given keyword
        RigCaseCellResultsData* resultData = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        for ( auto category : categories )
            if ( resultData->hasResultEntry( RigEclipseResultAddress( category, keyword ) ) ) return category;

        return RiaDefines::ResultCatType::UNDEFINED;
    };

    std::vector<RiaDefines::ResultCatType> resultCategories = validResultCategories();

    std::vector<std::pair<QString, RiaDefines::ResultCatType>> props;
    for ( auto keyword : selectedKeyWords )
    {
        auto resultCategory = findResultCategory( keyword, resultCategories, m_caseData );
        props.push_back( std::make_pair( keyword, resultCategory ) );
    }

    return props;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaDefines::ResultCatType> RicCreateEnsembleWellLogUi::validResultCategories() const
{
    return { RiaDefines::ResultCatType::STATIC_NATIVE,
             RiaDefines::ResultCatType::DYNAMIC_NATIVE,
             RiaDefines::ResultCatType::INPUT_PROPERTY };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicCreateEnsembleWellLogUi::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicCreateEnsembleWellLogUi::wellPathFilePath() const
{
    return m_well().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateEnsembleWellLogUi::WellPathSource RicCreateEnsembleWellLogUi::wellPathSource() const
{
    return m_wellPathSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogUi::setWellPathSource( RicCreateEnsembleWellLogUi::WellPathSource wellPathSource )
{
    m_wellPathSource = wellPathSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicCreateEnsembleWellLogUi::wellPathFromProject() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogUi::setWellPathFromProject( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogUi::setCaseData( RigEclipseCaseData* caseData )
{
    m_caseData = caseData;

    if ( m_selectedKeywords().empty() )
    {
        RigCaseCellResultsData* resultData      = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        std::vector<QString>    defaultKeywords = { "INDEX_K", "PORO", "PERMZ", "PRESSURE" };
        std::vector<RiaDefines::ResultCatType> categories = validResultCategories();

        for ( auto keyword : defaultKeywords )
        {
            for ( auto category : categories )
            {
                if ( resultData->hasResultEntry( RigEclipseResultAddress( category, keyword ) ) )
                {
                    m_selectedKeywords.v().push_back( keyword );
                    break;
                }
            }
        }
    }
}
