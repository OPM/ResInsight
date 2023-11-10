/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RicGridCalculatorUi.h"

#include "RimGridCalculationCollection.h"
#include "RimProject.h"
#include "RimUserDefinedCalculationCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"

CAF_PDM_SOURCE_INIT( RicGridCalculatorUi, "RicGridCalculator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridCalculatorUi::RicGridCalculatorUi()
{
    CAF_PDM_InitObject( "RicGridCalculator" );

    CAF_PDM_InitFieldNoDefault( &m_importCalculations, "ImportCalculations", "Import Calculations" );
    RicUserDefinedCalculatorUi::assignPushButtonEditor( &m_importCalculations );

    CAF_PDM_InitFieldNoDefault( &m_exportCalculations, "ExportCalculations", "Export Calculations" );
    RicUserDefinedCalculatorUi::assignPushButtonEditor( &m_exportCalculations );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicGridCalculatorUi::calculationsGroupName() const
{
    return "CalculationsGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicGridCalculatorUi::calulationGroupName() const
{
    return "CalulationGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorUi::notifyCalculatedNameChanged( int id, const QString& newName ) const
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RicUserDefinedCalculatorUi::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* group = uiOrdering.findGroup( calculationsGroupName() );
    if ( group )
    {
        group->add( &m_importCalculations );
        group->appendToRow( &m_exportCalculations );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RicUserDefinedCalculatorUi::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_importCalculations )
    {
        if ( auto feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicImportGridCalculationExpressionsFeature" ) )
        {
            feature->action()->trigger();
        }

        m_importCalculations = false;
    }
    else if ( changedField == &m_exportCalculations )
    {
        if ( auto feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicExportGridCalculationExpressionsFeature" ) )
        {
            feature->action()->trigger();
        }

        m_exportCalculations = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RicUserDefinedCalculatorUi::defineEditorAttribute( field, uiConfigName, attribute );

    if ( &m_importCalculations == field )
    {
        RicUserDefinedCalculatorUi::assignPushButtonEditorText( attribute, "Import Calculations" );
    }
    else if ( &m_exportCalculations == field )
    {
        RicUserDefinedCalculatorUi::assignPushButtonEditorText( attribute, "Export Calculations" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationCollection* RicGridCalculatorUi::calculationCollection() const
{
    return RimProject::current()->gridCalculationCollection();
}
