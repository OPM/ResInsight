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

#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifEclipseOutputFileTools.h"

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_nnc_geometry.h"
#include "ert/ecl/ecl_nnc_data.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::RifEclipseUnifiedRestartFileAccess()
    : RifEclipseRestartDataAccess()
{
    m_ecl_file = NULL;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::~RifEclipseUnifiedRestartFileAccess()
{
    if (m_ecl_file)
    {
        ecl_file_close(m_ecl_file);
    }
}

//--------------------------------------------------------------------------------------------------
/// Open file
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::open()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::openFile()
{
    if (!m_ecl_file)
    {
        m_ecl_file = ecl_file_open(m_filename.toAscii().data(), ECL_FILE_CLOSE_STREAM);
    }

    if (!m_ecl_file) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Close file
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::close()
{
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifEclipseUnifiedRestartFileAccess::timeStepCount()
{
    if (!openFile())
    {
        return 0;
    }
    std::vector<QDateTime> timeSteps;
    std::vector<double> daysSinceSimulationStart;

    this->timeSteps(&timeSteps, &daysSinceSimulationStart);

    return timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time steps
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::timeSteps(std::vector<QDateTime>* timeSteps, std::vector<double>* daysSinceSimulationStart)
{
    if (openFile())
    {
        RifEclipseOutputFileTools::timeSteps(m_ecl_file, timeSteps, daysSinceSimulationStart);
    }
}


//--------------------------------------------------------------------------------------------------
/// Get list of result names
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts)
{
    if (openFile())
    {
        std::vector< ecl_file_type* > filesUsedToFindAvailableKeywords;
        filesUsedToFindAvailableKeywords.push_back(m_ecl_file);

        RifEclipseOutputFileTools::findKeywordsAndItemCount(filesUsedToFindAvailableKeywords, resultNames, resultDataItemCounts);
    }
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::results(const QString& resultName, size_t timeStep, size_t gridCount, std::vector<double>* values)
{
    if (!openFile())
    {
        return false;
    }

    ecl_file_push_block(m_ecl_file);

    for (size_t i = 0; i < gridCount; i++)
    {
        ecl_file_select_block(m_ecl_file, INTEHEAD_KW, static_cast<int>(timeStep * gridCount + i));

        int namedKeywordCount = ecl_file_get_num_named_kw(m_ecl_file, resultName.toAscii().data());
        for (int iOcc = 0; iOcc < namedKeywordCount; iOcc++)
        {
            std::vector<double> partValues;
            RifEclipseOutputFileTools::keywordData(m_ecl_file, resultName, iOcc, &partValues);

            values->insert(values->end(), partValues.begin(), partValues.end());
        }
    }

    ecl_file_pop_block(m_ecl_file);

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::dynamicNNCResults(const ecl_grid_type* grid, size_t timeStep, std::vector<double>* waterFlux, std::vector<double>* oilFlux, std::vector<double>* gasFlux)
{
    if (timeStep > timeStepCount())
    {
        return false;
    }

    if (!openFile())
    {
        return false;
    }

    ecl_file_view_type* summaryView = ecl_file_get_restart_view(m_ecl_file, static_cast<int>(timeStep), 0, 0, 0);
    ecl_nnc_geometry_type* nnc_geo = ecl_nnc_geometry_alloc(grid);

    {
        ecl_nnc_data_type* waterFluxData = ecl_nnc_data_alloc_wat_flux(grid, nnc_geo, summaryView);
        const double* waterFluxValues = ecl_nnc_data_get_values(waterFluxData);
        waterFlux->insert(waterFlux->end(), &waterFluxValues[0], &waterFluxValues[ecl_nnc_data_get_size(waterFluxData)]);
        ecl_nnc_data_free(waterFluxData);
    }
    {
        ecl_nnc_data_type* oilFluxData = ecl_nnc_data_alloc_oil_flux(grid, nnc_geo, summaryView);
        const double* oilFluxValues = ecl_nnc_data_get_values(oilFluxData);
        oilFlux->insert(oilFlux->end(), &oilFluxValues[0], &oilFluxValues[ecl_nnc_data_get_size(oilFluxData)]);
        ecl_nnc_data_free(oilFluxData);
    }
    {
        ecl_nnc_data_type* gasFluxData = ecl_nnc_data_alloc_gas_flux(grid, nnc_geo, summaryView);
        const double* gasFluxValues = ecl_nnc_data_get_values(gasFluxData);
        gasFlux->insert(gasFlux->end(), &gasFluxValues[0], &gasFluxValues[ecl_nnc_data_get_size(gasFluxData)]);
        ecl_nnc_data_free(gasFluxData);
    }

    ecl_nnc_geometry_free(nnc_geo);

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::readWellData(well_info_type* well_info, bool importCompleteMswData)
{
    if (!well_info) return;

    if (openFile())
    {
        well_info_add_UNRST_wells(well_info, m_ecl_file, importCompleteMswData);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::setRestartFiles(const QStringList& fileSet)
{
    m_filename = fileSet[0];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifEclipseUnifiedRestartFileAccess::readUnitsType()
{
    openFile();

    return RifEclipseOutputFileTools::readUnitsType(m_ecl_file);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RifEclipseUnifiedRestartFileAccess::reportNumbers()
{
    std::vector<int> reportNr;

    if (openFile())
    {
        // Taken from well_info_add_UNRST_wells

        int num_blocks = ecl_file_get_num_named_kw(m_ecl_file, SEQNUM_KW);
        int block_nr;
        for (block_nr = 0; block_nr < num_blocks; block_nr++) {
            ecl_file_push_block(m_ecl_file);      // <-------------------------------------------------------
            {                                                                                               //
                ecl_file_subselect_block(m_ecl_file, SEQNUM_KW, block_nr);                                  //  Ensure that the status
                {                                                                                             //  is not changed as a side
                    const ecl_kw_type * seqnum_kw = ecl_file_iget_named_kw(m_ecl_file, SEQNUM_KW, 0);          //  effect.
                    int report_nr = ecl_kw_iget_int(seqnum_kw, 0);                                           //

                    reportNr.push_back(report_nr);
                }                                                                                             //
            }                                                                                               //
            ecl_file_pop_block(m_ecl_file);       // <-------------------------------------------------------
        }
    }

    return reportNr;
}

