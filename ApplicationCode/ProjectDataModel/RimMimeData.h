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
    virtual bool            hasFormat(const QString& mimetype) const;
    virtual QStringList     formats() const;
    static QString          formatName();

private:
    QModelIndexList m_indexes;
};

Q_DECLARE_METATYPE(MimeDataWithIndexes)
