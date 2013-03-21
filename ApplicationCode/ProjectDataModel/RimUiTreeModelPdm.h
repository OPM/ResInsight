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
#include "cafPdmDocument.h"
#include "cafUiTreeModelPdm.h"

#include <QMimeData>

class QFileSystemWatcher;

class RimCellPropertyFilter;
class RimCellRangeFilter;
class RimReservoir;
class RimReservoirView;
class RimInputProperty;
class RimStatisticalCalculation;
class RimIdenticalGridCaseGroup;

//--------------------------------------------------------------------------------------------------
/// MimeData class used to carry a QModelIndexList
//--------------------------------------------------------------------------------------------------
class MimeDataWithIndexes : public QMimeData
{
    Q_OBJECT

public:
    MimeDataWithIndexes()
    {
    }


    MimeDataWithIndexes(const MimeDataWithIndexes & other) : QMimeData()
    {
        setIndexes(other.indexes());
    }

    void setIndexes(const QModelIndexList & indexes)
    {
        m_indexes = indexes;
    }

    const QModelIndexList& indexes() const { return m_indexes; }

    virtual bool hasFormat( const QString &mimetype ) const
    {
        return (mimetype == formatName());
    }

    virtual QStringList formats() const
    {
        QStringList supportedFormats = QMimeData::formats();
        supportedFormats << formatName();

        return supportedFormats;
    }

    static QString formatName()
    {
        return "MimeDataWithIndexes";
    }

private:
    QModelIndexList m_indexes;
};

Q_DECLARE_METATYPE(MimeDataWithIndexes)

    
//==================================================================================================
///
///
//==================================================================================================
class RimUiTreeModelPdm : public caf::UiTreeModelPdm
{
    Q_OBJECT;

public:
    RimUiTreeModelPdm(QObject* parent);


    // TO BE DELETED, NOT USED
    virtual bool    insertRows_special(int position, int rows, const QModelIndex &parent = QModelIndex());

    // Special edit methods
    bool            deleteRangeFilter(const QModelIndex& itemIndex);
    bool            deletePropertyFilter(const QModelIndex& itemIndex);
    bool            deleteReservoirView(const QModelIndex& itemIndex);
    void            deleteInputProperty(const QModelIndex& itemIndex);
    void            deleteReservoir(RimReservoir* reservoir);

    RimCellPropertyFilter*  addPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimCellRangeFilter*     addRangeFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimReservoirView*       addReservoirView(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    void                    addInputProperty(const QModelIndex& itemIndex, const QStringList& fileNames);
    void                    addObjects(const QModelIndex& itemIndex, caf::PdmObjectGroup& pdmObjects);
    
    RimStatisticalCalculation*       addStatisticalCalculation(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);
    RimIdenticalGridCaseGroup*       addCaseGroup(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex);

    bool            deleteObjectFromPdmPointersField(const QModelIndex& itemIndex);

    void            updateScriptPaths();

    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData * mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;

private slots:
    void            slotRefreshScriptTree(QString path);
    void            clearClipboard();

private:
    QFileSystemWatcher* m_scriptChangeDetector;
};





