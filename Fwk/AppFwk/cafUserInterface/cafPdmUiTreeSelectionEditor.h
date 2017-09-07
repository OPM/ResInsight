//##################################################################################################
//
//   Custom Visualization Core library
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

#include "cafPdmUiFieldEditorHandle.h"

class QLabel;
class QTreeView;
class QAbstractItemModel;

namespace caf
{

//==================================================================================================
/// 
//==================================================================================================
class PdmUiTreeSelectionEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTreeSelectionEditor(); 
    virtual ~PdmUiTreeSelectionEditor(); 

protected:
    virtual QWidget*    createEditorWidget(QWidget * parent);
    virtual QWidget*    createLabelWidget(QWidget * parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

private slots:
    void                slotSetSelectionStateForIndex(int index, bool setSelected);
    void                customMenuRequested(const QPoint& pos);

    void                slotSetSelectedOn();
    void                slotSetSelectedOff();
    void                slotSetSubItemsOn();
    void                slotSetSubItemsOff();

private:
    std::vector<int>    selectedCheckableItems() const;
    std::vector<int>    selectedHeaderItems() const;
    void                setSelectionStateForIndices(const std::vector<int>& indices, bool setSelected);

private:
    QPointer<QTreeView> m_treeView;
    QPointer<QLabel>    m_label;
};

} // end namespace caf
