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

#include <QMimeData>
#include <QModelIndex>
#include <QStringList>

#include <vector>


//--------------------------------------------------------------------------------------------------
/// MimeData class used to carry a QModelIndexList
//--------------------------------------------------------------------------------------------------
class MimeDataWithIndexes : public QMimeData
{
    Q_OBJECT

public:
    MimeDataWithIndexes();
    MimeDataWithIndexes(const MimeDataWithIndexes & other);

    void                    setIndexes(const QModelIndexList& indexes);
    const QModelIndexList&  indexes() const;
    bool            hasFormat(const QString& mimetype) const override;
    QStringList     formats() const override;
    static QString          formatName();

private:
    QModelIndexList m_indexes;
};

Q_DECLARE_METATYPE(MimeDataWithIndexes)


//--------------------------------------------------------------------------------------------------
/// MimeData class used to carry string references to pdm objects
//--------------------------------------------------------------------------------------------------
class MimeDataWithReferences : public QMimeData
{
    Q_OBJECT

public:
    MimeDataWithReferences();
    MimeDataWithReferences(const MimeDataWithReferences& other);

    void                        setReferences(const std::vector<QString>& references);
    const std::vector<QString>& references() const;
    bool                hasFormat(const QString& mimetype) const override;
    QStringList         formats() const override;
    static QString              formatName();

private:
    std::vector<QString> m_references;
};

Q_DECLARE_METATYPE(MimeDataWithReferences)
