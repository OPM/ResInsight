/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include <map>

class QString;
class QStringList;

class RimIdenticalGridCaseGroup;

//==================================================================================================
///
//==================================================================================================
class RiaImportEclipseCaseTools
{
public:
    typedef std::map<QString, int> FileCaseIdMap;

    static bool openEclipseCasesFromFile( const QStringList& fileNames,
                                          bool               createView,
                                          FileCaseIdMap*     openedFilesOut = nullptr,
                                          bool               noDialog       = false );

    static bool openEclipseCaseShowTimeStepFilter( const QString& fileName );

    static int openEclipseInputCaseFromFileNames( const QStringList& fileNames, QString* fileContainingGrid = nullptr );
    static bool openMockModel( const QString& name );

    static bool addEclipseCases( const QStringList& fileNames, RimIdenticalGridCaseGroup** resultingCaseGroup = nullptr );

    static int openEclipseCaseFromFile( const QString& fileName, bool createView );

private:
    static int openEclipseCaseShowTimeStepFilterImpl( const QString& fileName, bool showTimeStepFilter, bool createView );
};
