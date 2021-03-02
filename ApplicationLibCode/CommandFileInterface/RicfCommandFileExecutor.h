/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafAppEnum.h"
#include "cafPdmScriptIOMessages.h"

#include <map>
#include <vector>

class RicfCommandObject;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCommandFileExecutor
{
public:
    enum class ExportType
    {
        COMPLETIONS,
        SNAPSHOTS,
        PROPERTIES,
        STATISTICS,
        WELLPATHS,
        CELLS,
        LGRS
    };

    typedef caf::AppEnum<ExportType> ExportTypeEnum;

public:
    RicfCommandFileExecutor();
    ~RicfCommandFileExecutor();

    void    executeCommands( QTextStream& stream );
    void    setExportPath( ExportType type, const QString& path );
    QString getExportPath( ExportType type ) const;
    void    setLastProjectPath( const QString& path );
    QString getLastProjectPath() const;

    void setExportDataSouceAsComment( bool enable );
    bool exportDataSouceAsComment() const;

    static RicfCommandFileExecutor* instance();

    static std::vector<RicfCommandObject*>
        prepareFileCommandsForExecution( const std::vector<RicfCommandObject*>& commandsReadFromFile );

private:
    void clearCachedData();

    std::map<ExportType, QString> m_exportPaths;
    QString                       m_lastProjectPath;
    bool                          m_exportDataSourceAsComment;
};
