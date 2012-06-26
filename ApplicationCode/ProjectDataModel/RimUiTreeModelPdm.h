/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafUiTreeModelPdm.h"

class QFileSystemWatcher;

class RimCellPropertyFilter;
class RimCellRangeFilter;
class RimReservoirView;
class RimInputProperty;

//==================================================================================================
///
///
//==================================================================================================
class RimUiTreeModelPdm : public caf::UiTreeModelPdm
{
    Q_OBJECT;

public:
    RimUiTreeModelPdm(QObject* parent);

    // Overrides
    virtual bool    insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());

    // Special edit methods
    bool            deleteRangeFilter(const QModelIndex& itemIndex);
    bool            deletePropertyFilter(const QModelIndex& itemIndex);
    bool            deleteReservoirView(const QModelIndex& itemIndex);
    void            deleteInputProperty(const QModelIndex& itemIndex);

    RimCellPropertyFilter*  addPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimCellRangeFilter*     addRangeFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimReservoirView*       addReservoirView(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    void                    addInputProperty(const QModelIndex& itemIndex, const QStringList& fileNames);

    void            updateScriptPaths();

private slots:
    void            slotRefreshScriptTree(QString path);

private:
    QFileSystemWatcher* m_scriptChangeDetector;
};





