/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "RicRefinementSettings.h"

#include "RiaLogging.h"

#include "RigNoRefinement.h"
#include "RigNonUniformRefinement.h"
#include "RigUniformRefinement.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiRadioButtonEditor.h"

namespace caf
{
template <>
void AppEnum<RicRefinementSettings::RefinementMode>::setUp()
{
    addItem( RicRefinementSettings::NONE, "NONE", "None" );
    addItem( RicRefinementSettings::UNIFORM, "UNIFORM", "Uniform" );
    addItem( RicRefinementSettings::NON_UNIFORM, "NON_UNIFORM", "Non-Uniform" );
    setDefault( RicRefinementSettings::NONE );
}
template <>
void AppEnum<RicRefinementSettings::NonUniformSubMode>::setUp()
{
    addItem( RicRefinementSettings::CUSTOM_WIDTHS, "CUSTOM_WIDTHS", "Custom Widths" );
    addItem( RicRefinementSettings::LINEAR_EQUAL_SPLIT, "LINEAR_EQUAL_SPLIT", "Linear (Equal Split)" );
    addItem( RicRefinementSettings::LOGARITHMIC_CENTER, "LOGARITHMIC_CENTER", "Logarithmic (Towards Center)" );
    setDefault( RicRefinementSettings::CUSTOM_WIDTHS );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RicRefinementSettings, "RicRefinementSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRefinementSettings::RicRefinementSettings()
    : m_sectorMinI( 1 )
    , m_sectorMinJ( 1 )
    , m_sectorMinK( 1 )
    , m_sectorMaxI( 1 )
    , m_sectorMaxJ( 1 )
    , m_sectorMaxK( 1 )
{
    CAF_PDM_InitObject( "Refinement Settings" );

    CAF_PDM_InitField( &m_refinementCountI, "RefinementCountI", 1, "Cell Count I, J, K" );
    CAF_PDM_InitField( &m_refinementCountJ, "RefinementCountJ", 1, "" );
    CAF_PDM_InitField( &m_refinementCountK, "RefinementCountK", 1, "" );

    m_refinementCountI.setRange( 1, 10 );
    m_refinementCountJ.setRange( 1, 10 );
    m_refinementCountK.setRange( 1, 10 );

    CAF_PDM_InitFieldNoDefault( &m_refinementMode, "RefinementMode", "Refinement Mode" );
    m_refinementMode.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_nonUniformEnableI, "NonUniformEnableI", false, "Enable I Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_nonUniformEnableI );
    CAF_PDM_InitField( &m_nonUniformRangeStartI, "NonUniformRangeStartI", 1, "Grid Cell Start" );
    m_nonUniformRangeStartI.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformRangeEndI, "NonUniformRangeEndI", 1, "Grid Cell End" );
    m_nonUniformRangeEndI.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformIntervalsI, "NonUniformIntervalsI", QString( "0.5, 0.5" ), "Fractional Widths" );

    CAF_PDM_InitField( &m_nonUniformEnableJ, "NonUniformEnableJ", false, "Enable J Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_nonUniformEnableJ );
    CAF_PDM_InitField( &m_nonUniformRangeStartJ, "NonUniformRangeStartJ", 1, "Grid Cell Start" );
    m_nonUniformRangeStartJ.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformRangeEndJ, "NonUniformRangeEndJ", 1, "Grid Cell End" );
    m_nonUniformRangeEndJ.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformIntervalsJ, "NonUniformIntervalsJ", QString( "0.5, 0.5" ), "Fractional Widths" );

    CAF_PDM_InitField( &m_nonUniformEnableK, "NonUniformEnableK", false, "Enable K Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_nonUniformEnableK );
    CAF_PDM_InitField( &m_nonUniformRangeStartK, "NonUniformRangeStartK", 1, "Grid Cell Start" );
    m_nonUniformRangeStartK.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformRangeEndK, "NonUniformRangeEndK", 1, "Grid Cell End" );
    m_nonUniformRangeEndK.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformIntervalsK, "NonUniformIntervalsK", QString( "0.5, 0.5" ), "Fractional Widths" );

    CAF_PDM_InitFieldNoDefault( &m_nonUniformSubMode, "NonUniformSubMode", "Non-Uniform Mode" );

    CAF_PDM_InitField( &m_nonUniformSubcellCountI, "NonUniformSubcellCountI", 2, "Subcells per Cell" );
    m_nonUniformSubcellCountI.setRange( 2, 100 );
    CAF_PDM_InitField( &m_nonUniformSubcellCountJ, "NonUniformSubcellCountJ", 2, "Subcells per Cell" );
    m_nonUniformSubcellCountJ.setRange( 2, 100 );
    CAF_PDM_InitField( &m_nonUniformSubcellCountK, "NonUniformSubcellCountK", 2, "Subcells per Cell" );
    m_nonUniformSubcellCountK.setRange( 2, 100 );

    CAF_PDM_InitField( &m_nonUniformTotalCellsI, "NonUniformTotalCellsI", 10, "Total Cells" );
    m_nonUniformTotalCellsI.setRange( 2, 1000 );
    CAF_PDM_InitField( &m_nonUniformTotalCellsJ, "NonUniformTotalCellsJ", 10, "Total Cells" );
    m_nonUniformTotalCellsJ.setRange( 2, 1000 );
    CAF_PDM_InitField( &m_nonUniformTotalCellsK, "NonUniformTotalCellsK", 10, "Total Cells" );
    m_nonUniformTotalCellsK.setRange( 2, 1000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefinementSettings::setSectorBounds( const caf::VecIjk0& min, const caf::VecIjk0& max )
{
    m_sectorMinI = static_cast<int>( min.x() ) + 1;
    m_sectorMinJ = static_cast<int>( min.y() ) + 1;
    m_sectorMinK = static_cast<int>( min.z() ) + 1;
    m_sectorMaxI = static_cast<int>( max.x() ) + 1;
    m_sectorMaxJ = static_cast<int>( max.y() ) + 1;
    m_sectorMaxK = static_cast<int>( max.z() ) + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigRefinement> RicRefinementSettings::effectiveRefinement() const
{
    size_t sectorSizeI = m_sectorMaxI - m_sectorMinI + 1;
    size_t sectorSizeJ = m_sectorMaxJ - m_sectorMinJ + 1;
    size_t sectorSizeK = m_sectorMaxK - m_sectorMinK + 1;

    cvf::Vec3st sectorSize( sectorSizeI, sectorSizeJ, sectorSizeK );

    if ( m_refinementMode() == NONE ) return std::make_unique<RigNoRefinement>( sectorSize );

    if ( m_refinementMode() == UNIFORM )
    {
        return std::make_unique<RigUniformRefinement>( cvf::Vec3st( m_refinementCountI(), m_refinementCountJ(), m_refinementCountK() ),
                                                       sectorSize );
    }

    return nonUniformRefinement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicRefinementSettings::refinement() const
{
    if ( m_refinementMode() != UNIFORM )
    {
        return cvf::Vec3st( 1, 1, 1 );
    }
    return cvf::Vec3st( m_refinementCountI(), m_refinementCountJ(), m_refinementCountK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRefinementSettings::RefinementMode RicRefinementSettings::refinementMode() const
{
    return m_refinementMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigNonUniformRefinement> RicRefinementSettings::nonUniformRefinement() const
{
    size_t sectorSizeI = m_sectorMaxI - m_sectorMinI + 1;
    size_t sectorSizeJ = m_sectorMaxJ - m_sectorMinJ + 1;
    size_t sectorSizeK = m_sectorMaxK - m_sectorMinK + 1;

    cvf::Vec3st sectorSize( sectorSizeI, sectorSizeJ, sectorSizeK );

    auto result = std::make_unique<RigNonUniformRefinement>( sectorSize );

    if ( m_refinementMode() != NON_UNIFORM ) return result;

    struct DimensionConfig
    {
        bool                               enabled;
        int                                rangeStart;
        int                                rangeEnd;
        QString                            intervals;
        int                                subcellCount;
        int                                totalCells;
        RigNonUniformRefinement::Dimension dim;
    };

    std::vector<DimensionConfig> dims = {
        { m_nonUniformEnableI(),
          m_nonUniformRangeStartI(),
          m_nonUniformRangeEndI(),
          m_nonUniformIntervalsI(),
          m_nonUniformSubcellCountI(),
          m_nonUniformTotalCellsI(),
          RigNonUniformRefinement::DimI },
        { m_nonUniformEnableJ(),
          m_nonUniformRangeStartJ(),
          m_nonUniformRangeEndJ(),
          m_nonUniformIntervalsJ(),
          m_nonUniformSubcellCountJ(),
          m_nonUniformTotalCellsJ(),
          RigNonUniformRefinement::DimJ },
        { m_nonUniformEnableK(),
          m_nonUniformRangeStartK(),
          m_nonUniformRangeEndK(),
          m_nonUniformIntervalsK(),
          m_nonUniformSubcellCountK(),
          m_nonUniformTotalCellsK(),
          RigNonUniformRefinement::DimK },
    };

    // Sector min indices (1-based) for converting original grid coordinates to sector-relative
    int sectorMin[3] = { m_sectorMinI, m_sectorMinJ, m_sectorMinK };

    const QString dimLabels[3] = { "I", "J", "K" };

    auto subMode = m_nonUniformSubMode();

    RiaLogging::info( QString( "Non-uniform refinement: sector size [%1, %2, %3], sector min [%4, %5, %6]" )
                          .arg( sectorSize.x() )
                          .arg( sectorSize.y() )
                          .arg( sectorSize.z() )
                          .arg( sectorMin[0] )
                          .arg( sectorMin[1] )
                          .arg( sectorMin[2] ) );

    for ( const auto& dc : dims )
    {
        if ( !dc.enabled ) continue;

        // Convert from original grid coordinates (1-based) to sector-relative (0-based)
        int sectorStart = dc.rangeStart - sectorMin[static_cast<size_t>( dc.dim )];
        int sectorEnd   = dc.rangeEnd - sectorMin[static_cast<size_t>( dc.dim )];

        // Clamp to sector bounds
        int sectorMaxIdx = static_cast<int>( result->sectorSize( dc.dim ) ) - 1;
        sectorStart      = std::max( 0, sectorStart );
        sectorEnd        = std::min( sectorMaxIdx, sectorEnd );
        if ( sectorStart > sectorEnd )
        {
            RiaLogging::warning( QString( "Non-uniform refinement %1: range [%2, %3] is outside sector after clamping, skipping" )
                                     .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                     .arg( sectorStart )
                                     .arg( sectorEnd ) );
            continue;
        }

        if ( subMode == CUSTOM_WIDTHS )
        {
            auto widths = parseWidths( dc.intervals );
            if ( widths.empty() )
            {
                RiaLogging::warning( QString( "Non-uniform refinement %1: no valid widths parsed from '%2'" )
                                         .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                         .arg( dc.intervals ) );
                continue;
            }

            RiaLogging::info(
                QString( "Non-uniform refinement %1: grid range [%2, %3] -> sector range [%4, %5] (sector size %6, %7 widths)" )
                    .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                    .arg( dc.rangeStart )
                    .arg( dc.rangeEnd )
                    .arg( sectorStart )
                    .arg( sectorEnd )
                    .arg( result->sectorSize( dc.dim ) )
                    .arg( widths.size() ) );

            result->distributeWidthsAcrossCells( dc.dim, sectorStart, sectorEnd, widths );
        }
        else if ( subMode == LINEAR_EQUAL_SPLIT )
        {
            // Each cell in the range gets exactly N equal subcells
            auto equalFractions = RigRefinement::generateEqualFractions( static_cast<size_t>( dc.subcellCount ) );

            for ( int c = sectorStart; c <= sectorEnd; ++c )
            {
                result->setCumulativeFractions( dc.dim, static_cast<size_t>( c ), equalFractions );
            }

            RiaLogging::info( QString( "Non-uniform refinement %1 (linear): %2 subcells per cell in range [%3, %4]" )
                                  .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                  .arg( dc.subcellCount )
                                  .arg( sectorStart )
                                  .arg( sectorEnd ) );
        }
        else if ( subMode == LOGARITHMIC_CENTER )
        {
            auto widths = RigRefinement::generateLogarithmicWidths( static_cast<size_t>( dc.totalCells ) );

            RiaLogging::info( QString( "Non-uniform refinement %1 (logarithmic): %2 total cells across range [%3, %4]" )
                                  .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                  .arg( dc.totalCells )
                                  .arg( sectorStart )
                                  .arg( sectorEnd ) );

            result->distributeWidthsAcrossCells( dc.dim, sectorStart, sectorEnd, widths );
        }

        RiaLogging::info( QString( "Non-uniform refinement %1: total refined count = %2 (was %3)" )
                              .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                              .arg( result->totalRefinedCount( dc.dim ) )
                              .arg( result->sectorSize( dc.dim ) ) );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRefinementSettings::hasNonUniformRefinement() const
{
    return m_refinementMode() == NON_UNIFORM && ( m_nonUniformEnableI() || m_nonUniformEnableJ() || m_nonUniformEnableK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefinementSettings::addToUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_refinementMode );
    uiOrdering.addNewLabel( "" );

    if ( m_refinementMode() == UNIFORM )
    {
        uiOrdering.add( &m_refinementCountI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        uiOrdering.appendToRow( &m_refinementCountJ );
        uiOrdering.appendToRow( &m_refinementCountK );
    }
    else if ( m_refinementMode() == NON_UNIFORM )
    {
        uiOrdering.add( &m_nonUniformSubMode );
        uiOrdering.addNewLabel( "" );

        auto subMode = m_nonUniformSubMode();

        auto addDimensionGroup = [&]( const QString&          label,
                                      caf::PdmField<bool>&    enableField,
                                      caf::PdmField<int>&     rangeStartField,
                                      caf::PdmField<int>&     rangeEndField,
                                      caf::PdmField<QString>& intervalsField,
                                      caf::PdmField<int>&     subcellCountField,
                                      caf::PdmField<int>&     totalCellsField )
        {
            auto* grp = uiOrdering.addNewGroup( label );
            grp->setCollapsedByDefault();
            grp->add( &enableField );
            grp->add( &rangeStartField );
            grp->add( &rangeEndField );

            if ( subMode == CUSTOM_WIDTHS )
            {
                grp->add( &intervalsField );
            }
            else if ( subMode == LINEAR_EQUAL_SPLIT )
            {
                grp->add( &subcellCountField );
            }
            else if ( subMode == LOGARITHMIC_CENTER )
            {
                grp->add( &totalCellsField );
            }

            bool dimEnabled = enableField();
            rangeStartField.uiCapability()->setUiReadOnly( !dimEnabled );
            rangeEndField.uiCapability()->setUiReadOnly( !dimEnabled );
            intervalsField.uiCapability()->setUiReadOnly( !dimEnabled );
            subcellCountField.uiCapability()->setUiReadOnly( !dimEnabled );
            totalCellsField.uiCapability()->setUiReadOnly( !dimEnabled );
        };

        addDimensionGroup( "I Direction",
                           m_nonUniformEnableI,
                           m_nonUniformRangeStartI,
                           m_nonUniformRangeEndI,
                           m_nonUniformIntervalsI,
                           m_nonUniformSubcellCountI,
                           m_nonUniformTotalCellsI );
        addDimensionGroup( "J Direction",
                           m_nonUniformEnableJ,
                           m_nonUniformRangeStartJ,
                           m_nonUniformRangeEndJ,
                           m_nonUniformIntervalsJ,
                           m_nonUniformSubcellCountJ,
                           m_nonUniformTotalCellsJ );
        addDimensionGroup( "K Direction",
                           m_nonUniformEnableK,
                           m_nonUniformRangeStartK,
                           m_nonUniformRangeEndK,
                           m_nonUniformIntervalsK,
                           m_nonUniformSubcellCountK,
                           m_nonUniformTotalCellsK );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RicRefinementSettings::validateSettings() const
{
    std::map<QString, QString> fieldErrors;

    if ( m_refinementMode() == NONE ) return fieldErrors;

    if ( m_refinementMode() == UNIFORM )
    {
        for ( auto& field : { &m_refinementCountI, &m_refinementCountJ, &m_refinementCountK } )
        {
            auto errStr = field->validate();
            if ( !errStr.isEmpty() )
            {
                fieldErrors[field->keyword()] = errStr;
            }
        }
    }
    else
    {
        struct DimValidation
        {
            const caf::PdmField<bool>&    enable;
            const caf::PdmField<int>&     rangeStart;
            const caf::PdmField<int>&     rangeEnd;
            const caf::PdmField<QString>& intervals;
            int                           sectorMin;
            int                           sectorMax;
            QString                       label;
        };

        std::vector<DimValidation> dims = {
            { m_nonUniformEnableI, m_nonUniformRangeStartI, m_nonUniformRangeEndI, m_nonUniformIntervalsI, m_sectorMinI, m_sectorMaxI, "I" },
            { m_nonUniformEnableJ, m_nonUniformRangeStartJ, m_nonUniformRangeEndJ, m_nonUniformIntervalsJ, m_sectorMinJ, m_sectorMaxJ, "J" },
            { m_nonUniformEnableK, m_nonUniformRangeStartK, m_nonUniformRangeEndK, m_nonUniformIntervalsK, m_sectorMinK, m_sectorMaxK, "K" },
        };

        for ( const auto& dv : dims )
        {
            if ( !dv.enable() ) continue;

            if ( dv.rangeStart() > dv.rangeEnd() )
            {
                fieldErrors[dv.rangeStart.keyword()] =
                    QString( "%1 direction: Grid Cell Start cannot be larger than Grid Cell End." ).arg( dv.label );
            }
            if ( dv.rangeStart() < dv.sectorMin || dv.rangeEnd() > dv.sectorMax )
            {
                fieldErrors[dv.rangeEnd.keyword()] =
                    QString( "%1 direction: Refinement range [%2, %3] is outside the sector model range [%4, %5]." )
                        .arg( dv.label )
                        .arg( dv.rangeStart() )
                        .arg( dv.rangeEnd() )
                        .arg( dv.sectorMin )
                        .arg( dv.sectorMax );
            }
            if ( m_nonUniformSubMode() == CUSTOM_WIDTHS && parseWidths( dv.intervals() ).empty() )
            {
                fieldErrors[dv.intervals.keyword()] =
                    QString( "%1 direction: Fractional widths must contain at least one positive value." ).arg( dv.label );
            }
        }
    }

    return fieldErrors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefinementSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    addToUiOrdering( uiOrdering );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRefinementSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( ( changedField == &m_refinementMode ) || ( changedField == &m_nonUniformEnableI ) || ( changedField == &m_nonUniformEnableJ ) ||
         ( changedField == &m_nonUniformEnableK ) || ( changedField == &m_nonUniformSubMode ) )
    {
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RicRefinementSettings::parseWidths( const QString& text )
{
    QStringList         parts = text.split( ",", Qt::SkipEmptyParts );
    std::vector<double> widths;
    for ( const auto& part : parts )
    {
        bool   ok    = false;
        double value = part.trimmed().toDouble( &ok );
        if ( ok && value > 0.0 ) widths.push_back( value );
    }
    return widths;
}
