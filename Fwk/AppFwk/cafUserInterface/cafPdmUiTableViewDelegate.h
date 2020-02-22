//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once


#include <QStyledItemDelegate>


namespace caf 
{

class PdmUiTableViewQModel;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PdmUiTableViewDelegate(QObject* parent, PdmUiTableViewQModel* model);
    ~PdmUiTableViewDelegate() override;

    QWidget*    createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void        setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void        updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    bool        isEditorOpen() const;

    protected slots:
    void        slotEditorDestroyed(QObject* obj);

protected:
    void        paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    PdmUiTableViewQModel* m_model;

    // Counter for active table cell editors 
    mutable int          m_activeEditorCount;
};



} // end namespace caf
