/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

class RifEclipseSummaryAddress;
class RimSummaryNameHelper;
class RiaSummaryCurveAddress;

class RimSummaryCurveAutoName : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveAutoName();

    QString curveName( const RiaSummaryCurveAddress& summaryCurveAddress,
                       const RimSummaryNameHelper*   currentNameHelper,
                       const RimSummaryNameHelper*   plotNameHelper ) const;

    void enableVectorName( bool enable );

private:
    QString curveNameY( const RifEclipseSummaryAddress& summaryAddress,
                        const RimSummaryNameHelper*     currentNameHelper,
                        const RimSummaryNameHelper*     plotNameHelper ) const;

    QString curveNameX( const RifEclipseSummaryAddress& summaryAddress,
                        const RimSummaryNameHelper*     currentNameHelper,
                        const RimSummaryNameHelper*     plotNameHelper ) const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void appendAddressDetails( std::string& text, const RifEclipseSummaryAddress& summaryAddress, const RimSummaryNameHelper* nameHelper ) const;
    void appendWellName( std::string& text, const RifEclipseSummaryAddress& summaryAddress, const RimSummaryNameHelper* nameHelper ) const;
    void appendLgrName( std::string& text, const RifEclipseSummaryAddress& summaryAddress ) const;

    QString buildCurveName( const RifEclipseSummaryAddress& summaryAddress,
                            const RimSummaryNameHelper*     currentNameHelper,
                            const RimSummaryNameHelper*     plotNameHelper,
                            const std::string&              unitText,
                            const std::string&              caseName ) const;

private:
    caf::PdmField<bool> m_vectorName;
    caf::PdmField<bool> m_longVectorName;
    caf::PdmField<bool> m_unit;
    caf::PdmField<bool> m_regionNumber;
    caf::PdmField<bool> m_groupName;
    caf::PdmField<bool> m_wellName;
    caf::PdmField<bool> m_wellSegmentNumber;
    caf::PdmField<bool> m_wellCompletionNumber;
    caf::PdmField<bool> m_lgrName;
    caf::PdmField<bool> m_connection;
    caf::PdmField<bool> m_aquiferNumber;

    caf::PdmField<bool> m_caseName;

    caf::PdmField<bool> m_showAdvancedProperties;
};
