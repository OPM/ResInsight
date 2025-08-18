/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#pragma once

#include "VerticalFlowPerformance/RimVfpDefines.h"

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <QString>

#include <optional>
#include <vector>

namespace Opm
{
class VFPInjTable;
class VFPProdTable;
} // namespace Opm

class VfpPlotData
{
public:
    void setXAxisTitle( const QString& xAxisTitle ) { m_xAxisTitle = xAxisTitle; }
    void setYAxisTitle( const QString& yAxisTitle ) { m_yAxisTitle = yAxisTitle; }

    const QString& xAxisTitle() const { return m_xAxisTitle; }
    const QString& yAxisTitle() const { return m_yAxisTitle; }

    void appendCurve( const QString& curveTitle, const std::vector<double>& xData, const std::vector<double>& yData )
    {
        m_curveTitles.push_back( curveTitle );
        m_xData.push_back( xData );
        m_yData.push_back( yData );
    }

    const QString& curveTitle( size_t idx ) const { return m_curveTitles[idx]; }

    size_t size() const { return m_xData.size(); }

    size_t curveSize( size_t idx ) const { return m_xData[idx].size(); }

    const std::vector<double>& xData( size_t idx ) const { return m_xData[idx]; }
    const std::vector<double>& yData( size_t idx ) const { return m_yData[idx]; }

private:
    QString                          m_xAxisTitle;
    QString                          m_yAxisTitle;
    std::vector<QString>             m_curveTitles;
    std::vector<std::vector<double>> m_xData;
    std::vector<std::vector<double>> m_yData;
};

struct VfpTableSelection
{
    int flowRateIdx;
    int thpIdx;
    int articifialLiftQuantityIdx;
    int waterCutIdx;
    int gasLiquidRatioIdx;
};

struct VfpValueSelection
{
    double flowRateValue;
    double thpValue;
    double artificialLiftQuantityValue;
    double waterCutValue;
    double gasLiquidRatioValue;

    std::vector<double> familyValues;
};

struct VfpTableInitialData
{
    bool                                    isProductionTable;
    int                                     tableNumber;
    double                                  datumDepth;
    RimVfpDefines::FlowingPhaseType         flowingPhase;
    RimVfpDefines::FlowingWaterFractionType waterFraction;
    RimVfpDefines::FlowingGasFractionType   gasFraction;
};

//==================================================================================================
///
//==================================================================================================
class RigVfpTables
{
public:
    void setUnitSystem( const Opm::UnitSystem& unitSystem );
    void addInjectionTable( const Opm::VFPInjTable& table );
    void addProductionTable( const Opm::VFPProdTable& table );

    std::vector<int> injectionTableNumbers() const;
    std::vector<int> productionTableNumbers() const;

    VfpTableInitialData getTableInitialData( int tableIndex ) const;

    std::vector<double> getProductionTableData( int tableIndex, RimVfpDefines::ProductionVariableType variableType ) const;

    VfpPlotData populatePlotData( int                                     tableIndex,
                                  RimVfpDefines::ProductionVariableType   primaryVariable,
                                  RimVfpDefines::ProductionVariableType   familyVariable,
                                  RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                  RimVfpDefines::FlowingPhaseType         flowingPhase,
                                  const VfpTableSelection&                tableSelection ) const;

    VfpPlotData populatePlotData( int                                     tableIndex,
                                  RimVfpDefines::ProductionVariableType   primaryVariable,
                                  RimVfpDefines::ProductionVariableType   familyVariable,
                                  RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                  RimVfpDefines::FlowingPhaseType         flowingPhase,
                                  const VfpValueSelection&                valueSelection ) const;

    QString asciiDataForTable( int                                     tableNumber,
                               RimVfpDefines::ProductionVariableType   primaryVariable,
                               RimVfpDefines::ProductionVariableType   familyVariable,
                               RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                               RimVfpDefines::FlowingPhaseType         flowingPhase,
                               const VfpTableSelection&                tableSelection ) const;

    // Returns the indices of the closest values in valuesToMatch for each value in sourceValues. Returned index value -1 indicates no
    // match. A index value is only returned once.
    static std::vector<int> uniqueClosestIndices( const std::vector<double>& sourceValues, const std::vector<double>& valuesToMatch );

private:
    static VfpPlotData populatePlotData( const Opm::VFPInjTable&                 table,
                                         RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                         RimVfpDefines::FlowingPhaseType         flowingPhase );

    VfpPlotData populatePlotData( const Opm::VFPProdTable&                table,
                                  RimVfpDefines::ProductionVariableType   primaryVariable,
                                  RimVfpDefines::ProductionVariableType   familyVariable,
                                  RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                  RimVfpDefines::FlowingPhaseType         flowingPhase,
                                  const VfpTableSelection&                tableSelection ) const;

    VfpPlotData populatePlotData( const Opm::VFPProdTable&                table,
                                  RimVfpDefines::ProductionVariableType   primaryVariable,
                                  RimVfpDefines::ProductionVariableType   familyVariable,
                                  RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                  RimVfpDefines::FlowingPhaseType         flowingPhase,
                                  const VfpValueSelection&                valueSelection ) const;

    static QString axisTitle( RimVfpDefines::ProductionVariableType variableType, RimVfpDefines::FlowingPhaseType flowingPhase );
    static QString getDisplayUnit( RimVfpDefines::ProductionVariableType variableType );
    static QString getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType );

    static double convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType );
    static void   convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType );

    static QString textForPlotData( const VfpPlotData& plotData );

    std::vector<double> getProductionTableData( const Opm::VFPProdTable& table, RimVfpDefines::ProductionVariableType variableType ) const;
    size_t              getVariableIndex( const Opm::VFPProdTable&              table,
                                          RimVfpDefines::ProductionVariableType targetVariable,
                                          RimVfpDefines::ProductionVariableType primaryVariable,
                                          size_t                                primaryValue,
                                          RimVfpDefines::ProductionVariableType familyVariable,
                                          size_t                                familyValue,
                                          const VfpTableSelection&              tableSelection ) const;

    size_t getVariableIndexForValue( const Opm::VFPProdTable&              table,
                                     RimVfpDefines::ProductionVariableType targetVariable,
                                     RimVfpDefines::ProductionVariableType primaryVariable,
                                     double                                primaryValue,
                                     RimVfpDefines::ProductionVariableType familyVariable,
                                     double                                familyValue,
                                     const VfpValueSelection&              valueSelection ) const;

    std::optional<Opm::VFPInjTable>  injectionTable( int tableNumber ) const;
    std::optional<Opm::VFPProdTable> productionTable( int tableNumber ) const;

    static RimVfpDefines::FlowingPhaseType         getFlowingPhaseType( const Opm::VFPProdTable& table );
    static RimVfpDefines::FlowingPhaseType         getFlowingPhaseType( const Opm::VFPInjTable& table );
    static RimVfpDefines::FlowingWaterFractionType getFlowingWaterFractionType( const Opm::VFPProdTable& table );
    static RimVfpDefines::FlowingGasFractionType   getFlowingGasFractionType( const Opm::VFPProdTable& table );

    // Returns the indices of the closest values in valuesToMatch for each value in sourceValues. Returned index value -1 indicates no match.
    static std::vector<int> findClosestIndices( const std::vector<double>& sourceValues, const std::vector<double>& valuesToMatch );

private:
    std::vector<Opm::VFPInjTable>  m_injectionTables;
    std::vector<Opm::VFPProdTable> m_productionTables;
    Opm::UnitSystem                m_unitSystem;
};
