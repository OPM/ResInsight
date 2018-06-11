//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015- Ceetron Solutions AS
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

#include <QModelIndexList>

class QMimeData;

namespace caf 
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiDragDropInterface
{
public:
    virtual ~PdmUiDragDropInterface() = 0;

protected:

    friend class PdmUiTreeViewQModel;
    friend class PdmUiTreeViewWidget;

    // Forwarding from Qt functions in QAbstractItemModel
    virtual Qt::DropActions     supportedDropActions() const = 0;
    virtual Qt::ItemFlags       flags(const QModelIndex &index) const = 0;
    virtual bool                dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) = 0;
    virtual QMimeData*          mimeData(const QModelIndexList &indexes) const = 0;
    virtual QStringList         mimeTypes() const = 0;

    // Forwarding of tree view events
    virtual void                onDragCanceled() = 0;
    virtual void                onProposedDropActionUpdated(Qt::DropAction action) = 0;
};

inline PdmUiDragDropInterface::~PdmUiDragDropInterface() { }

} // end namespace caf
