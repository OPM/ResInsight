/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimCellEdgeColors.h"

#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RicCreateEnsembleWellLogUi.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "cvfMath.h"

#include <array>

namespace caf
{
template <>
void AppEnum<RimCellEdgeColors::PropertyType>::setUp()
{
    addItem( RimCellEdgeColors::MULTI_AXIS_STATIC_PROPERTY, "MULTI_AXIS_STATIC_PROPERTY", "Multi Axis Static Property" );
    QStringList aliasList = { "ANY_SINGLE_PROPERTY" };
    addItem( RimCellEdgeColors::CUSTOM_PROPERTIES, "CUSTOM_PROPERTIES", "Selected Properties", aliasList );
    setDefault( RimCellEdgeColors::MULTI_AXIS_STATIC_PROPERTY );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCellEdgeColors, "CellEdgeResultSlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::RimCellEdgeColors()
{
    CAF_PDM_InitObject( "Cell Edge Result", ":/EdgeResult_1.png" );

    CAF_PDM_InitField( &m_enableCellEdgeColors, "EnableCellEdgeColors", true, "Enable Cell Edge Results" );

    CAF_PDM_InitFieldNoDefault( &m_propertyType, "propertyType", "Property Type" );

    CAF_PDM_InitFieldNoDefault( &m_resultVariable, "CellEdgeVariable", "Result property" );
    CAF_PDM_InitField( &useXVariable, "UseXVariable", true, "Use X Values" );
    CAF_PDM_InitField( &useYVariable, "UseYVariable", true, "Use Y Values" );
    CAF_PDM_InitField( &useZVariable, "UseZVariable", true, "Use Z Values" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", ":/Legend.png" );

    CAF_PDM_InitFieldNoDefault( &m_selectedKeywords, "SelectedProperties", "Selected Properties" );

    CAF_PDM_InitField( &m_showTextValuesIfItemIsUnchecked,
                       "ShowTextValuesIfItemIsUnchecked",
                       false,
                       "Always Show Values in Result Info Window",
                       "",
                       "Allow display of result values in Result Info window if the Cell Edge object is unchecked in "
                       "Property Editor." );

    m_legendConfig = new RimRegularLegendConfig();

    CAF_PDM_InitFieldNoDefault( &m_singleVarEdgeResultColors_OBSOLETE, "SingleVarEdgeResult", "Result Property", ":/CellResult.png" );
    m_singleVarEdgeResultColors_OBSOLETE = new RimEclipseCellColors();
    m_singleVarEdgeResultColors_OBSOLETE.uiCapability()->setUiHidden( true );

    m_ignoredResultScalar = cvf::UNDEFINED_DOUBLE;
    resetResultAddresses();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::~RimCellEdgeColors()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setReservoirView( RimEclipseView* ownerReservoirView )
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::loadResult()
{
    if ( !m_reservoirView->currentGridCellResults() ) return;

    resetResultAddresses();

    if ( isUserDefinedPropertiesActive() )
    {
        std::vector<RiaDefines::ResultCatType> categories = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                              RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                              RiaDefines::ResultCatType::INPUT_PROPERTY };

        std::vector<QString> resultNames = m_selectedKeywords();
        if ( resultNames.size() > 6 )
        {
            QString title = "Cell Edge Results";

            QString text = QString( "Selected Cell Edge result property count is %1. Up to 6 results are supported, no "
                                    "data will be loaded.\n\nPlease select equal to or less than 6 properties to show data." )
                               .arg( resultNames.size() );

            RiaLogging::errorInMessageBox( nullptr, title, text );

            return;
        }

        auto properties = RicCreateEnsembleWellLogUi::properties( resultNames, categories, m_reservoirView->eclipseCase()->eclipseCaseData() );

        for ( size_t i = 0; i < properties.size(); i++ )
        {
            auto [name, category]       = properties[i];
            RigEclipseResultAddress adr = RigEclipseResultAddress( category, name );

            m_reservoirView->currentGridCellResults()->ensureKnownResultLoaded( adr );

            m_resultNameToAddressPairs[i] = std::make_pair( name, adr );
        }
    }
    else
    {
        QStringList vars = findResultVariableNames();
        updateIgnoredScalarValue();

        int i;
        for ( i = 0; i < vars.size(); ++i )
        {
            m_reservoirView->currentGridCellResults()->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, vars[i] ) );
            int cubeFaceIdx;
            for ( cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx )
            {
                if ( ( ( cubeFaceIdx == 0 || cubeFaceIdx == 1 ) && useXVariable() ) ||
                     ( ( cubeFaceIdx == 2 || cubeFaceIdx == 3 ) && useYVariable() ) ||
                     ( ( cubeFaceIdx == 4 || cubeFaceIdx == 5 ) && useZVariable() ) )
                {
                    QString varEnd = EdgeFaceEnum::textFromIndex( cubeFaceIdx );

                    if ( vars[i].endsWith( varEnd ) )
                    {
                        m_resultNameToAddressPairs[cubeFaceIdx] =
                            std::make_pair( vars[i], RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, vars[i] ) );
                    }
                }
            }
        }
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::initAfterRead()
{
    updateUiIconFromToggleField();

    if ( m_singleVarEdgeResultColors_OBSOLETE->resultVariableUiName() != RiaResultNames::undefinedResultName() )
    {
        m_selectedKeywords.v().push_back( m_singleVarEdgeResultColors_OBSOLETE->resultVariableUiName() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    loadResult();

    if ( m_reservoirView ) m_reservoirView->scheduleCreateDisplayModelAndRedraw();

    if ( changedField == objectToggleField() )
    {
        updateUiIconFromToggleField();
    }
}

namespace caf
{
template <>
void RimCellEdgeColors::EdgeFaceEnum::setUp()
{
    addItem( RimCellEdgeColors::X, "X", "" );
    addItem( RimCellEdgeColors::NEG_X, "X-", "" );
    addItem( RimCellEdgeColors::Y, "Y", "" );
    addItem( RimCellEdgeColors::NEG_Y, "Y-", "" );
    addItem( RimCellEdgeColors::Z, "Z", "" );
    addItem( RimCellEdgeColors::NEG_Z, "Z-", "" );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

QList<caf::PdmOptionItemInfo> RimCellEdgeColors::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultVariable )
    {
        if ( m_reservoirView && m_reservoirView->currentGridCellResults() )
        {
            QStringList varList;
            varList = m_reservoirView->currentGridCellResults()->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );

            // TODO: Must also handle input properties
            // varList += m_reservoirView->gridCellResults()->resultNames(RiaDefines::INPUT_PROPERTY);

            std::map<QString, std::array<QString, 6>> varBaseNameToVarsMap;

            int i;
            for ( i = 0; i < varList.size(); ++i )
            {
                if ( RiaResultNames::isPerCellFaceResult( varList[i] ) ) continue;

                size_t cubeFaceIdx;
                for ( cubeFaceIdx = 0; cubeFaceIdx < EdgeFaceEnum::size(); ++cubeFaceIdx )
                {
                    QString varEnd = EdgeFaceEnum::textFromIndex( cubeFaceIdx );
                    if ( varList[i].endsWith( varEnd ) )
                    {
                        QStringList splits                                  = varList[i].split( varEnd );
                        QString     variableBasename                        = splits.front();
                        varBaseNameToVarsMap[variableBasename][cubeFaceIdx] = varList[i];
                    }
                }
            }

            std::map<QString, std::array<QString, 6>>::iterator it;

            for ( it = varBaseNameToVarsMap.begin(); it != varBaseNameToVarsMap.end(); ++it )
            {
                QString optionUiName = it->first;
                optionUiName += " (";
                int  cubeFaceIdx;
                bool firstText = true;
                for ( cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx )
                {
                    if ( !it->second[cubeFaceIdx].isEmpty() )
                    {
                        if ( firstText )
                        {
                            optionUiName += it->second[cubeFaceIdx];
                            firstText = false;
                        }
                        else
                        {
                            optionUiName += QString( ", " ) + it->second[cubeFaceIdx];
                        }
                    }
                }
                optionUiName += ")";

                options.push_back( caf::PdmOptionItemInfo( optionUiName, QVariant( it->first ) ) );
            }

            options.push_front( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), "" ) );

            return options;
        }
    }

    if ( fieldNeedingOptions == &m_selectedKeywords )
    {
        RigCaseCellResultsData* resultData = m_reservoirView->currentGridCellResults();

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

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showTextValuesIfItemIsUnchecked );
    uiOrdering.add( &m_propertyType );

    if ( isUserDefinedPropertiesActive() )
    {
        uiOrdering.add( &m_selectedKeywords );
    }
    else
    {
        uiOrdering.add( &m_resultVariable );

        uiOrdering.add( &useXVariable );
        uiOrdering.add( &useYVariable );
        uiOrdering.add( &useZVariable );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( legendConfig() );
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimCellEdgeColors::findResultVariableNames()
{
    QStringList varNames;

    if ( m_reservoirView && m_reservoirView->currentGridCellResults() && !m_resultVariable().isEmpty() )
    {
        QStringList varList;
        varList = m_reservoirView->currentGridCellResults()->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );
        // TODO: Must handle Input properties

        int i;
        for ( i = 0; i < varList.size(); ++i )
        {
            if ( RiaResultNames::isPerCellFaceResult( varList[i] ) ) continue;

            if ( varList[i].startsWith( m_resultVariable ) )
            {
                varNames.append( varList[i] );
            }
        }
    }
    return varNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<RigEclipseResultAddress, 6> RimCellEdgeColors::resultAddresses() const
{
    std::array<RigEclipseResultAddress, 6> adr;
    for ( int cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex )
    {
        adr[cubeFaceIndex] = RigEclipseResultAddress( m_resultNameToAddressPairs[cubeFaceIndex].second );
    }

    return adr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::gridScalarResultNames( std::vector<QString>* resultNames )
{
    CVF_ASSERT( resultNames );

    int cubeFaceIndex;
    for ( cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex )
    {
        resultNames->push_back( m_resultNameToAddressPairs[cubeFaceIndex].first );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellEdgeColors::isUserDefinedPropertiesActive() const
{
    return ( m_propertyType == CUSTOM_PROPERTIES );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::cellEdgeMetaData( std::vector<RimCellEdgeMetaData>* metaDataVector )
{
    CVF_ASSERT( metaDataVector );

    auto addresses = resultAddresses();

    std::vector<QString> resultNames;
    gridScalarResultNames( &resultNames );

    bool isStatic = true;

    for ( size_t i = 0; i < 6; i++ )
    {
        if ( isUserDefinedPropertiesActive() )
        {
            isStatic = addresses[i].resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE;
        }

        RimCellEdgeMetaData metaData;
        metaData.m_eclipseResultAddress = addresses[i];
        metaData.m_resultVariable       = resultNames[i];
        metaData.m_isStatic             = isStatic;

        metaDataVector->push_back( metaData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::resetResultAddresses()
{
    int cubeFaceIndex;
    for ( cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex )
    {
        m_resultNameToAddressPairs[cubeFaceIndex].second = RigEclipseResultAddress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellEdgeColors::hasResult() const
{
    if ( !m_enableCellEdgeColors() ) return false;

    return showTextResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellEdgeColors::showTextResult() const
{
    if ( !m_enableCellEdgeColors && !m_showTextValuesIfItemIsUnchecked ) return false;

    if ( isUserDefinedPropertiesActive() )
    {
        return true;
    }

    bool hasResult = false;
    int  cubeFaceIndex;
    for ( cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex )
    {
        hasResult |= m_resultNameToAddressPairs[cubeFaceIndex].second.isValid();
    }

    return hasResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::updateIgnoredScalarValue()
{
    if ( m_resultVariable() == "MULT" || m_resultVariable() == "riMULT" )
    {
        m_ignoredResultScalar = 1.0;
    }
    else
    {
        m_ignoredResultScalar = cvf::UNDEFINED_DOUBLE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::minMaxCellEdgeValues( double& min, double& max )
{
    double globalMin, globalMax;
    globalMin = HUGE_VAL;
    globalMax = -HUGE_VAL;

    if ( m_reservoirView->currentGridCellResults() )
    {
        auto resultAddresses = this->resultAddresses();

        size_t faceIdx;
        for ( faceIdx = 0; faceIdx < 6; faceIdx++ )
        {
            if ( !resultAddresses[faceIdx].isValid() ) continue;
            if ( !m_reservoirView->currentGridCellResults()->hasResultEntry( resultAddresses[faceIdx] ) ) continue;

            {
                double cMin, cMax;
                m_reservoirView->currentGridCellResults()->minMaxCellScalarValues( resultAddresses[faceIdx], cMin, cMax );

                globalMin = std::min( globalMin, cMin );
                globalMax = std::max( globalMax, cMax );
            }
        }
    }

    min = globalMin;
    max = globalMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::posNegClosestToZero( double& pos, double& neg )
{
    pos = HUGE_VAL;
    neg = -HUGE_VAL;

    auto resultAddresses = this->resultAddresses();

    size_t faceIdx;
    for ( faceIdx = 0; faceIdx < 6; faceIdx++ )
    {
        if ( !resultAddresses[faceIdx].isValid() ) continue;
        if ( !m_reservoirView->currentGridCellResults()->hasResultEntry( resultAddresses[faceIdx] ) ) continue;

        {
            double localPos, localNeg;
            m_reservoirView->currentGridCellResults()->posNegClosestToZero( resultAddresses[faceIdx], localPos, localNeg );

            if ( localPos > 0 && localPos < pos ) pos = localPos;
            if ( localNeg < 0 && localNeg > neg ) neg = localNeg;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setResultVariable( const QString& variableName )
{
    m_resultVariable = variableName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellEdgeColors::resultVariableUiShortName() const
{
    if ( isUserDefinedPropertiesActive() )
    {
        if ( m_selectedKeywords().empty() ) return "";

        if ( m_selectedKeywords().size() == 1 ) return m_selectedKeywords().front();

        return "Multiple Properties";
    }

    return m_resultVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setActive( bool active )
{
    m_enableCellEdgeColors = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellEdgeColors::objectToggleField()
{
    return &m_enableCellEdgeColors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimCellEdgeColors::legendConfig()
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::PropertyType RimCellEdgeColors::propertyType() const
{
    return m_propertyType();
}
