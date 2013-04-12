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

#include <QTreeView>

class QItemSelection;
class RimIdenticalGridCaseGroup;

namespace caf {
    class PdmObjectGroup;
}

//==================================================================================================
///
///
//==================================================================================================
class RimUiTreeView : public QTreeView
{
    Q_OBJECT

public:
    RimUiTreeView(QWidget *parent = 0);
    ~RimUiTreeView();

    virtual void setModel(QAbstractItemModel* model);

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
    void slotDeleteRangeFilter();

    void slotAddPropertyFilter();
    void slotDeletePropertyFilter();

    void slotReadScriptContentFromFile();
    void slotEditScript();
    void slotNewScript();
    void slotExecuteScript();

    void slotAddView();
    void slotDeleteView();

    void slotAddInputProperty();
    void slotDeleteObjectFromContainer();
    void slotWriteInputProperty();
    void slotWriteBinaryResultAsInputProperty();

    void slotCloseCase();

    void slotNewStatisticsCase();
    void slotComputeStatistics();
    
    void slotAddCaseGroup();
    void slotDeleteObjectFromPdmPointersField();

    void slotCopyPdmObjectToClipboard();
    void slotPastePdmObjects();

    void slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

signals:
    void selectedObjectChanged( caf::PdmObject* pdmObject );

private:
    bool userConfirmedGridCaseGroupChange(const QModelIndexList& itemIndexList);
    bool hasAnyStatisticsResults(RimIdenticalGridCaseGroup* gridCaseGroup);

    void createPdmObjectsFromClipboard(caf::PdmObjectGroup* objectGroup);
    bool hasClipboardValidData();

    virtual void keyPressEvent(QKeyEvent* keyEvent);
    virtual void dropEvent(QDropEvent* dropEvent);

private:
    QAction* m_pasteAction;
};


