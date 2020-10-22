/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiaSummaryCurveDefinition.h"

#include <QDialog>

#include <memory>

class QLabel;

class RiuSummaryVectorSelectionUi;
class RiuSummaryVectorSelectionWidgetCreator;

//==================================================================================================
///
///
//==================================================================================================
class RiuSummaryVectorSelectionDialog : public QDialog
{
public:
    RiuSummaryVectorSelectionDialog( QWidget* parent );
    ~RiuSummaryVectorSelectionDialog() override;

    void setCaseAndAddress( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& address );
    void setEnsembleAndAddress( RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& address );
    void setEnsembleAndAddresses( RimSummaryCaseCollection*                    ensemble,
                                  std::vector<const RifEclipseSummaryAddress&> addresses );
    void setCurveSelection( const std::vector<RiaSummaryCurveDefinition>& selection );
    std::vector<RiaSummaryCurveDefinition> curveSelection() const;

    void hideEnsembles();
    void hideSummaryCases();
    void enableMultiSelect( bool enable );
    void enableIndividualEnsembleCaseSelection( bool enable );

private:
    RiuSummaryVectorSelectionUi* summaryAddressSelection() const;
    void                         updateLabel();

private:
    std::unique_ptr<RiuSummaryVectorSelectionWidgetCreator> m_addrSelWidget;

    QLabel* m_label;
};
