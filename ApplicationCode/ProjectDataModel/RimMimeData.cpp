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

#include "RiaStdInclude.h"

#include "RimMimeData.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MimeDataWithIndexes::MimeDataWithIndexes()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MimeDataWithIndexes::MimeDataWithIndexes(const MimeDataWithIndexes & other) : QMimeData()
{
    setIndexes(other.indexes());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MimeDataWithIndexes::setIndexes(const QModelIndexList & indexes)
{
    m_indexes = indexes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QModelIndexList& MimeDataWithIndexes::indexes() const
{
    return m_indexes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MimeDataWithIndexes::hasFormat(const QString &mimetype) const
{
    return (mimetype == formatName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList MimeDataWithIndexes::formats() const
{
    QStringList supportedFormats = QMimeData::formats();
    supportedFormats << formatName();

    return supportedFormats;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString MimeDataWithIndexes::formatName()
{
    return "MimeDataWithIndexes";
}

