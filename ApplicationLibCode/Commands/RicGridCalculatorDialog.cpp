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

#include "RicGridCalculatorDialog.h"

#include <memory>

#include "RicCalculatorWidgetCreator.h"
#include "RicGridCalculatorUi.h"

#include "RimGridCalculation.h"
#include "RimGridCalculationCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridCalculatorDialog::RicGridCalculatorDialog( QWidget* parent )
    : RicUserDefinedCalculatorDialog( parent, "Grid Property Calculator" )
{
    setUp();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridCalculatorDialog::~RicGridCalculatorDialog()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorDialog::setCalculationAndUpdateUi( RimUserDefinedCalculation* calculation )
{
    CAF_ASSERT( m_calcEditor );
    m_calcEditor->calculator()->setCurrentCalculation( calculation );
    updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorDialog::updateUi()
{
    CAF_ASSERT( m_calcEditor );
    m_calcEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationCollection* RicGridCalculatorDialog::calculationCollection() const
{
    CAF_ASSERT( m_calcEditor );
    return m_calcEditor->calculator()->calculationCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RicGridCalculatorDialog::getCalculatorWidget()
{
    if ( !m_calcEditor )
    {
        m_calcEditor = std::make_unique<RicCalculatorWidgetCreator>( std::make_unique<RicGridCalculatorUi>() );
    }

    return m_calcEditor->getOrCreateWidget( this );
}
