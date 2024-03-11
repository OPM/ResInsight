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

#include "cvfCollection.h"
#include "cvfObject.h"

#include "RiaDefines.h"
#include "RifReaderInterface.h"
#include "RigFault.h"

#include "ert/ecl/ecl_kw.h"

#include <map>

#include <QString>

class RigEclipseCaseData;
class QFile;
class QTextStream;

//--------------------------------------------------------------------------------------------------
/// Structure used to cache file position of keywords
//--------------------------------------------------------------------------------------------------
struct RifKeywordAndFilePos
{
    QString keyword;
    qint64  filePos;
};

//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifEclipseInputFileTools : public cvf::Object
{
public:
    RifEclipseInputFileTools();
    ~RifEclipseInputFileTools() override;

    static bool openGridFile( const QString& fileName, RigEclipseCaseData* eclipseCase, bool readFaultData, QString* errorMessages );

    static bool exportGrid( const QString&         gridFileName,
                            RigEclipseCaseData*    eclipseCase,
                            bool                   exportInLocalCoordinates,
                            const cvf::UByteArray* cellVisibilityOverrideForActnum = nullptr,
                            const cvf::Vec3st&     min                             = cvf::Vec3st::ZERO,
                            const cvf::Vec3st&     max                             = cvf::Vec3st::UNDEFINED,
                            const cvf::Vec3st&     refinement                      = cvf::Vec3st( 1, 1, 1 ) );

    static bool exportKeywords( const QString&              resultFileName,
                                RigEclipseCaseData*         eclipseCase,
                                const std::vector<QString>& keywords,
                                bool                        writeEchoKeywords,
                                const cvf::Vec3st&          min        = cvf::Vec3st::ZERO,
                                const cvf::Vec3st&          max        = cvf::Vec3st::UNDEFINED,
                                const cvf::Vec3st&          refinement = cvf::Vec3st( 1, 1, 1 ) );

    static void saveFault( QString                                 completeFilename,
                           const RigMainGrid*                      mainGrid,
                           const std::vector<RigFault::FaultFace>& faultFaces,
                           QString                                 faultName,
                           const cvf::Vec3st&                      min        = cvf::Vec3st::ZERO,
                           const cvf::Vec3st&                      max        = cvf::Vec3st::UNDEFINED,
                           const cvf::Vec3st&                      refinement = cvf::Vec3st( 1, 1, 1 ) );

    static void saveFault( QTextStream&                            stream,
                           const RigMainGrid*                      mainGrid,
                           const std::vector<RigFault::FaultFace>& faultFaces,
                           QString                                 faultName,
                           const cvf::Vec3st&                      min        = cvf::Vec3st::ZERO,
                           const cvf::Vec3st&                      max        = cvf::Vec3st::UNDEFINED,
                           const cvf::Vec3st&                      refinement = cvf::Vec3st( 1, 1, 1 ) );

    static void saveFaults( QTextStream&       stream,
                            const RigMainGrid* mainGrid,
                            const cvf::Vec3st& min        = cvf::Vec3st::ZERO,
                            const cvf::Vec3st& max        = cvf::Vec3st::UNDEFINED,
                            const cvf::Vec3st& refinement = cvf::Vec3st( 1, 1, 1 ) );

    static bool importFaultsFromFile( RigEclipseCaseData* eclipseCase, const QString& fileName );

    static void readFaultsInGridSection( const QString&             fileName,
                                         cvf::Collection<RigFault>* faults,
                                         std::vector<QString>*      filenamesWithFaults,
                                         const QString&             faultIncludeFileAbsolutePathPrefix );
    static void parseAndReadFaults( const QString& fileName, cvf::Collection<RigFault>* faults );

    static void parseAndReadPathAliasKeyword( const QString& fileName, std::vector<std::pair<QString, QString>>* pathAliasDefinitions );

    static QStringList readKeywordContentFromFile( const QString& keyword, const QString& keywordToStopParsing, QFile& file );

    // Public for unit testing, use readKeywordContentFromFile()
    static bool readKeywordAndParseIncludeStatementsRecursively( const QString&                                  keyword,
                                                                 const QString&                                  keywordToStopParsing,
                                                                 const std::vector<std::pair<QString, QString>>& pathAliasDefinitions,
                                                                 const QString&        includeStatementAbsolutePathPrefix,
                                                                 QFile&                file,
                                                                 qint64                startPos,
                                                                 QStringList&          keywordDataContent,
                                                                 std::vector<QString>& filenamesContainingKeyword,
                                                                 bool&                 isStopParsingKeywordDetected );

    static RiaDefines::EclipseUnitSystem readUnitSystem( QFile& file, qint64 gridunitPos );

    static cvf::StructGridInterface::FaceEnum faceEnumFromText( const QString& faceString );

    static bool hasGridData( const QString& fileName );

private:
    static void readFaults( QFile& data, qint64 filePos, cvf::Collection<RigFault>* faults, bool* isEditKeywordDetected );

    static void readFaults( const QString& fileName, const std::vector<RifKeywordAndFilePos>& fileKeywords, cvf::Collection<RigFault>* faults );
    static bool readFaultsAndParseIncludeStatementsRecursively( QFile&                                          file,
                                                                qint64                                          startPos,
                                                                const std::vector<std::pair<QString, QString>>& pathAliasDefinitions,
                                                                cvf::Collection<RigFault>*                      faults,
                                                                std::vector<QString>*                           filenamesWithFaults,
                                                                bool*                                           isEditKeywordDetected,
                                                                const QString& faultIncludeFileAbsolutePathPrefix );

    static void readKeywordDataContent( QFile& data, qint64 filePos, QStringList& textContent, bool& isStopParsingKeywordDetected );

    static void findGridKeywordPositions( const std::vector<RifKeywordAndFilePos>& keywords,
                                          qint64*                                  coordPos,
                                          qint64*                                  zcornPos,
                                          qint64*                                  specgridPos,
                                          qint64*                                  actnumPos,
                                          qint64*                                  mapaxesPos,
                                          qint64*                                  gridunitPos );

    static size_t findFaultByName( const cvf::Collection<RigFault>& faults, const QString& name );

    static qint64 findKeyword( const QString& keyword, QFile& file, qint64 startPos );

    static void writeFaultLine( QTextStream&                       stream,
                                QString                            faultName,
                                size_t                             i,
                                size_t                             j,
                                size_t                             startK,
                                size_t                             endK,
                                cvf::StructGridInterface::FaceType faceType );

    static QString faultFaceText( cvf::StructGridInterface::FaceType faceType );
};
