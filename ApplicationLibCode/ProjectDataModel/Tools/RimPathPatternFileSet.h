/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimPlotWindow;
class RimSummaryPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimPathPatternFileSet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPathPatternFileSet();

    void    setPathPattern( const QString& pathPattern );
    QString pathPattern() const;

    void    setRangeString( const QString& rangeString );
    QString rangeString() const;

    static std::pair<QString, QString> findPathPattern( const QStringList& filePaths, const QString& placeholderString );
    static QStringList createPathsFromPattern( const QString& basePath, const QString& numberRange, const QString& placeholderString );

private:
    caf::PdmField<QString> m_pathPattern;
    caf::PdmField<QString> m_rangeString;
};
