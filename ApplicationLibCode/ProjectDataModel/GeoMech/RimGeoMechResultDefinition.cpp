/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//  Copyright (C) 2015-2018 Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechResultDefinition.h"

#include "RiaMedianCalculator.h"
#include "RiaResultNames.h"

#include "RifGeoMechReaderInterface.h"

#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultCalculatorStressAnisotropy.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"
#include "RigWbsParameter.h"
#include "Well/RigWellPath.h"

#include "Rim3dWellLogCurve.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimPlotCurve.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimViewLinker.h"
#include "RimWellPath.h"

#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QDoubleValidator>

namespace caf
{
template <>
void caf::AppEnum<RigFemResultPosEnum>::setUp()
{
    addItem( RIG_NODAL, "NODAL", "Nodal" );
    addItem( RIG_ELEMENT_NODAL, "ELEMENT_NODAL", "Element Nodal" );
    addItem( RIG_INTEGRATION_POINT, "INTEGRATION_POINT", "Integration Point" );
    addItem( RIG_ELEMENT_NODAL_FACE, "ELEMENT_NODAL_FACE", "Element Nodal On Face" );
    addItem( RIG_FORMATION_NAMES, "FORMATION_NAMES", "Formation Names" );
    addItem( RIG_ELEMENT, "ELEMENT", "Element" );
    addItem( RIG_WELLPATH_DERIVED, "WELLPATH_DERIVED", "Well Path Derived" );
    addItem( RIG_DIFFERENTIALS, "DIFFERENTIALS", "Differentials" );
    setDefault( RIG_NODAL );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimGeoMechResultDefinition, "GeoMechResultDefinition" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition::RimGeoMechResultDefinition()
{
    CAF_PDM_InitObject( "Color Result", ":/CellResult.png" );

    CAF_PDM_InitFieldNoDefault( &m_resultPositionType, "ResultPositionType", "Result Position" );
    m_resultPositionType.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_resultFieldName, "ResultFieldName", QString( "" ), "Field Name" );
    m_resultFieldName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_resultComponentName, "ResultComponentName", QString( "" ), "Component" );
    m_resultComponentName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_timeLapseBaseTimestep, "TimeLapseBaseTimeStep", RigFemResultAddress::noTimeLapseValue(), "Base Time Step" );
    CAF_PDM_InitField( &m_referenceTimeStep, "ReferenceTimeStep", 0, "Reference Time Step" );

    CAF_PDM_InitField( &m_compactionRefLayer, "CompactionRefLayer", 0, "Compaction Ref Layer" );
    m_compactionRefLayer.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_resultPositionTypeUiField, "ResultPositionTypeUi", "Result Position" );
    m_resultPositionTypeUiField.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_resultVariableUiField, "ResultVariableUI", QString( "" ), "Value" );
    m_resultVariableUiField.xmlCapability()->disableIO();

    m_resultVariableUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_resultVariableUiField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_normalizeByHydrostaticPressure, "NormalizeByHSP", false, "Normalize by Hydrostatic Pressure" );
    CAF_PDM_InitField( &m_normalizationAirGap, "NormalizationAirGap", 0.0, "Air Gap" );
    m_normalizationAirGap.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_compactionRefLayerUiField,
                       "CompactionRefLayerUi",
                       RigFemResultAddress::noCompactionValue(),
                       "Compaction Ref Layer",
                       "",
                       "The compaction is calculated with reference to this layer. Default layer is the topmost "
                       "layer "
                       "with POR",
                       "" );
    m_compactionRefLayerUiField.xmlCapability()->disableIO();

    m_isChangedByField          = false;
    m_addWellPathDerivedResults = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition::~RimGeoMechResultDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultPositionTypeUiField );
    uiOrdering.add( &m_resultVariableUiField );

    QString valueLabel = "Value";
    if ( m_timeLapseBaseTimestep != RigFemResultAddress::noTimeLapseValue() )
    {
        valueLabel += QString( " (%1)" ).arg( diffResultUiName() );
    }
    m_resultVariableUiField.uiCapability()->setUiName( valueLabel );

    if ( normalizableResultSelected() )
    {
        caf::PdmUiGroup* normalizationGroup = uiOrdering.addNewGroup( "Result Normalization" );
        normalizationGroup->add( &m_normalizeByHydrostaticPressure );
        if ( m_normalizeByHydrostaticPressure )
        {
            normalizationGroup->add( &m_normalizationAirGap );
        }
    }

    if ( m_resultPositionTypeUiField() != RIG_FORMATION_NAMES )
    {
        bool isReferenceCaseDependent = referenceCaseDependentResultSelected();
        if ( isReferenceCaseDependent )
        {
            caf::PdmUiGroup* referenceCaseGr = uiOrdering.addNewGroup( "Reference Case" );
            referenceCaseGr->add( &m_referenceTimeStep );
        }
        else
        {
            caf::PdmUiGroup* timeLapseGr = uiOrdering.addNewGroup( "Difference Options" );
            timeLapseGr->add( &m_timeLapseBaseTimestep );
        }
    }

    if ( m_resultPositionTypeUiField() == RIG_NODAL )
    {
        caf::PdmUiGroup* compactionGroup = uiOrdering.addNewGroup( "Compaction Options" );
        compactionGroup->add( &m_compactionRefLayerUiField );

        if ( m_compactionRefLayerUiField == RigFemResultAddress::noCompactionValue() )
        {
            if ( m_geomCase && m_geomCase->geoMechData() )
            {
                m_compactionRefLayerUiField =
                    (int)m_geomCase->geoMechData()->femParts()->part( 0 )->getOrCreateStructGrid()->reservoirIJKBoundingBox().first.z();
            }
        }
    }

    if ( !m_isChangedByField )
    {
        m_resultPositionTypeUiField = m_resultPositionType;
        m_resultVariableUiField     = composeFieldCompString( m_resultFieldName(), m_resultComponentName() );
    }

    m_isChangedByField = false;

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechResultDefinition::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( m_geomCase )
    {
        if ( &m_resultPositionTypeUiField == fieldNeedingOptions )
        {
            std::vector<RigFemResultPosEnum> optionItems = { RIG_NODAL,
                                                             RIG_ELEMENT_NODAL,
                                                             RIG_INTEGRATION_POINT,
                                                             RIG_ELEMENT_NODAL_FACE,
                                                             RIG_FORMATION_NAMES,
                                                             RIG_ELEMENT,
                                                             RIG_DIFFERENTIALS };
            if ( m_addWellPathDerivedResults )
            {
                optionItems.push_back( RIG_WELLPATH_DERIVED );
            }
            for ( RigFemResultPosEnum value : optionItems )
            {
                options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RigFemResultPosEnum>::uiText( value ), QVariant( value ) ) );
            }
        }
        else if ( &m_resultVariableUiField == fieldNeedingOptions )
        {
            std::map<std::string, std::vector<std::string>> fieldCompNames = getResultMetaDataForUIFieldSetting();

            QStringList uiVarNames;
            QStringList varNames;

            getUiAndResultVariableStringList( &uiVarNames, &varNames, fieldCompNames );

            for ( int oIdx = 0; oIdx < uiVarNames.size(); ++oIdx )
            {
                options.push_back( caf::PdmOptionItemInfo( uiVarNames[oIdx], varNames[oIdx] ) );
            }
        }
        else if ( &m_timeLapseBaseTimestep == fieldNeedingOptions )
        {
            std::vector<std::string> stepNames;
            if ( m_geomCase->geoMechData() )
            {
                stepNames = m_geomCase->geoMechData()->femPartResults()->stepNames();
            }

            options.push_back( caf::PdmOptionItemInfo( QString( "Disabled" ), RigFemResultAddress::noTimeLapseValue() ) );
            for ( size_t stepIdx = 0; stepIdx < stepNames.size(); ++stepIdx )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( QString( "%1 (#%2)" ).arg( QString::fromStdString( stepNames[stepIdx] ) ).arg( stepIdx ),
                                            static_cast<int>( stepIdx ) ) );
            }
        }
        else if ( &m_referenceTimeStep == fieldNeedingOptions )
        {
            std::vector<std::string> stepNames;
            if ( m_geomCase->geoMechData() )
            {
                stepNames = m_geomCase->geoMechData()->femPartResults()->stepNames();
            }

            for ( size_t stepIdx = 0; stepIdx < stepNames.size(); ++stepIdx )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( QString( "%1 (#%2)" ).arg( QString::fromStdString( stepNames[stepIdx] ) ).arg( stepIdx ),
                                            static_cast<int>( stepIdx ) ) );
            }
        }
        else if ( &m_compactionRefLayerUiField == fieldNeedingOptions )
        {
            if ( m_geomCase->geoMechData() )
            {
                size_t kCount = m_geomCase->geoMechData()->femParts()->part( 0 )->getOrCreateStructGrid()->cellCountK();
                for ( size_t layerIdx = 0; layerIdx < kCount; ++layerIdx )
                {
                    options.push_back( caf::PdmOptionItemInfo( QString::number( layerIdx + 1 ), (int)layerIdx ) );
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setGeoMechCase( RimGeoMechCase* geomCase )
{
    m_geomCase = geomCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechResultDefinition::geoMechCase() const
{
    return m_geomCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    m_isChangedByField = true;

    if ( &m_resultPositionTypeUiField == changedField )
    {
        if ( m_resultPositionTypeUiField() == RIG_WELLPATH_DERIVED || m_resultPositionType() == RIG_FORMATION_NAMES )
        {
            m_timeLapseBaseTimestep = RigFemResultAddress::noTimeLapseValue();
            m_timeLapseBaseTimestep.uiCapability()->setUiReadOnly( true );
        }
        else
        {
            m_timeLapseBaseTimestep.uiCapability()->setUiReadOnly( false );
        }
    }

    if ( &m_resultPositionTypeUiField == changedField || &m_timeLapseBaseTimestep == changedField || &m_referenceTimeStep == changedField )
    {
        std::map<std::string, std::vector<std::string>> fieldCompNames = getResultMetaDataForUIFieldSetting();
        QStringList                                     uiVarNames;
        QStringList                                     varNames;

        getUiAndResultVariableStringList( &uiVarNames, &varNames, fieldCompNames );

        if ( m_resultPositionTypeUiField() == m_resultPositionType() &&
             varNames.contains( composeFieldCompString( m_resultFieldName(), m_resultComponentName() ) ) )
        {
            m_resultVariableUiField = composeFieldCompString( m_resultFieldName(), m_resultComponentName() );
        }
        else
        {
            m_resultVariableUiField = "";
        }

        if ( &m_referenceTimeStep == changedField )
        {
            m_geomCase->geoMechData()->femPartResults()->setReferenceTimeStep( m_referenceTimeStep() );
        }
    }
    if ( &m_normalizeByHydrostaticPressure == changedField && m_normalizationAirGap == 0.0 )
    {
        calculateNormalizationAirGapDefault();
        if ( m_normalizeByHydrostaticPressure )
        {
            m_geomCase->geoMechData()->femPartResults()->setNormalizationAirGap( m_normalizationAirGap );
        }
    }

    // Get the possible property filter owner
    auto propFilter        = dynamic_cast<RimGeoMechPropertyFilter*>( parentField()->ownerObject() );
    auto view              = firstAncestorOrThisOfType<RimGridView>();
    auto curve             = firstAncestorOrThisOfType<RimPlotCurve>();
    auto rim3dWellLogCurve = firstAncestorOrThisOfType<Rim3dWellLogCurve>();

    if ( &m_resultVariableUiField == changedField || &m_compactionRefLayerUiField == changedField ||
         &m_timeLapseBaseTimestep == changedField || &m_normalizeByHydrostaticPressure == changedField ||
         &m_normalizationAirGap == changedField || &m_referenceTimeStep == changedField || &m_isChecked == changedField )
    {
        QStringList fieldComponentNames = m_resultVariableUiField().split( QRegularExpression( "\\s+" ) );
        if ( !fieldComponentNames.empty() )
        {
            m_resultPositionType = m_resultPositionTypeUiField;
            if ( m_resultPositionType() == RIG_FORMATION_NAMES )
            {
                // Complete string of selected formation is stored in m_resultFieldName
                m_resultFieldName     = m_resultVariableUiField();
                m_resultComponentName = "";
            }
            else
            {
                m_resultFieldName = fieldComponentNames[0];
                if ( fieldComponentNames.size() > 1 )
                {
                    m_resultComponentName = fieldComponentNames[1];
                }
                else
                {
                    m_resultComponentName = "";
                }

                m_compactionRefLayer = m_compactionRefLayerUiField();
            }

            if ( propFilter )
            {
                propFilter->setToDefaultValues();

                if ( view ) view->scheduleGeometryRegen( PROPERTY_FILTERED );
            }

            if ( view )
            {
                view->scheduleCreateDisplayModelAndRedraw();
                view->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
            }

            if ( dynamic_cast<RimGeoMechCellColors*>( this ) )
            {
                updateLegendCategorySettings();

                if ( view )
                {
                    RimViewLinker* viewLinker = view->assosiatedViewLinker();
                    if ( viewLinker )
                    {
                        viewLinker->updateCellResult();
                    }
                }
            }

            if ( curve )
            {
                curve->loadDataAndUpdate( true );
            }

            if ( rim3dWellLogCurve )
            {
                rim3dWellLogCurve->updateCurveIn3dView();
            }
        }
    }

    if ( rim3dWellLogCurve )
    {
        rim3dWellLogCurve->resetMinMaxValues();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::calculateNormalizationAirGapDefault()
{
    RiaMedianCalculator<double> airGapCalc;
    for ( auto wellPath : RimProject::current()->allWellPaths() )
    {
        if ( wellPath->wellPathGeometry() )
        {
            double airGap = wellPath->wellPathGeometry()->rkbDiff();
            if ( airGap > 0.0 )
            {
                airGapCalc.add( airGap );
            }
        }
    }
    if ( airGapCalc.valid() )
    {
        m_normalizationAirGap = airGapCalc.median();
    }
    else
    {
        m_normalizationAirGap = 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechResultDefinition::useUndoRedoForFieldChanged()
{
    // Do not use undo/redo, because a state variable is initialized in fieldChanged(), and the undo/redo framework
    // trigger a call to defineUiOrdering() before fieldChanged causing invalid state in this object.
    // Having a state variable in fieldChanged/defineUiOrdering is a fragile pattern and should be avoided

    // See CmdFieldChangeExec::redo()

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RimGeoMechResultDefinition::getResultMetaDataForUIFieldSetting()
{
    RimGeoMechCase*                                 gmCase = m_geomCase;
    std::map<std::string, std::vector<std::string>> fieldWithComponentNames;
    if ( gmCase && gmCase->geoMechData() )
    {
        fieldWithComponentNames = gmCase->geoMechData()->femPartResults()->scalarFieldAndComponentNames( m_resultPositionTypeUiField() );
    }

    return fieldWithComponentNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::getUiAndResultVariableStringList( QStringList*                                           uiNames,
                                                                   QStringList*                                           variableNames,
                                                                   const std::map<std::string, std::vector<std::string>>& fieldCompNames )
{
    CVF_ASSERT( uiNames && variableNames );

    std::map<std::string, std::vector<std::string>>::const_iterator fieldIt;
    for ( fieldIt = fieldCompNames.begin(); fieldIt != fieldCompNames.end(); ++fieldIt )
    {
        QString resultFieldName = QString::fromStdString( fieldIt->first );

        if ( resultFieldName == "E" || resultFieldName == "S" || resultFieldName == "POR" )
            continue; // We will not show the native POR, Stress and Strain

        QString resultFieldUiName = convertToUiResultFieldName( resultFieldName );

        uiNames->push_back( resultFieldUiName );
        variableNames->push_back( resultFieldName );

        std::vector<std::string>::const_iterator compIt;
        for ( compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt )
        {
            QString resultCompName = QString::fromStdString( *compIt );
            uiNames->push_back( "   " + resultCompName );
            variableNames->push_back( composeFieldCompString( resultFieldName, resultCompName ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::composeFieldCompString( const QString& resultFieldName, const QString& resultComponentName )
{
    if ( resultComponentName.isEmpty() )
        return resultFieldName;
    else
        return resultFieldName + " " + resultComponentName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::initAfterRead()
{
    if ( m_resultComponentName() == "STM" || m_resultComponentName() == "SEM" ) m_resultComponentName = "SM";

    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultVariableUiField     = composeFieldCompString( m_resultFieldName(), m_resultComponentName() );
    m_compactionRefLayerUiField = m_compactionRefLayer;

    m_timeLapseBaseTimestep.uiCapability()->setUiReadOnly( resultPositionType() == RIG_WELLPATH_DERIVED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_normalizationAirGap )
    {
        auto attr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->m_decimals  = 2;
            attr->m_validator = new QDoubleValidator( 0.0, std::numeric_limits<double>::max(), 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::loadResult()
{
    if ( m_geomCase && m_geomCase->geoMechData() )
    {
        if ( resultAddress().fieldName == RiaResultNames::wbsFGResult().toStdString() ||
             resultAddress().fieldName == RiaResultNames::wbsSFGResult().toStdString() )
        {
            RigFemResultAddress stressResAddr( RIG_ELEMENT_NODAL, std::string( "ST" ), "" );
            RigFemResultAddress porBarResAddr = RigFemAddressDefines::elementNodalPorBarAddress();
            m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded( stressResAddr );
            m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded( porBarResAddr );
        }
        else
        {
            m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded( resultAddress() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setAddWellPathDerivedResults( bool addWellPathDerivedResults )
{
    m_addWellPathDerivedResults = addWellPathDerivedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimGeoMechResultDefinition::resultAddress() const
{
    // Convert differentials to their underlying position type
    if ( resultPositionType() == RIG_DIFFERENTIALS )
    {
        RigFemResultPosEnum resultPositionType = RIG_ELEMENT_NODAL;
        if ( resultFieldName().toStdString() == RigFemAddressDefines::porBar() )
        {
            resultPositionType = RIG_NODAL;
        }

        return RigFemResultAddress( resultPositionType,
                                    resultFieldName().toStdString(),
                                    resultComponentName().toStdString(),
                                    m_timeLapseBaseTimestep(),
                                    RigFemResultAddress::noCompactionValue() );
    }
    else
    {
        RigFemResultAddress address( resultPositionType(),
                                     resultFieldName().toStdString(),
                                     resultComponentName().toStdString(),
                                     m_timeLapseBaseTimestep(),
                                     resultFieldName().toStdString() == RigFemPartResultsCollection::FIELD_NAME_COMPACTION
                                         ? m_compactionRefLayer()
                                         : RigFemResultAddress::noCompactionValue(),
                                     m_normalizeByHydrostaticPressure );
        if ( !RigFemPartResultsCollection::isNormalizableResult( address ) )
        {
            address.normalizedByHydrostaticPressure = false;
        }

        if ( RigFemPartResultsCollection::isReferenceCaseDependentResult( address ) )
        {
            address.timeLapseBaseStepIdx = RigFemResultAddress::noTimeLapseValue();
        }

        return address;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RimGeoMechResultDefinition::observedResults() const
{
    return std::vector<RigFemResultAddress>( 1, resultAddress() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultPosEnum RimGeoMechResultDefinition::resultPositionType() const
{
    return m_resultPositionType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultFieldName() const
{
    if ( !isChecked() ) return "";

    return m_resultFieldName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultComponentName() const
{
    return m_resultComponentName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::diffResultUiName() const
{
    QString diffResultString;
    if ( m_geomCase->geoMechData() )
    {
        if ( referenceCaseDependentResultSelected() )
        {
            std::vector<std::string> timeStepNames  = m_geomCase->geoMechData()->femPartResults()->stepNames();
            QString                  timeStepString = QString::fromStdString( timeStepNames[m_referenceTimeStep()] );
            diffResultString += QString( "<b>Reference Time Step</b>: %1" ).arg( timeStepString );
        }
        else if ( m_timeLapseBaseTimestep != RigFemResultAddress::noTimeLapseValue() )
        {
            std::vector<std::string> timeStepNames  = m_geomCase->geoMechData()->femPartResults()->stepNames();
            QString                  timeStepString = QString::fromStdString( timeStepNames[m_timeLapseBaseTimestep()] );
            diffResultString += QString( "<b>Base Time Step</b>: %1" ).arg( timeStepString );
        }
    }
    return diffResultString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::diffResultUiShortName() const
{
    QString diffResultString;
    if ( m_geomCase->geoMechData() )
    {
        if ( referenceCaseDependentResultSelected() )
        {
            diffResultString += QString( "Ref. Time: #%1" ).arg( m_referenceTimeStep() );
        }
        else if ( m_timeLapseBaseTimestep != RigFemResultAddress::noTimeLapseValue() )
        {
            diffResultString += QString( "Base Time: #%1" ).arg( m_timeLapseBaseTimestep() );
        }
    }
    return diffResultString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData* RimGeoMechResultDefinition::ownerCaseData() const
{
    return m_geomCase ? m_geomCase->geoMechData() : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Is the result probably valid and possible to load
//--------------------------------------------------------------------------------------------------
bool RimGeoMechResultDefinition::hasResult()
{
    RigGeoMechCaseData* caseData = ownerCaseData();
    if ( caseData )
    {
        RigFemPartResultsCollection* results = caseData->femPartResults();
        if ( results )
        {
            return results->assertResultsLoaded( resultAddress() );
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultFieldUiName() const
{
    return convertToUiResultFieldName( m_resultFieldName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultComponentUiName() const
{
    return m_resultComponentName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultVariableUiName() const
{
    QString name        = resultFieldName();
    QString resCompName = resultComponentName();

    if ( !resCompName.isEmpty() )
    {
        name += "." + resCompName;
    }
    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultVariableName() const
{
    if ( m_resultPositionType == RIG_WELLPATH_DERIVED )
    {
        RigWbsParameter param;
        if ( RigWbsParameter::findParameter( resultFieldName(), &param ) )
        {
            QString lasName = param.addressString( RigWbsParameter::LAS_FILE );
            if ( !lasName.isEmpty() ) return lasName;
        }
    }
    return resultVariableUiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::currentResultUnits() const
{
    RigFemResultAddress rigFemResultAddress = resultAddress();

    if ( RigFemPartResultCalculatorStressAnisotropy::isAnisotropyResult( rigFemResultAddress ) )
    {
        return RiaWellLogUnitTools<double>::noUnitString();
    }

    if ( resultFieldName() == "SE" || resultFieldName() == "ST" ||
         resultFieldName() == QString::fromStdString( RigFemAddressDefines::porBar() ) || resultFieldName() == "SM" ||
         resultFieldName() == "SEM" || resultFieldName() == "Q" )
    {
        auto componentName = resultComponentName();
        if ( componentName.endsWith( "azi" ) || componentName.endsWith( "inc" ) )
        {
            return "Deg";
        }

        if ( rigFemResultAddress.normalizeByHydrostaticPressure() )
        {
            return "sg";
        }

        return "Bar";
    }
    else if ( resultFieldName() == "MODULUS" )
    {
        return "GPa";
    }
    else if ( resultFieldName() == "COMPRESSIBILITY" && ( resultComponentName() == "PORE" || resultComponentName() == "VERTICAL" ) )
    {
        return "1/GPa";
    }
    else if ( resultFieldName() == "PORO-PERM" && resultComponentName() == "PERM" )
    {
        return "mD";
    }
    else
    {
        for ( auto resultName : RiaResultNames::wbsDerivedResultNames() )
        {
            if ( resultName == resultFieldName() )
            {
                return RiaWellLogUnitTools<double>::sg_emwUnitString();
            }
        }
    }

    return RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::defaultLasUnits() const
{
    if ( m_resultPositionType == RIG_WELLPATH_DERIVED )
    {
        RigWbsParameter param;
        if ( RigWbsParameter::findParameter( resultFieldName(), &param ) )
        {
            return param.units( RigWbsParameter::LAS_FILE );
        }
    }
    return currentResultUnits();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechResultDefinition::normalizationAirGap() const
{
    return m_normalizationAirGap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setNormalizationAirGap( double airGap )
{
    m_normalizationAirGap = airGap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::convertToUiResultFieldName( QString resultFieldName )
{
    QString newName( resultFieldName );

    if ( resultFieldName == "E" ) newName = "NativeAbaqus Strain";
    if ( resultFieldName == "S" ) newName = "NativeAbaqus Stress";
    if ( resultFieldName == "NE" ) newName = "E"; // Make NE and NS appear as E and SE
    if ( resultFieldName == QString::fromStdString( RigFemAddressDefines::porBar() ) ) newName = "POR"; // POR-Bar appear as POR
    if ( resultFieldName == "MODULUS" ) newName = "Young's Modulus";
    if ( resultFieldName == "RATIO" ) newName = "Poisson's Ratio";
    if ( resultFieldName == "UCS" ) newName = "UCS bar/ 100";

    return newName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechResultDefinition::normalizableResultSelected() const
{
    return RigFemPartResultsCollection::isNormalizableResult( resultAddress() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechResultDefinition::referenceCaseDependentResultSelected() const
{
    return RigFemPartResultsCollection::isReferenceCaseDependentResult( resultAddress() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setResultAddress( const RigFemResultAddress& resultAddress )
{
    m_resultPositionType             = resultAddress.resultPosType;
    m_resultFieldName                = QString::fromStdString( resultAddress.fieldName );
    m_resultComponentName            = QString::fromStdString( resultAddress.componentName );
    m_timeLapseBaseTimestep          = resultAddress.timeLapseBaseStepIdx;
    m_compactionRefLayer             = resultAddress.refKLayerIndex;
    m_normalizeByHydrostaticPressure = resultAddress.normalizedByHydrostaticPressure;

    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultVariableUiField     = composeFieldCompString( m_resultFieldName(), m_resultComponentName() );
    m_compactionRefLayerUiField = m_compactionRefLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::updateLegendTextAndRanges( RimRegularLegendConfig* legendConfigToUpdate,
                                                            const QString&          legendHeading,
                                                            int                     viewerStepIndex )
{
    if ( !ownerCaseData() || !( resultAddress().isValid() ) )
    {
        return;
    }

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    RigGeoMechCaseData* gmCase = ownerCaseData();
    CVF_ASSERT( gmCase );

    RigFemResultAddress resVarAddress = resultAddress();

    auto [stepIdx, frameIdx] = gmCase->femPartResults()->stepListIndexToTimeStepAndDataFrameIndex( viewerStepIndex );

    gmCase->femPartResults()->minMaxScalarValues( resVarAddress, stepIdx, frameIdx, &localMin, &localMax );
    gmCase->femPartResults()->posNegClosestToZero( resVarAddress, stepIdx, frameIdx, &localPosClosestToZero, &localNegClosestToZero );

    gmCase->femPartResults()->minMaxScalarValues( resVarAddress, &globalMin, &globalMax );
    gmCase->femPartResults()->posNegClosestToZero( resVarAddress, &globalPosClosestToZero, &globalNegClosestToZero );

    legendConfigToUpdate->setClosestToZeroValues( globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero );
    legendConfigToUpdate->setAutomaticRanges( globalMin, globalMax, localMin, localMax );

    if ( hasCategoryResult() )
    {
        std::vector<QString> fnVector = gmCase->femPartResults()->formationNames();
        legendConfigToUpdate->setNamedCategories( fnVector );
    }

    QString legendTitle = legendHeading + caf::AppEnum<RigFemResultPosEnum>( resultPositionType() ).uiText() + "\n" + resultFieldUiName();

    if ( !resultComponentUiName().isEmpty() )
    {
        legendTitle += ", " + resultComponentUiName();
    }

    QString unitString = currentResultUnits();
    if ( unitString != RiaWellLogUnitTools<double>::noUnitString() )
    {
        legendTitle += QString( " [%1]" ).arg( unitString );
    }

    if ( !diffResultUiShortName().isEmpty() )
    {
        legendTitle += QString( "\nTime Diff:\n%1" ).arg( diffResultUiShortName() );
    }

    legendConfigToUpdate->setTitle( legendTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechResultDefinition::isBiotCoefficientDependent() const
{
    return ( resultFieldName() == "COMPRESSIBILITY" || resultFieldName() == "PORO-PERM" );
}
