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

#include "RigCaseCellResultsData.h"
#include "RigFlowDiagResults.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfMath.h"
#include <array>

namespace caf
{
template <>
void AppEnum<RimCellEdgeColors::PropertyType>::setUp()
{
    addItem( RimCellEdgeColors::MULTI_AXIS_STATIC_PROPERTY, "MULTI_AXIS_STATIC_PROPERTY", "Multi Axis Static Property" );
    addItem( RimCellEdgeColors::ANY_SINGLE_PROPERTY, "ANY_SINGLE_PROPERTY", "Any Single Property" );
    setDefault( RimCellEdgeColors::MULTI_AXIS_STATIC_PROPERTY );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCellEdgeColors, "CellEdgeResultSlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::RimCellEdgeColors()
{
    CAF_PDM_InitObject( "Cell Edge Result", ":/EdgeResult_1.png", "", "" );

    CAF_PDM_InitField( &m_enableCellEdgeColors, "EnableCellEdgeColors", true, "Enable Cell Edge Results", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_propertyType, "propertyType", "Property Type", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_resultVariable, "CellEdgeVariable", "Result property", "", "", "" );
    CAF_PDM_InitField( &useXVariable, "UseXVariable", true, "Use X Values", "", "", "" );
    CAF_PDM_InitField( &useYVariable, "UseYVariable", true, "Use Y Values", "", "", "" );
    CAF_PDM_InitField( &useZVariable, "UseZVariable", true, "Use Z Values", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", ":/Legend.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_singleVarEdgeResultColors,
                                "SingleVarEdgeResult",
                                "Result Property",
                                ":/CellResult.png",
                                "",
                                "" );
    m_singleVarEdgeResultColors = new RimEclipseCellColors();

    m_resultVariable.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    m_legendConfig = new RimRegularLegendConfig();

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
    m_singleVarEdgeResultColors->setReservoirView( ownerReservoirView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::loadResult()
{
    if ( !m_enableCellEdgeColors ) return;
    if ( !m_reservoirView->currentGridCellResults() ) return;

    if ( isUsingSingleVariable() )
    {
        m_singleVarEdgeResultColors->loadResult();
        ;

        RigEclipseResultAddress resultAddr = m_singleVarEdgeResultColors->eclipseResultAddress();
        for ( int cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx )
        {
            m_resultNameToAddressPairs[cubeFaceIdx] =
                std::make_pair( m_singleVarEdgeResultColors->resultVariable(), resultAddr );
        }
    }
    else
    {
        resetResultAddresses();
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
                            std::make_pair( vars[i],
                                            RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, vars[i] ) );
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
    m_singleVarEdgeResultColors->initAfterRead();
    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
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

QList<caf::PdmOptionItemInfo> RimCellEdgeColors::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                        bool*                      useOptionsOnly )
{
    if ( fieldNeedingOptions == &m_resultVariable )
    {
        if ( m_reservoirView && m_reservoirView->currentGridCellResults() )
        {
            QStringList varList;
            varList = m_reservoirView->currentGridCellResults()->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );

            // TODO: Must also handle input properties
            // varList += m_reservoirView->gridCellResults()->resultNames(RiaDefines::INPUT_PROPERTY);

            QList<caf::PdmOptionItemInfo> options;

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

            if ( useOptionsOnly ) *useOptionsOnly = true;

            return options;
        }
    }

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_propertyType );

    if ( isUsingSingleVariable() )
    {
        m_singleVarEdgeResultColors->uiOrdering( uiConfigName, uiOrdering );
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
void RimCellEdgeColors::gridScalarIndices( RigEclipseResultAddress resultIndices[6] )
{
    int cubeFaceIndex;
    for ( cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex )
    {
        resultIndices[cubeFaceIndex] = RigEclipseResultAddress( m_resultNameToAddressPairs[cubeFaceIndex].second );
    }
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
bool RimCellEdgeColors::isUsingSingleVariable() const
{
    return ( m_propertyType == ANY_SINGLE_PROPERTY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::cellEdgeMetaData( std::vector<RimCellEdgeMetaData>* metaDataVector )
{
    CVF_ASSERT( metaDataVector );

    RigEclipseResultAddress resultIndices[6];
    this->gridScalarIndices( resultIndices );

    std::vector<QString> resultNames;
    this->gridScalarResultNames( &resultNames );

    bool isStatic = true;
    if ( isUsingSingleVariable() )
    {
        isStatic = m_singleVarEdgeResultColors->resultType() == RiaDefines::ResultCatType::STATIC_NATIVE;
    }

    for ( size_t i = 0; i < 6; i++ )
    {
        RimCellEdgeMetaData metaData;
        metaData.m_eclipseResultAddress = resultIndices[i];
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

    if ( isUsingSingleVariable() && m_singleVarEdgeResultColors->isFlowDiagOrInjectionFlooding() )
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
    if ( m_resultVariable == "MULT" || m_resultVariable == "riMULT" )
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
        if ( isUsingSingleVariable() && singleVarEdgeResultColors()->isFlowDiagOrInjectionFlooding() )
        {
            int currentTimeStep = m_reservoirView->currentTimeStep();

            RigFlowDiagResults*      fldResults = singleVarEdgeResultColors()->flowDiagSolution()->flowDiagResults();
            RigFlowDiagResultAddress resAddr    = singleVarEdgeResultColors()->flowDiagResAddress();

            fldResults->minMaxScalarValues( resAddr, currentTimeStep, &globalMin, &globalMax );
        }
        else
        {
            RigEclipseResultAddress resultAddresses[6];
            this->gridScalarIndices( resultAddresses );

            size_t faceIdx;
            for ( faceIdx = 0; faceIdx < 6; faceIdx++ )
            {
                if ( !resultAddresses[faceIdx].isValid() ) continue;

                {
                    double cMin, cMax;
                    m_reservoirView->currentGridCellResults()->minMaxCellScalarValues( resultAddresses[faceIdx], cMin, cMax );

                    globalMin = std::min( globalMin, cMin );
                    globalMax = std::max( globalMax, cMax );
                }
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

    RigEclipseResultAddress resultAddresses[6];
    this->gridScalarIndices( resultAddresses );

    size_t faceIdx;
    for ( faceIdx = 0; faceIdx < 6; faceIdx++ )
    {
        if ( !resultAddresses[faceIdx].isValid() ) continue;

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
void RimCellEdgeColors::updateUiFieldsFromActiveResult()
{
    m_singleVarEdgeResultColors->updateUiFieldsFromActiveResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_singleVarEdgeResultColors->setEclipseCase( eclipseCase );
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
QString RimCellEdgeColors::resultVariable() const
{
    if ( isUsingSingleVariable() )
    {
        return m_singleVarEdgeResultColors->resultVariable();
    }
    else
    {
        return m_resultVariable;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellEdgeColors::resultVariableUiName() const
{
    if ( isUsingSingleVariable() )
    {
        return m_singleVarEdgeResultColors->resultVariableUiName();
    }
    else
    {
        return m_resultVariable;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellEdgeColors::resultVariableUiShortName() const
{
    if ( isUsingSingleVariable() )
    {
        return m_singleVarEdgeResultColors->resultVariableUiShortName();
    }
    else
    {
        return m_resultVariable;
    }
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
bool RimCellEdgeColors::hasCategoryResult() const
{
    return isUsingSingleVariable() && m_singleVarEdgeResultColors->hasCategoryResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors* RimCellEdgeColors::singleVarEdgeResultColors()
{
    return m_singleVarEdgeResultColors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimCellEdgeColors::legendConfig()
{
    if ( isUsingSingleVariable() )
    {
        return m_singleVarEdgeResultColors->legendConfig();
    }
    else
    {
        return m_legendConfig;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::PropertyType RimCellEdgeColors::propertyType() const
{
    return m_propertyType();
}
