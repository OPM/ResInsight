/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "cvfVector3.h"

#include <QDialog>

class QRadioButton;
class QLineEdit;
class QPushButton;
class QLabel;

namespace caf
{
class VecIjk;
}

class RiuEclipseSelectionItem;
class RigMainGrid;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuCellSelectionTool : public QDialog
{
    Q_OBJECT

public:
    RiuCellSelectionTool( QWidget* parent = nullptr );

private slots:
    void validateAndSelect();
    void validateAndAppend();

private:
    static QVector<double>   parseDoubleValues( const QString& input );
    RiuEclipseSelectionItem* createSelectionItemFromInput();
    void                     updateVisibleUiItems();
    void                     setupUI();

    static size_t findTopCellIndex( double easting, double northing, const RigMainGrid* mainGrid );
    static size_t findCellForPoint( const cvf::Vec3d& source, double distance, const RigMainGrid* mainGrid );

private:
    QRadioButton* m_xyzRadio;
    QRadioButton* m_ijkRadio;

    QLabel*    m_coordinateLabel;
    QLineEdit* m_coordinateEdit;
    QLabel*    m_cellLabel;
    QLineEdit* m_cellEdit;

    QPushButton* m_submitButton;
    QPushButton* m_appendButton;
};
