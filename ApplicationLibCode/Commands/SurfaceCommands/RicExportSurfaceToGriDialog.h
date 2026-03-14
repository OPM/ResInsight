/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

class QDialogButtonBox;
class QDoubleSpinBox;
class QSpinBox;

//==================================================================================================
/// Result struct holding the regular grid parameters chosen by the user.
//==================================================================================================
struct RicGriExportGridParams
{
    bool   accepted   = false;
    int    nx         = 10;
    int    ny         = 10;
    double originX    = 0.0;
    double originY    = 0.0;
    double incrementX = 1.0;
    double incrementY = 1.0;
};

//==================================================================================================
/// Dialog for specifying a regular grid to resample an unstructured surface onto before
/// exporting to the GRI (IRAP binary) format.
//==================================================================================================
class RicExportSurfaceToGriDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RicExportSurfaceToGriDialog( QWidget* parent = nullptr );

    static RicGriExportGridParams openDialog( QWidget*                  parent,
                                              const RicGriExportGridParams& defaults );

    RicGriExportGridParams exportParams() const;

private slots:
    void slotOkClicked();
    void slotCancelClicked();

private:
    QSpinBox*      m_nx;
    QSpinBox*      m_ny;
    QDoubleSpinBox* m_originX;
    QDoubleSpinBox* m_originY;
    QDoubleSpinBox* m_incrementX;
    QDoubleSpinBox* m_incrementY;

    QDialogButtonBox* m_buttons;
};
