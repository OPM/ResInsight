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

#include <QStringList>

#include <map>
#include <memory>
#include <utility>
#include <vector>

class QString;

class RimIdenticalGridCaseGroup;
class RimRoffCase;
class RifReaderSettings;

//==================================================================================================
///
//==================================================================================================
class RiaImportEclipseCaseTools
{
public:
    using CaseFileNameAndId = std::pair<QString, int>;
    using FileCaseIdMap     = std::map<QString, int>;

    static bool openEclipseCasesFromFile( const QStringList&                 fileNames,
                                          bool                               createView,
                                          FileCaseIdMap*                     openedFilesOut,
                                          bool                               noDialog,
                                          std::shared_ptr<RifReaderSettings> readerSettings = nullptr );

    static bool openEclipseCaseShowTimeStepFilter( const QString& fileName );

    static CaseFileNameAndId openEclipseInputCaseAndPropertiesFromFileNames( const QStringList& fileNames, bool createDefaultView );
    static std::vector<int>  openEclipseInputCasesFromFileNames( const QStringList& fileNames, bool createDefaultView );

    static bool openMockModel( const QString& name );

    static bool addEclipseCases( const QStringList& fileNames, RimIdenticalGridCaseGroup** resultingCaseGroup = nullptr );

    static int openEclipseCaseFromFile( const QString& fileName, bool createView, std::shared_ptr<RifReaderSettings> readerSettings = nullptr );

    static std::vector<int> openRoffCasesFromFileNames( const QStringList& fileNames, bool createDefaultView );
    static RimRoffCase*     openRoffCaseFromFileName( const QString& fileName, bool createDefaultView );

    static bool openEmFilesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );

private:
    static int openEclipseCaseShowTimeStepFilterImpl( const QString&                     fileName,
                                                      bool                               showTimeStepFilter,
                                                      bool                               createView,
                                                      std::shared_ptr<RifReaderSettings> readerSettings );
};
