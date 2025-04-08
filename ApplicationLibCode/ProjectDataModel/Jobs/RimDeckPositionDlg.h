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

#include <QDialog>
#include <QString>

#include <utility>
#include <vector>

class QListWidget;
class QDialogButtonBox;

//==================================================================================================
///
//==================================================================================================
class RimDeckPositionDlg : public QDialog
{
    Q_OBJECT

public:
    RimDeckPositionDlg( QWidget* parent );
    ~RimDeckPositionDlg() override;

    static int askForPosition( QWidget* parent, std::vector<std::pair<int, QString>> items, QString newItemName, int defPosition );

private:
    void addItems( std::vector<std::pair<int, QString>> items );
    void addNewItem( QString name, int defPosition );
    int  selectedPosition() const;

private slots:
    void slotDialogOkClicked();
    void slotDialogCancelClicked();
    void slotMoveUpClicked();
    void slotMoveDownClicked();

private:
    QListWidget*      m_list;
    QDialogButtonBox* m_buttons;
    int               m_selectedPosition;
};
