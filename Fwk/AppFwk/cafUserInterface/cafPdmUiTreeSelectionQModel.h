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
class OptionItemTreeData
{
public:
    int             childCount;
    QModelIndex     parentModelIndex;
};

//==================================================================================================
/// 
//==================================================================================================
class PdmUiTreeSelectionQModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PdmUiTreeSelectionQModel(QObject *parent = 0);

    void setOptions(caf::PdmUiFieldEditorHandle* field, const QList<caf::PdmOptionItemInfo>& options);

    int                             optionItemCount() const;
    const caf::PdmOptionItemInfo*   optionItem(const QModelIndex &index) const;
    int                             optionItemIndex(const QModelIndex& modelIndex) const;
    std::vector<int>                allSubItemIndices(int headingIndex) const;

    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual int             columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex     parent(const QModelIndex &child) const override;
    virtual int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

signals:
    void                    signalSelectionStateForIndexHasChanged(int index, bool isSelected);

private:
    void                    computeOptionItemTreeData();

private:
    QList<caf::PdmOptionItemInfo>   m_options;
    caf::PdmUiFieldEditorHandle*    m_uiFieldHandle;

    std::vector<OptionItemTreeData> m_optionsTreeData;
    int                             m_zeroLevelRowCount;
    std::map<int, int>              m_zeroLevelRowToOptionIndex;
};


} // end namespace caf
