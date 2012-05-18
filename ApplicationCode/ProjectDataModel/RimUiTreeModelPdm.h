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
#include "RimCellRangeFilter.h"

#include <QTreeView>
#include "cafUiPropertyCreatorPdm.h"

class QFileSystemWatcher;

class RimCellPropertyFilter;

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
    bool            removeRangeFilter(const QModelIndex& itemIndex);
    bool            removePropertyFilter(const QModelIndex& itemIndex);
    bool            removeReservoirView(const QModelIndex& itemIndex);
    
    RimCellPropertyFilter*  addPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimCellRangeFilter*     addRangeFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimReservoirView*       addReservoirView(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);

    void            updateScriptPaths();

private slots:
    void            slotRefreshScriptTree(QString path);

private:
    QFileSystemWatcher* m_scriptChangeDetector;
};



//==================================================================================================
///
///
//==================================================================================================
class RimTreeView : public QTreeView
{
    Q_OBJECT

public:
    RimTreeView(QWidget *parent = 0);

protected:
    void contextMenuEvent(QContextMenuEvent* event);

private slots:
    void slotAddChildItem();
    void slotDeleteItem();
    void slotShowWindow();
    
    void slotAddRangeFilter();
    void slotAddSliceFilterI();
    void slotAddSliceFilterJ();
    void slotAddSliceFilterK();

    void slotAddPropertyFilter();

    void slotDeletePropertyFilter();
    void slotDeleteRangeFilter();

    void slotReadScriptContentFromFile();
    void slotEditScript();
    void slotNewScript();
    void slotExecuteScript();

    void slotAddView();
    void slotDeleteView();
};



class RimUiPropertyCreatorPdm : public caf::UiPropertyCreatorPdm
{
public:
    RimUiPropertyCreatorPdm(QObject* parent);

    virtual void uiFields(const caf::PdmObject* object, std::vector<caf::PdmFieldHandle*>& fields) const;
};
