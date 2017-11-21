/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimPlotCurve.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafAppEnum.h"

#include <QString>
#include <QLocale>

//==================================================================================================
/// 
//==================================================================================================
class RicPasteAsciiDataToSummaryPlotFeatureUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum DecimalSeparator
    {
        DECIMAL_COMMA,
        DECIMAL_DOT,
    };

    typedef caf::AppEnum<DecimalSeparator> DecimalSeparatorEnum;

    enum DateFormat
    {
        DATE_DDMMYYYY_DOT_SEPARATED,
    };

    typedef caf::AppEnum<DateFormat> DateFormatEnum;

    enum TimeFormat
    {
        TIME_NONE,
        TIME_HHMM,
        TIME_HHMMSS,
        TIME_HHMMSSZZZ,
    };

    typedef caf::AppEnum<TimeFormat> TimeFormatEnum;

    enum CellSeparator
    {
        CELL_COMMA,
        CELL_TAB,
        CELL_SEMICOLON
    };

    typedef caf::AppEnum<CellSeparator> CellSeparatorEnum;

public:
    RicPasteAsciiDataToSummaryPlotFeatureUi();

    QString                       dateFormat()          const;
    QLocale                       decimalLocale()       const;
    QString                       cellSeparator()       const;
    QString                       plotTitle()           const;
    QString                       curvePrefix()         const;
    RimPlotCurve::LineStyleEnum   lineStyle()           const;
    RimPlotCurve::PointSymbolEnum pointSymbol()         const;
    float                         symbolSkipDinstance() const;

    void    createNewPlot();
    void    setPreviewText(const QString& text);

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    caf::PdmField<QString>              m_plotTitle;
    caf::PdmField<QString>              m_curvePrefix;
    caf::PdmField<DecimalSeparatorEnum> m_decimalSeparator;
    caf::PdmField<DateFormatEnum>       m_dateFormat;
    caf::PdmField<TimeFormatEnum>       m_timeFormat;
    caf::PdmField<bool>                 m_useCustomDateFormat;
    caf::PdmField<QString>              m_customDateFormat;
    caf::PdmField<CellSeparatorEnum>    m_cellSeparator;

    caf::PdmField<caf::AppEnum<RimPlotCurve::LineStyleEnum>>   m_curveLineStyle;
    caf::PdmField<caf::AppEnum<RimPlotCurve::PointSymbolEnum>> m_curveSymbol;
    caf::PdmField<float>                                       m_curveSymbolSkipDistance;

    bool                                m_createNewPlot;

    caf::PdmField<QString>              m_previewText;
};
