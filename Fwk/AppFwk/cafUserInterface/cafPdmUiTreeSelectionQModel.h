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
#include "cafUiTreeItem.h"

#include <QAbstractItemModel>

#include <vector>
#include <map>

namespace caf 
{

class PdmOptionItemInfo;
class PdmUiFieldHandle;


//==================================================================================================
/// 
//==================================================================================================
class PdmUiTreeSelectionQModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PdmUiTreeSelectionQModel(QObject *parent = 0);
    ~PdmUiTreeSelectionQModel();

    static int              headingRole();
    static int              optionItemValueRole();

    void                    setCheckedStateForItems(const QModelIndexList& indices, bool checked);

    int                     optionItemCount() const;
    void                    setOptions(caf::PdmUiFieldEditorHandle* field, const QList<caf::PdmOptionItemInfo>& options);

    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual int             columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex     parent(const QModelIndex &child) const override;
    virtual int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Consider moving these functions to PdmUiFieldHandle
    static bool                            isSingleValueField(const QVariant& fieldValue);
    static bool                            isMultipleValueField(const QVariant& fieldValue);

private:
    typedef caf::UiTreeItem<int> TreeItemType;

    const caf::PdmOptionItemInfo*   optionItem(const QModelIndex &index) const;
    int                             optionIndex(const QModelIndex &index) const;
    void                            buildOptionItemTree(int optionIndex, TreeItemType* parentNode);

    void                            notifyChangedForAllModelIndices();
    void                            recursiveNotifyChildren(const QModelIndex& index);


private:
    QList<caf::PdmOptionItemInfo>   m_options;
    caf::PdmUiFieldEditorHandle*    m_uiFieldHandle;

    TreeItemType*                   m_tree;
};


} // end namespace caf
