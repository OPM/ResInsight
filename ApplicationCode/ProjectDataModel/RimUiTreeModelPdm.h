/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
#include "cafPdmDocument.h"
#include "cafUiTreeModelPdm.h"

class QFileSystemWatcher;

class RimEclipsePropertyFilter;
class RimCellRangeFilter;
class RimGeoMechPropertyFilter;
class RimEclipseCase;
class RimEclipseView;
class RimEclipseInputProperty;
class RimEclipseStatisticsCase;
class RimIdenticalGridCaseGroup;

class RimView;

    
//==================================================================================================
///
///
//==================================================================================================
class RimUiTreeModelPdm : public caf::UiTreeModelPdm
{
    Q_OBJECT;

public:
    RimUiTreeModelPdm(QObject* parent);

    // Special edit methods
    bool                        deleteRangeFilter(const QModelIndex& itemIndex);
    bool                        deletePropertyFilter(const QModelIndex& itemIndex);
    bool                        deleteGeoMechPropertyFilter(const QModelIndex& itemIndex);

    void                        deleteInputProperty(const QModelIndex& itemIndex);
    void                        deleteReservoir(RimEclipseCase* reservoir);
    void                        deleteAllWellPaths(const QModelIndex& itemIndex);

    RimEclipsePropertyFilter*   addPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimGeoMechPropertyFilter*   addGeoMechPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimCellRangeFilter*         addRangeFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);

    RimView*                    addReservoirView(const std::vector<caf::PdmUiItem*>& treeSelection);
    void                        deleteReservoirViews(const std::vector<caf::PdmUiItem*>& treeSelection);
    void                        deleteGeoMechCases(const std::vector<caf::PdmUiItem*>& treeSelection);

    void                        addInputProperty(const QModelIndex& itemIndex, const QStringList& fileNames);
    
    void                        addToParentAndBuildUiItems(caf::PdmUiTreeItem* parentTreeItem, int position, caf::PdmObject* pdmObject);
    
    void                        populateObjectGroupFromModelIndexList(const QModelIndexList& modelIndexList, caf::PdmObjectGroup* objectGroup);
    void                        addObjects(const QModelIndex& itemIndex, const caf::PdmObjectGroup& pdmObjects);
    void                        moveObjects(const QModelIndex& itemIndex, caf::PdmObjectGroup& pdmObjects);
    
    RimEclipseStatisticsCase*   addStatisticalCalculation(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimIdenticalGridCaseGroup*  addCaseGroup(QModelIndex& insertedModelIndex);

    bool                        deleteObjectFromPdmPointersField(const QModelIndex& itemIndex);

    void                        updateScriptPaths();

    virtual Qt::DropActions     supportedDropActions() const;
    virtual Qt::ItemFlags       flags(const QModelIndex &index) const;
    virtual bool                dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData*          mimeData(const QModelIndexList &indexes) const;
    virtual QStringList         mimeTypes() const;

    RimIdenticalGridCaseGroup*  gridCaseGroupFromItemIndex(const QModelIndex& itemIndex);
    void                        setObjectToggleStateForSelection(QModelIndexList selectedIndexes, int state);

private slots:
    void                        slotRefreshScriptTree(QString path);

private:
    void                        clearClipboard();
    RimEclipseCase*             caseFromItemIndex(const QModelIndex& itemIndex);
private:
    QFileSystemWatcher*         m_scriptChangeDetector;
};

