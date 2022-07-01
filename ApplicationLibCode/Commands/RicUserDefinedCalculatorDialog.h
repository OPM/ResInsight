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

#include <QDialog>

class RimUserDefinedCalculation;
class RimUserDefinedCalculationCollection;

//==================================================================================================
///
///
//==================================================================================================
class RicUserDefinedCalculatorDialog : public QDialog
{
    Q_OBJECT

public:
    RicUserDefinedCalculatorDialog( QWidget* parent, const QString& title );
    ~RicUserDefinedCalculatorDialog() override;

    virtual void setCalculationAndUpdateUi( RimUserDefinedCalculation* calculation ) = 0;
    virtual RimUserDefinedCalculationCollection* calculationCollection() const       = 0;
    virtual QWidget*                             getCalculatorWidget()               = 0;
    virtual void                                 updateUi()                          = 0;

private slots:
    void slotTryCloseDialog();

protected:
    void   setUp();
    size_t dirtyCount() const;
};
