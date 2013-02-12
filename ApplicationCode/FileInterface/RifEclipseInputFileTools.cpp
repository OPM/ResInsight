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

#include "RifEclipseInputFileTools.h"
#include "RifReaderEclipseOutput.h"
#include "RigReservoirCellResults.h"

#include "RigReservoir.h"
#include "cafProgressInfo.h"

#include <vector>
#include <cmath>
#include <iostream>

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "ecl_grid.h"
#include "well_state.h"
#include "util.h"
#include <fstream>
#include "RigGridScalarDataAccess.h"




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
bool RifEclipseInputFileTools::openGridFile(const QString& fileName, RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    qint64 coordPos = -1;
    qint64 zcornPos = -1;
    qint64 specgridPos = -1;
    qint64 actnumPos = -1;
    qint64 mapaxesPos = -1;

    findGridKeywordPositions(fileName, &coordPos, &zcornPos, &specgridPos, &actnumPos, &mapaxesPos);

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
    caf::ProgressInfo progress(7, "Read Grid from Eclipse Input file");



    bool allKwReadOk = true;
    bool continueReading = true;

    fseek(gridFilePointer, specgridPos, SEEK_SET);
    allKwReadOk = allKwReadOk && NULL != (specGridKw = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ECL_INT_TYPE));
    progress.setProgress(1);

    fseek(gridFilePointer, zcornPos, SEEK_SET);
    allKwReadOk = allKwReadOk && NULL != (zCornKw    = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ECL_FLOAT_TYPE));
    progress.setProgress(2);

    fseek(gridFilePointer, coordPos, SEEK_SET);
    allKwReadOk = allKwReadOk && NULL != (coordKw    = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ECL_FLOAT_TYPE));
    progress.setProgress(3);

    // If ACTNUM is not defined, this pointer will be NULL, which is a valid condition
    if (actnumPos >= 0)
    {
        fseek(gridFilePointer, actnumPos, SEEK_SET);
        allKwReadOk = allKwReadOk && NULL != (actNumKw   = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer, false , ECL_INT_TYPE));
        progress.setProgress(4);
    }

    // If MAPAXES is not defined, this pointer will be NULL, which is a valid condition
    if (mapaxesPos >= 0)
    {
        fseek(gridFilePointer, mapaxesPos, SEEK_SET);
        mapAxesKw = ecl_kw_fscanf_alloc_current_grdecl__( gridFilePointer, false , ECL_FLOAT_TYPE);
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

    RifReaderEclipseOutput::transferGeometry(inputGrid, reservoir);

    progress.setProgress(7);
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
std::map<QString, QString>  RifEclipseInputFileTools::readProperties(const QString &fileName, RigReservoir *reservoir)
{
    CVF_ASSERT(reservoir);

    std::set<QString> knownKeywordSet;
    {
        const std::vector<QString>& knownKeywords = RifEclipseInputFileTools::knownPropertyKeywords();
        for( size_t fkIt = 0; fkIt < knownKeywords.size(); ++fkIt) knownKeywordSet.insert(knownKeywords[fkIt]);
    }

    caf::ProgressInfo mainProgress(2, "Reading Eclipse Input properties");
    caf::ProgressInfo startProgress(knownKeywordSet.size(), "Scanning for known properties");

    std::vector<RifKeywordAndFilePos> fileKeywords = RifEclipseInputFileTools::findKeywordsOnFile(fileName);

    mainProgress.setProgress(1);
    caf::ProgressInfo progress(fileKeywords.size(), "Reading properties");

    FILE* gridFilePointer = util_fopen(fileName.toLatin1().data(), "r");

    if (!gridFilePointer || !fileKeywords.size() ) 
    {
        return std::map<QString, QString>();
    }

    bool isSomethingRead = false;
    std::map<QString, QString> newResults;
    for (size_t i = 0; i < fileKeywords.size(); ++i)
    {
        //std::cout << fileKeywords[i].keyword.toLatin1().data() << std::endl;
        if (knownKeywordSet.count(fileKeywords[i].keyword))
        {
            fseek(gridFilePointer, fileKeywords[i].filePos, SEEK_SET);
            ecl_kw_type* eclKeyWordData = ecl_kw_fscanf_alloc_current_grdecl__(gridFilePointer,  false , ECL_FLOAT_TYPE);
            if (eclKeyWordData)
            {
                QString newResultName = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(fileKeywords[i].keyword);

                size_t resultIndex = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->addEmptyScalarResult(RimDefines::INPUT_PROPERTY, newResultName); // Should really merge with inputProperty object information because we need to use PropertyName, and not keyword

                std::vector< std::vector<double> >& newPropertyData = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
                newPropertyData.push_back(std::vector<double>());
                newPropertyData[0].resize(ecl_kw_get_size(eclKeyWordData), HUGE_VAL);
                ecl_kw_get_data_as_double(eclKeyWordData, newPropertyData[0].data());

                ecl_kw_free(eclKeyWordData);
                newResults[newResultName] = fileKeywords[i].keyword;
            }
        }
        progress.setProgress(i);
    }

    util_fclose(gridFilePointer);
    return newResults;
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
std::vector< RifKeywordAndFilePos > RifEclipseInputFileTools::findKeywordsOnFile(const QString &fileName)
{
    std::vector< RifKeywordAndFilePos > keywords;

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
                keyPos.keyword = line.left(8).trimmed();
                keywords.push_back(keyPos);
                //qDebug() << keyPos.keyword << " - " << keyPos.filePos;
            }
        }
    }
    while (lineLength != -1);

    return keywords;
}

//--------------------------------------------------------------------------------------------------
/// Reads the property data requested into the \a reservoir, overwriting any previous 
/// propeties with the same name.
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::readProperty(const QString& fileName, RigReservoir* reservoir, const QString& eclipseKeyWord, const QString& resultName)
{
    CVF_ASSERT(reservoir);

    FILE* filePointer = util_fopen(fileName.toLatin1().data(), "r");
    if (!filePointer) return false;

    ecl_kw_type* eclKeyWordData = ecl_kw_fscanf_alloc_grdecl_dynamic__( filePointer , eclipseKeyWord.toLatin1().data() , false , ECL_FLOAT_TYPE);
    bool isOk = false;
    if (eclKeyWordData)
    {
        QString newResultName = resultName;
        size_t resultIndex = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(newResultName);
        if (resultIndex == cvf::UNDEFINED_SIZE_T)
        {
            resultIndex = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->addEmptyScalarResult(RimDefines::INPUT_PROPERTY, newResultName); 
        }

        std::vector< std::vector<double> >& newPropertyData = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
        newPropertyData.resize(1);
        newPropertyData[0].resize(ecl_kw_get_size(eclKeyWordData), HUGE_VAL);
        ecl_kw_get_data_as_double(eclKeyWordData, newPropertyData[0].data());
        isOk = true;
        ecl_kw_free(eclKeyWordData);
    }

    util_fclose(filePointer);
    return isOk;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<QString>& RifEclipseInputFileTools::knownPropertyKeywords()
{
    static std::vector<QString> knownKeywords;
    static bool isInitialized = false;
    if (!isInitialized)
    {
        knownKeywords.push_back("AQUIFERA");
        knownKeywords.push_back("ACTNUM");
        knownKeywords.push_back("EQLNUM");
        knownKeywords.push_back("FIPNUM");
        knownKeywords.push_back("KRG");
        knownKeywords.push_back("KRGR");
        knownKeywords.push_back("KRO");
        knownKeywords.push_back("KRORG");
        knownKeywords.push_back("KRORW");
        knownKeywords.push_back("KRW");
        knownKeywords.push_back("KRWR");
        knownKeywords.push_back("MINPVV");
        knownKeywords.push_back("MULTPV");
        knownKeywords.push_back("MULTX");
        knownKeywords.push_back("MULTX-");
        knownKeywords.push_back("MULTY");
        knownKeywords.push_back("MULTY-");
        knownKeywords.push_back("MULTZ");
        knownKeywords.push_back("NTG");
        knownKeywords.push_back("PCG");
        knownKeywords.push_back("PCW");
        knownKeywords.push_back("PERMX");
        knownKeywords.push_back("PERMY");
        knownKeywords.push_back("PERMZ");
        knownKeywords.push_back("PORO");
        knownKeywords.push_back("PVTNUM");
        knownKeywords.push_back("SATNUM");
        knownKeywords.push_back("SGCR");
        knownKeywords.push_back("SGL");
        knownKeywords.push_back("SGLPC");
        knownKeywords.push_back("SGU");
        knownKeywords.push_back("SGWCR");
        knownKeywords.push_back("SWATINIT");
        knownKeywords.push_back("SWCR");
        knownKeywords.push_back("SWGCR");
        knownKeywords.push_back("SWL");
        knownKeywords.push_back("SWLPC");
        knownKeywords.push_back("TRANX");
        knownKeywords.push_back("TRANY");
        knownKeywords.push_back("TRANZ");

        isInitialized = true;
    }
    return knownKeywords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::writePropertyToTextFile(const QString& fileName, RigReservoir* reservoir, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord)
{
    CVF_ASSERT(reservoir);

    size_t resultIndex = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(resultName);
    if (resultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    std::vector< std::vector<double> >& resultData = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
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
bool RifEclipseInputFileTools::writeBinaryResultToTextFile(const QString& fileName, RigReservoir* reservoir, RifReaderInterface::PorosityModelResultType porosityModel, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord, const double undefinedValue)
{
    CVF_ASSERT(reservoir);

    size_t resultIndex = reservoir->mainGrid()->results(porosityModel)->findScalarResultIndex(resultName);
    if (resultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    RigActiveCellInfo* activeCellInfo = reservoir->activeCellInfo();
    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = reservoir->mainGrid()->dataAccessObject(activeCellInfo, porosityModel, timeStep, resultIndex);
    if (dataAccessObject.isNull())
    {
        return false;
    }

    std::vector<double> resultData;
    size_t i, j, k;
    for (k = 0; k < reservoir->mainGrid()->cellCountK(); k++)
    {
        for (j = 0; j < reservoir->mainGrid()->cellCountJ(); j++)
        {
            for (i = 0; i < reservoir->mainGrid()->cellCountI(); i++)
            {
                double resultValue = dataAccessObject->cellScalar(i, j, k);
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
void RifEclipseInputFileTools::findGridKeywordPositions(const QString& filename, qint64* coordPos, qint64* zcornPos, qint64* specgridPos, qint64* actnumPos, qint64* mapaxesPos)
{
    CVF_ASSERT(coordPos && zcornPos && specgridPos && actnumPos && mapaxesPos);


    std::vector< RifKeywordAndFilePos > keywordsAndFilePos = findKeywordsOnFile(filename);
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
bool RifEclipseInputFileTools::readPropertyAtFilePosition(const QString& fileName, RigReservoir* reservoir, const QString& eclipseKeyWord, qint64 filePos, const QString& resultName)
{
    CVF_ASSERT(reservoir);

    FILE* filePointer = util_fopen(fileName.toLatin1().data(), "r");
    if (!filePointer) return false;

    fseek(filePointer, filePos, SEEK_SET);
    ecl_kw_type* eclKeyWordData = ecl_kw_fscanf_alloc_current_grdecl__(filePointer, false , ECL_FLOAT_TYPE);
    bool isOk = false;
    if (eclKeyWordData)
    {
        QString newResultName = resultName;
        size_t resultIndex = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(newResultName);
        if (resultIndex == cvf::UNDEFINED_SIZE_T)
        {
            resultIndex = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->addEmptyScalarResult(RimDefines::INPUT_PROPERTY, newResultName); 
        }

        std::vector< std::vector<double> >& newPropertyData = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
        newPropertyData.resize(1);
        newPropertyData[0].resize(ecl_kw_get_size(eclKeyWordData), HUGE_VAL);
        ecl_kw_get_data_as_double(eclKeyWordData, newPropertyData[0].data());
        isOk = true;
        ecl_kw_free(eclKeyWordData);
    }

    util_fclose(filePointer);
    return isOk;
}
