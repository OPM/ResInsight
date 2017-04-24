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

#include "RifEclipseInputFileTools.h"

#include "RifReaderEclipseOutput.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "cafProgressInfo.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>

#include "ert/ecl/ecl_box.h"
#include "ert/ecl/ecl_kw.h"

QString includeKeyword("INCLUDE");
QString faultsKeyword("FAULTS");
QString editKeyword("EDIT");
QString gridKeyword("GRID");
QString pathsKeyword("PATHS");



//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseInputFileTools::RifEclipseInputFileTools()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseInputFileTools::~RifEclipseInputFileTools()
{
  
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::openGridFile(const QString& fileName, RigEclipseCaseData* eclipseCase, bool readFaultData)
{
    CVF_ASSERT(eclipseCase);

    std::vector< RifKeywordAndFilePos > keywordsAndFilePos;
    findKeywordsOnFile(fileName, &keywordsAndFilePos);

    qint64 coordPos = -1;
    qint64 zcornPos = -1;
    qint64 specgridPos = -1;
    qint64 actnumPos = -1;
    qint64 mapaxesPos = -1;

    findGridKeywordPositions(keywordsAndFilePos, &coordPos, &zcornPos, &specgridPos, &actnumPos, &mapaxesPos);

    if (coordPos < 0 || zcornPos < 0 || specgridPos < 0)
    {
        return false;
    }


    FILE* gridFilePointer = util_fopen(fileName.toLatin1().data(), "r");
    if (!gridFilePointer) return false;

    // Main grid dimensions
    // SPECGRID - This is whats normally available, but not really the input to Eclipse.
    // DIMENS - Is what Eclipse expects and uses, but is not defined in the GRID section and is not (?) available normally
    // ZCORN, COORD, ACTNUM, MAPAXES

    //ecl_kw_type  *  ecl_kw_fscanf_alloc_grdecl_dynamic__( FILE * stream , const char * kw , bool strict , ecl_type_enum ecl_type);
    //ecl_grid_type * ecl_grid_alloc_GRDECL_kw( int nx, int ny , int nz , const ecl_kw_type * zcorn_kw , const ecl_kw_type * coord_kw , const ecl_kw_type * actnum_kw , const ecl_kw_type * mapaxes_kw ); 



    ecl_kw_type* specGridKw  = NULL;
    ecl_kw_type* zCornKw     = NULL;
    ecl_kw_type* coordKw     = NULL;
    ecl_kw_type* actNumKw    = NULL;
    ecl_kw_type* mapAxesKw   = NULL;

    // Try to read all the needed keywords. Early exit if some are not found
    caf::ProgressInfo progress(8, "Read Grid from Eclipse Input file");



    bool allKwReadOk = true;

    fseek(gridFilePointer, specgridPos, SEEK_SET);
    allKwReadOk = allKwReadOk && NULL != (specGridKw = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ecl_type_create_from_type(ECL_INT_TYPE)));
    progress.setProgress(1);

    fseek(gridFilePointer, zcornPos, SEEK_SET);
    allKwReadOk = allKwReadOk && NULL != (zCornKw    = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ecl_type_create_from_type(ECL_FLOAT_TYPE)));
    progress.setProgress(2);

    fseek(gridFilePointer, coordPos, SEEK_SET);
    allKwReadOk = allKwReadOk && NULL != (coordKw    = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ecl_type_create_from_type(ECL_FLOAT_TYPE)));
    progress.setProgress(3);

    // If ACTNUM is not defined, this pointer will be NULL, which is a valid condition
    if (actnumPos >= 0)
    {
        fseek(gridFilePointer, actnumPos, SEEK_SET);
        allKwReadOk = allKwReadOk && NULL != (actNumKw   = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ecl_type_create_from_type(ECL_INT_TYPE)));
        progress.setProgress(4);
    }

    // If MAPAXES is not defined, this pointer will be NULL, which is a valid condition
    if (mapaxesPos >= 0)
    {
        fseek(gridFilePointer, mapaxesPos, SEEK_SET);
        mapAxesKw = ecl_kw_fscanf_alloc_current_grdecl__( gridFilePointer, false , ecl_type_create_from_type(ECL_FLOAT_TYPE));
    }

    if (!allKwReadOk)
    {
        if(specGridKw) ecl_kw_free(specGridKw);
        if(zCornKw) ecl_kw_free(zCornKw);
        if(coordKw) ecl_kw_free(coordKw);
        if(actNumKw) ecl_kw_free(actNumKw);
        if(mapAxesKw) ecl_kw_free(mapAxesKw);

        return false;
    }

    progress.setProgress(5);

    int nx = ecl_kw_iget_int(specGridKw, 0); 
    int ny = ecl_kw_iget_int(specGridKw, 1); 
    int nz = ecl_kw_iget_int(specGridKw, 2);

    ecl_grid_type* inputGrid = ecl_grid_alloc_GRDECL_kw( nx, ny, nz, zCornKw, coordKw, actNumKw, mapAxesKw ); 

    progress.setProgress(6);

    RifReaderEclipseOutput::transferGeometry(inputGrid, eclipseCase);

    progress.setProgress(7);
    progress.setProgressDescription("Read faults ...");

    if (readFaultData)
    {
        cvf::Collection<RigFault> faults;
        RifEclipseInputFileTools::readFaults(fileName, keywordsAndFilePos, &faults);

        RigMainGrid* mainGrid = eclipseCase->mainGrid();
        mainGrid->setFaults(faults);
    }
    
    progress.setProgress(8);
    progress.setProgressDescription("Cleaning up ...");

    ecl_kw_free(specGridKw);
    ecl_kw_free(zCornKw);
    ecl_kw_free(coordKw);
    if (actNumKw) ecl_kw_free(actNumKw);
    if (mapAxesKw) ecl_kw_free(mapAxesKw);

    ecl_grid_free(inputGrid);

    util_fclose(gridFilePointer);
    
    return true;
}


//--------------------------------------------------------------------------------------------------
/// Read known properties from the input file
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RifEclipseInputFileTools::readProperties(const QString& fileName, RigEclipseCaseData* caseData)
{
    CVF_ASSERT(caseData);

    caf::ProgressInfo mainProgress(2, "Reading Eclipse Input properties");

    std::vector<RifKeywordAndFilePos> fileKeywords;
    RifEclipseInputFileTools::findKeywordsOnFile(fileName, &fileKeywords);

    mainProgress.setProgress(1);
    caf::ProgressInfo progress(fileKeywords.size(), "Reading properties");

    FILE* gridFilePointer = util_fopen(fileName.toLatin1().data(), "r");

    if (!gridFilePointer || !fileKeywords.size() ) 
    {
        return std::map<QString, QString>();
    }

    std::map<QString, QString> newResults;
    for (size_t i = 0; i < fileKeywords.size(); ++i)
    {
        if (!isValidDataKeyword(fileKeywords[i].keyword)) continue;

        fseek(gridFilePointer, fileKeywords[i].filePos, SEEK_SET);

        ecl_kw_type* eclipseKeywordData = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false, ecl_type_create_from_type(ECL_FLOAT_TYPE));
        if (eclipseKeywordData)
        {
            QString newResultName = caseData->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(fileKeywords[i].keyword);
            if (readDataFromKeyword(eclipseKeywordData, caseData, newResultName))
            {
                newResults[newResultName] = fileKeywords[i].keyword;
            }

            ecl_kw_free(eclipseKeywordData);
        }

        progress.setProgress(i);
    }

    util_fclose(gridFilePointer);
    return newResults;
}


//--------------------------------------------------------------------------------------------------
/// Reads the property data requested into the \a reservoir, overwriting any previous 
/// properties with the same name.
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::readProperty(const QString& fileName, RigEclipseCaseData* caseData, const QString& eclipseKeyWord, const QString& resultName)
{
    CVF_ASSERT(caseData);

    if (!isValidDataKeyword(eclipseKeyWord)) return false;

    FILE* filePointer = util_fopen(fileName.toLatin1().data(), "r");
    if (!filePointer) return false;

    ecl_kw_type* eclipseKeywordData = ecl_kw_fscanf_alloc_grdecl_dynamic__(filePointer, eclipseKeyWord.toLatin1().data(), false, ecl_type_create_from_type(ECL_FLOAT_TYPE));
    bool isOk = false;
    if (eclipseKeywordData)
    {
        isOk = readDataFromKeyword(eclipseKeywordData, caseData, resultName);

        ecl_kw_free(eclipseKeywordData);
    }

    util_fclose(filePointer);

    return isOk;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::readDataFromKeyword(ecl_kw_type* eclipseKeywordData, RigEclipseCaseData* caseData, const QString& resultName)
{
    CVF_ASSERT(caseData);
    CVF_ASSERT(eclipseKeywordData);

    bool mathingItemCount = false;
    {
        size_t itemCount = static_cast<size_t>(ecl_kw_get_size(eclipseKeywordData));
        if (itemCount == caseData->mainGrid()->cellCount())
        {
            mathingItemCount = true;
        }
        if (itemCount == caseData->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->reservoirActiveCellCount())
        {
            mathingItemCount = true;
        }
    }

    if (!mathingItemCount) return false;

    size_t resultIndex = RifEclipseInputFileTools::findOrCreateResult(resultName, caseData);
    if (resultIndex == cvf::UNDEFINED_SIZE_T) return false;

    std::vector< std::vector<double> >& newPropertyData = caseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
    newPropertyData.push_back(std::vector<double>());
    newPropertyData[0].resize(ecl_kw_get_size(eclipseKeywordData), HUGE_VAL);
    ecl_kw_get_data_as_double(eclipseKeywordData, newPropertyData[0].data());

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Read all the keywords from a file
//
// This code was originally written using QTextStream, but due to a bug in Qt version up to 4.8.0
// we had to implement the reading using QFile and QFile::readLine
//
// See:
// https://bugreports.qt-project.org/browse/QTBUG-9814
//
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::findKeywordsOnFile(const QString &fileName, std::vector< RifKeywordAndFilePos >* keywords)
{
    char buf[1024];

    QFile data(fileName);
    data.open(QFile::ReadOnly);

    QString line;
    qint64 filepos = -1;
    qint64 lineLength = -1;

    do
    {
        lineLength = data.readLine(buf, sizeof(buf));
        if (lineLength > 0)
        {
            line = QString::fromAscii(buf);
            if (line.size() && line[0].isLetter())
            {
                RifKeywordAndFilePos keyPos;

                filepos = data.pos() - lineLength;
                keyPos.filePos = filepos;
                keyPos.keyword = line.trimmed();
                keywords->push_back(keyPos);
                //qDebug() << keyPos.keyword << " - " << keyPos.filePos;
            }
        }
    }
    while (lineLength != -1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::parseAndReadPathAliasKeyword(const QString &fileName, std::vector< std::pair<QString, QString> >* pathAliasDefinitions)
{
    char buf[1024];

    QFile data(fileName);
    data.open(QFile::ReadOnly);

    QString line;

    bool foundPathsKeyword = false;

    do
    {
        qint64 lineLength = data.readLine(buf, sizeof(buf));
        if (lineLength > 0)
        {
            line = QString::fromAscii(buf);
            if (line.size() && (line[0].isLetter() || foundPathsKeyword))
            {
                line = line.trimmed();
                
                if (line == gridKeyword)
                {
                    return;
                }
                else if (line == pathsKeyword)
                {
                    foundPathsKeyword = true;
                }
                else if (foundPathsKeyword)
                {
                    if (line.startsWith("/", Qt::CaseInsensitive))
                    {
                        // Detected end of keyword data section
                        return;
                    }
                    else if (line.startsWith("--", Qt::CaseInsensitive))
                    {
                        continue;
                    }
                    else
                    {
                        // Replace tab with space to be able to split the string using space as splitter
                        line.replace("\t", " ");

                        // Remove character ' used to mark start and end of fault name, possibly also around face definition; 'I+'
                        line.remove("'");

                        QStringList entries = line.split(" ", QString::SkipEmptyParts);
                        if (entries.size() < 2)
                        {
                            continue;
                        }

                        pathAliasDefinitions->push_back(std::make_pair(entries[0], entries[1]));
                    }
                }
            }
        }
    }  while (!data.atEnd());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<QString>& RifEclipseInputFileTools::invalidPropertyDataKeywords()
{
    static std::vector<QString> keywords;
    static bool isInitialized = false;
    if (!isInitialized)
    {
        // Related to geometry
        keywords.push_back("COORD");
        keywords.push_back("ZCORN");
        keywords.push_back("SPECGRID");
        keywords.push_back("MAPAXES");

        keywords.push_back(faultsKeyword);

        isInitialized = true;
    }

    return keywords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::writePropertyToTextFile(const QString& fileName, RigEclipseCaseData* eclipseCase, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord)
{
    CVF_ASSERT(eclipseCase);

    size_t resultIndex = eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(resultName);
    if (resultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    std::vector< std::vector<double> >& resultData = eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
    if (resultData.size() == 0)
    {
        return false;
    }

    std::vector<double>& singleTimeStepData = resultData[timeStep];
    writeDataToTextFile(&file, eclipseKeyWord, singleTimeStepData);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Create and write a result vector with values for all cells.
/// undefinedValue is used for cells with no result
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::writeBinaryResultToTextFile(const QString& fileName, 
                                                           RigEclipseCaseData* eclipseCase, 
                                                           RifReaderInterface::PorosityModelResultType porosityModel, 
                                                           size_t timeStep, 
                                                           const QString& resultName, 
                                                           const QString& eclipseKeyWord, 
                                                           const double undefinedValue)
{
    CVF_ASSERT(eclipseCase);

    size_t resultIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(resultName);
    if (resultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromUiResultName(eclipseCase, eclipseCase->mainGrid()->gridIndex(), porosityModel, timeStep, resultName);
    if (resultAccessor.isNull())
    {
        return false;
    }

    std::vector<double> resultData;
    size_t i, j, k;
    for (k = 0; k < eclipseCase->mainGrid()->cellCountK(); k++)
    {
        for (j = 0; j < eclipseCase->mainGrid()->cellCountJ(); j++)
        {
            for (i = 0; i < eclipseCase->mainGrid()->cellCountI(); i++)
            {
                double resultValue = resultAccessor->cellScalar(eclipseCase->mainGrid()->cellIndexFromIJK(i, j, k));
                if (resultValue == HUGE_VAL)
                {
                    resultValue = undefinedValue;
                }

                resultData.push_back(resultValue);
            }
        }
    }

    writeDataToTextFile(&file, eclipseKeyWord, resultData);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::writeDataToTextFile(QFile* file, const QString& eclipseKeyWord, const std::vector<double>& resultData)
{
    QTextStream out(file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n";
    out << eclipseKeyWord << "\n" << right << qSetFieldWidth(16);

    caf::ProgressInfo pi(resultData.size(), QString("Writing data to file %1").arg(file->fileName()) );
    size_t progressSteps = resultData.size() / 20;

    size_t i;
    for (i = 0; i < resultData.size(); i++)
    {
        out << resultData[i];

        if ( (i + 1) % 5 == 0)
        {
            out << "\n";
        }

        if (i % progressSteps == 0)
        {
            pi.setProgress(i);
        }
    }

    out << "\n" << "/" << "\n";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::findGridKeywordPositions(const std::vector< RifKeywordAndFilePos >& keywordsAndFilePos, qint64* coordPos, qint64* zcornPos, qint64* specgridPos, qint64* actnumPos, qint64* mapaxesPos)
{
    CVF_ASSERT(coordPos && zcornPos && specgridPos && actnumPos && mapaxesPos);

    size_t i;
    for (i = 0; i < keywordsAndFilePos.size(); i++)
    {
        if (keywordsAndFilePos[i].keyword == "COORD")
        {
            *coordPos = keywordsAndFilePos[i].filePos;
        }
        else if (keywordsAndFilePos[i].keyword == "ZCORN")
        {
            *zcornPos = keywordsAndFilePos[i].filePos;
        }
        else if (keywordsAndFilePos[i].keyword == "SPECGRID")
        {
            *specgridPos = keywordsAndFilePos[i].filePos;
        }
        else if (keywordsAndFilePos[i].keyword == "ACTNUM")
        {
            *actnumPos = keywordsAndFilePos[i].filePos;
        }
        else if (keywordsAndFilePos[i].keyword == "MAPAXES")
        {
            *mapaxesPos = keywordsAndFilePos[i].filePos;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readFaults(const QString& fileName, const std::vector<RifKeywordAndFilePos>& fileKeywords, cvf::Collection<RigFault>* faults)
{
    QFile data(fileName);
    if (!data.open(QFile::ReadOnly))
    {
        return;
    }

    for (size_t i = 0; i < fileKeywords.size(); i++)
    {
        if (fileKeywords[i].keyword.compare(editKeyword, Qt::CaseInsensitive) == 0)
        {
            return;
        }
        else if (fileKeywords[i].keyword.compare(faultsKeyword, Qt::CaseInsensitive) != 0)
        {
            continue;
        }

        qint64 filePos = fileKeywords[i].filePos;

        bool isEditKeywordDetected = false;
        readFaults(data, filePos, faults, &isEditKeywordDetected);

        if (isEditKeywordDetected)
        {
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::parseAndReadFaults(const QString& fileName, cvf::Collection<RigFault>* faults)
{
    QFile data(fileName);
    if (!data.open(QFile::ReadOnly))
    {
        return;
    }

    qint64 filePos = findKeyword(faultsKeyword, data, 0);

    while (filePos != -1)
    {
        readFaults(data, filePos, faults, NULL);
        filePos = findKeyword(faultsKeyword, data, filePos);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readFaultsInGridSection(const QString& fileName, cvf::Collection<RigFault>* faults, std::vector<QString>* filenamesWithFaults, const QString& faultIncludeFileAbsolutePathPrefix)
{
    QFile data(fileName);
    if (!data.open(QFile::ReadOnly))
    {
        return;
    }

    // Search for keyword grid
    qint64 gridPos = findKeyword(gridKeyword, data, 0);
    if (gridPos < 0)
    {
        return;
    }

    bool isEditKeywordDetected = false;

    std::vector< std::pair<QString, QString> > pathAliasDefinitions;
    parseAndReadPathAliasKeyword(fileName, &pathAliasDefinitions);

    readFaultsAndParseIncludeStatementsRecursively(data, gridPos, pathAliasDefinitions, faults, filenamesWithFaults, &isEditKeywordDetected, faultIncludeFileAbsolutePathPrefix);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifEclipseInputFileTools::findFaultByName(const cvf::Collection<RigFault>& faults, const QString& name)
{
    for (size_t i = 0; i < faults.size(); i++)
    {
        if (faults.at(i)->name() == name)
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
qint64 RifEclipseInputFileTools::findKeyword(const QString& keyword, QFile& file, qint64 startPos)
{
    QString line;

    file.seek(startPos);

    do 
    {
        line = file.readLine();
        line = line.trimmed();

        if (line.startsWith("--", Qt::CaseInsensitive))
        {
            continue;
        }

        if (line.startsWith(keyword, Qt::CaseInsensitive))
        {
            return file.pos();
        }

    } while (!file.atEnd());


    return -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifEclipseInputFileTools::findOrCreateResult(const QString& newResultName, RigEclipseCaseData* reservoir)
{
    size_t resultIndex = reservoir->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(newResultName);
    if (resultIndex == cvf::UNDEFINED_SIZE_T)
    {
        resultIndex = reservoir->results(RifReaderInterface::MATRIX_RESULTS)->addEmptyScalarResult(RimDefines::INPUT_PROPERTY, newResultName, false);
    }

    return resultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::isValidDataKeyword(const QString& keyword)
{
    const std::vector<QString>& keywordsToSkip = RifEclipseInputFileTools::invalidPropertyDataKeywords();
    for (const QString keywordToSkip : keywordsToSkip)
    {
        if (keywordToSkip == keyword.toUpper())
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::readFaultsAndParseIncludeStatementsRecursively(  QFile& file, 
                                                                                qint64 startPos, 
                                                                                const std::vector< std::pair<QString, QString> >& pathAliasDefinitions, 
                                                                                cvf::Collection<RigFault>* faults, 
                                                                                std::vector<QString>* filenamesWithFaults, 
                                                                                bool* isEditKeywordDetected,
                                                                                const QString& faultIncludeFileAbsolutePathPrefix)
{
    QString line;

    if (!file.seek(startPos))
    {
        return false;
    }

    bool continueParsing = true;

    do 
    {
        line = file.readLine();
        line = line.trimmed();

        if (line.startsWith("--", Qt::CaseInsensitive))
        {
            continue;
        }
        else if (line.startsWith(editKeyword, Qt::CaseInsensitive))
        {
            if (isEditKeywordDetected)
            {
                *isEditKeywordDetected = true;
            }

            return false;
        }

        if (line.startsWith(includeKeyword, Qt::CaseInsensitive))
        {
            line = file.readLine();
            line = line.trimmed();

            while (line.startsWith("--", Qt::CaseInsensitive))
            {
                line = file.readLine();
                line = line.trimmed();
            }

            int firstQuote = line.indexOf("'");
            int lastQuote = line.lastIndexOf("'");

            if (!(firstQuote < 0 || lastQuote < 0 || firstQuote == lastQuote))
            {
                QDir currentFileFolder;
                {
                    QFileInfo fi(file.fileName());
                    currentFileFolder = fi.absoluteDir();
                }
                
                // Read include file name, and both relative and absolute path is supported
                QString includeFilename = line.mid(firstQuote + 1, lastQuote - firstQuote - 1);

                for (auto entry : pathAliasDefinitions)
                {
                    QString textToReplace = "$" + entry.first;
                    includeFilename.replace(textToReplace, entry.second);
                }

#ifdef WIN32
                if (includeFilename.startsWith('/'))
                {
                    // Absolute UNIX path, prefix on Windows
                    includeFilename = faultIncludeFileAbsolutePathPrefix + includeFilename;
                }
#endif

                QFileInfo fi(currentFileFolder, includeFilename);
                if (fi.exists())
                {
                    QString absoluteFilename = fi.canonicalFilePath();
                    QFile includeFile(absoluteFilename);
                    if (includeFile.open(QFile::ReadOnly))
                    {
                        //qDebug() << "Found include statement, and start parsing of\n  " << absoluteFilename;

                        if (!readFaultsAndParseIncludeStatementsRecursively(includeFile, 0, pathAliasDefinitions, faults, filenamesWithFaults, isEditKeywordDetected, faultIncludeFileAbsolutePathPrefix))
                        {
                            qDebug() << "Error when parsing include file : " << absoluteFilename;
                        }
                    }
                }
            }
        }
        else if (line.startsWith(faultsKeyword, Qt::CaseInsensitive))
        {
            if (!line.contains("/"))
            {
                readFaults(file, file.pos(), faults, isEditKeywordDetected);
                filenamesWithFaults->push_back(file.fileName());
            }
        }

        if (isEditKeywordDetected && *isEditKeywordDetected)
        {
            continueParsing = false;
        }

        if (file.atEnd())
        {
            continueParsing = false;
        }

    } while (continueParsing);
    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::StructGridInterface::FaceEnum RifEclipseInputFileTools::faceEnumFromText(const QString& faceString)
{
    QString upperCaseText = faceString.toUpper().trimmed();

    if (upperCaseText == "X" || upperCaseText == "X+" || upperCaseText == "I" || upperCaseText == "I+") return cvf::StructGridInterface::POS_I;
    if (upperCaseText == "Y" || upperCaseText == "Y+" || upperCaseText == "J" || upperCaseText == "J+") return cvf::StructGridInterface::POS_J;
    if (upperCaseText == "Z" || upperCaseText == "Z+" || upperCaseText == "K" || upperCaseText == "K+") return cvf::StructGridInterface::POS_K;
    
    if (upperCaseText == "X-" || upperCaseText == "I-") return cvf::StructGridInterface::NEG_I;
    if (upperCaseText == "Y-" || upperCaseText == "J-") return cvf::StructGridInterface::NEG_J;
    if (upperCaseText == "Z-" || upperCaseText == "K-") return cvf::StructGridInterface::NEG_K;

    return cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
/// The file pointer is pointing at the line following the FAULTS keyword.
/// Parse content of this keyword until end of file or
/// end of keyword when a single line with '/' is found
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readFaults(QFile &data, qint64 filePos, cvf::Collection<RigFault>* faults, bool* isEditKeywordDetected)
{
    if (!data.seek(filePos))
    {
        return;
    }

    // qDebug() << "Reading faults from\n  " << data.fileName();

    RigFault* fault = NULL;

    do 
    {
        QString line = data.readLine();
        line = line.trimmed();

        if (line.startsWith("--", Qt::CaseInsensitive))
        {
            // Skip comment lines
            continue;
        }
        else if (line.startsWith("/", Qt::CaseInsensitive))
        {
            // Detected end of keyword data section
            return;
        }
        else if (line.startsWith(editKeyword, Qt::CaseInsensitive))
        {
            // End parsing when edit keyword is detected

            if (isEditKeywordDetected)
            {
                *isEditKeywordDetected = true;
            }

            return;
        }

        // Replace tab with space to be able to split the string using space as splitter
        line.replace("\t", " ");

        // Remove character ' used to mark start and end of fault name, possibly also around face definition; 'I+'
        line.remove("'");

        QStringList entries = line.split(" ", QString::SkipEmptyParts);
        if (entries.size() < 8)
        {
            continue;
        }

        QString name = entries[0];

        int i1, i2, j1, j2, k1, k2;
        i1 = entries[1].toInt();
        i2 = entries[2].toInt();
        j1 = entries[3].toInt();
        j2 = entries[4].toInt();
        k1 = entries[5].toInt();
        k2 = entries[6].toInt();

        QString faceString = entries[7];

        cvf::StructGridInterface::FaceEnum cellFaceEnum = RifEclipseInputFileTools::faceEnumFromText(faceString);

        // Adjust from 1-based to 0-based cell indices
        // Guard against invalid cell ranges by limiting lowest possible range value to zero
        cvf::CellRange cellrange(CVF_MAX(i1 - 1, 0), CVF_MAX(j1 - 1, 0), CVF_MAX(k1 - 1, 0), CVF_MAX(i2 - 1, 0), CVF_MAX(j2 - 1, 0), CVF_MAX(k2 - 1, 0));

        if (!(fault && fault->name() == name))
        {
            if (findFaultByName(*faults, name) == cvf::UNDEFINED_SIZE_T)
            {
                RigFault* newFault = new RigFault;
                newFault->setName(name);

                faults->push_back(newFault);
            }

            size_t faultIndex = findFaultByName(*faults, name);
            if (faultIndex == cvf::UNDEFINED_SIZE_T)
            {
                CVF_ASSERT(faultIndex != cvf::UNDEFINED_SIZE_T);
                continue;
            }

            fault = faults->at(faultIndex);
        }

        CVF_ASSERT(fault);

        fault->addCellRangeForFace(cellFaceEnum, cellrange);

    } while (!data.atEnd());
}
