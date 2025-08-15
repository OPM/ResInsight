/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimSummaryAddressSelector.h"

#include "RiaResultNames.h"
#include "Summary/RiaSummaryCurveDefinition.h"
#include "Summary/RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimPlotAxisPropertiesInterface.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryAddressSelector, "RimSummaryAddressSelector" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressSelector::RimSummaryAddressSelector()
    : addressChanged( this )
    , m_showDataSource( true )
    , m_showResampling( true )
    , m_showAxis( true )
    , m_plotAxisOrientation( RimPlotAxisProperties::Orientation::ANY )

{
    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Case" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_summaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_summaryCaseCollection, "SummaryCaseCollection", "Ensemble" );
    m_summaryCaseCollection.uiCapability()->setUiTreeChildrenHidden( true );
    m_summaryCaseCollection.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddressUiField, "summaryAddressUiField", "Vector" );
    m_summaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );
    m_summaryAddressUiField.registerSetMethod( this, &RimSummaryAddressSelector::setAddress );
    m_summaryAddressUiField.registerGetMethod( this, &RimSummaryAddressSelector::summaryAddress );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "SummaryAddress", "Summary Address" );
    m_summaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonSelectSummaryAddress, "SelectAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonSelectSummaryAddress );
    m_pushButtonSelectSummaryAddress = false;

    CAF_PDM_InitFieldNoDefault( &m_plotAxisProperties, "PlotAxisProperties", "Axis" );

    m_summaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_resamplingPeriod, "Resampling", "Resampling" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setSummaryCase( RimSummaryCase* summaryCase )
{
    m_summaryCase = summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_summaryCaseCollection = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setAddress( const RifEclipseSummaryAddress& address )
{
    m_summaryAddress->setAddress( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setResamplingPeriod( RiaDefines::DateTimePeriodEnum resampling )
{
    m_resamplingPeriod = resampling;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setPlotAxisProperties( RimPlotAxisPropertiesInterface* plotAxisProperties )
{
    m_plotAxisProperties = plotAxisProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setShowDataSource( bool enable )
{
    m_showDataSource = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setShowResampling( bool enable )
{
    m_showResampling = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setShowAxis( bool enable )
{
    m_showAxis = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::setAxisOrientation( RimPlotAxisProperties::Orientation orientation )
{
    m_plotAxisOrientation = orientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryAddressSelector::summaryCase() const
{
    return m_summaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimSummaryAddressSelector::ensemble() const
{
    return m_summaryCaseCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddressSelector::summaryAddress() const
{
    return m_summaryAddress()->address();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DateTimePeriodEnum RimSummaryAddressSelector::resamplingPeriod() const
{
    return m_resamplingPeriod();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface* RimSummaryAddressSelector::plotAxisProperties() const
{
    return m_plotAxisProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_pushButtonSelectSummaryAddress )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );

        if ( isEnsemble() )
        {
            dlg.hideSummaryCases();
            dlg.setEnsembleAndAddress( m_summaryCaseCollection(), m_summaryAddress->address() );
        }
        else
        {
            dlg.hideEnsembles();
            dlg.setCaseAndAddress( m_summaryCase(), m_summaryAddress->address() );
        }

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_summaryCase           = curveSelection[0].summaryCaseY();
                m_summaryCaseCollection = curveSelection[0].ensemble();
                auto addr               = curveSelection[0].summaryAddressY();
                m_summaryAddress->setAddress( addr );
                m_summaryAddressUiField = addr;
            }
        }

        m_pushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_summaryAddressUiField )
    {
        m_summaryAddress->setAddress( m_summaryAddressUiField() );
    }

    addressChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto createOptionsForSummaryCase = []() -> QList<caf::PdmOptionItemInfo>
{
    RimProject*                  proj  = RimProject::current();
    std::vector<RimSummaryCase*> cases = proj->allSummaryCases();

    bool includeEnsembleName = false;
    auto options             = RiaSummaryTools::optionsForSummaryCases( cases, includeEnsembleName );
    if ( !options.empty() )
    {
        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto createOptionsForEnsemble = []() -> QList<caf::PdmOptionItemInfo>
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject*                      proj      = RimProject::current();
    std::vector<RimSummaryEnsemble*> ensembles = proj->summaryEnsembles();

    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        if ( ensemble->isEnsemble() ) options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
    }

    options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    return options;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto createOptionsForAddresses = []( const std::set<RifEclipseSummaryAddress>& allAddresses ) -> QList<caf::PdmOptionItemInfo>
{
    QList<caf::PdmOptionItemInfo> options;

    for ( auto& address : allAddresses )
    {
        if ( address.isErrorResult() ) continue;

        std::string name = address.uiText();
        QString     s    = QString::fromStdString( name );
        options.push_back( caf::PdmOptionItemInfo( s, QVariant::fromValue( address ) ) );
    }

    options.push_front( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
    return options;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryAddressSelector::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( fieldNeedingOptions == &m_summaryCase )
    {
        return createOptionsForSummaryCase();
    }

    if ( fieldNeedingOptions == &m_summaryCaseCollection )
    {
        return createOptionsForEnsemble();
    }

    if ( fieldNeedingOptions == &m_summaryAddressUiField )
    {
        std::set<RifEclipseSummaryAddress> addresses;
        if ( isEnsemble() && m_summaryCaseCollection() )
        {
            addresses = m_summaryCaseCollection()->ensembleSummaryAddresses();
        }
        else if ( m_summaryCase() )
        {
            RifSummaryReaderInterface* reader = m_summaryCase()->summaryReader();
            if ( reader )
            {
                addresses = reader->allResultAddresses();
            }
        }

        return createOptionsForAddresses( addresses );
    }

    if ( fieldNeedingOptions == &m_plotAxisProperties )
    {
        if ( auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>() )
        {
            QList<caf::PdmOptionItemInfo> options;
            for ( auto axis : plot->plotAxes( m_plotAxisOrientation ) )
            {
                options.push_back( caf::PdmOptionItemInfo( axis->objectName(), axis ) );
            }

            return options;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_showDataSource )
    {
        if ( isEnsemble() )
        {
            uiOrdering.add( &m_summaryCaseCollection );
        }
        else
        {
            uiOrdering.add( &m_summaryCase );
        }
    }

    // Update the UI field, as this is not serialized to file
    m_summaryAddressUiField = m_summaryAddress->address();

    uiOrdering.add( &m_summaryAddressUiField, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    uiOrdering.add( &m_pushButtonSelectSummaryAddress, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );

    if ( m_showResampling )
    {
        uiOrdering.add( &m_resamplingPeriod, { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );
    }

    if ( m_showAxis )
    {
        uiOrdering.add( &m_plotAxisProperties, { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressSelector::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_pushButtonSelectSummaryAddress == field )
    {
        auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "...";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddressSelector::isEnsemble() const
{
    return m_summaryCaseCollection() != nullptr;
}
