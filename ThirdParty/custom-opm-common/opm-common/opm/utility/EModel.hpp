/*
   Copyright 2019 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#ifndef EMODEL_HPP
#define EMODEL_HPP


#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/ERst.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <map>


class EModel
{
public:

    explicit EModel(const std::string& filename);

    bool hasParameter(const std::string &name) const;

    int getActiveReportStep() { return activeReportStep; }
    bool hasReportStep(int rstep);
    void setReportStep(int rstep);

    std::vector<std::tuple<std::string, Opm::EclIO::eclArrType>> getListOfParameters() const;

    std::vector<int> getListOfReportSteps() const {return rstfile->listOfReportStepNumbers(); };

    template <typename T>
    const std::vector<T>& getParam(const std::string& name);

    void resetFilter();

    template <typename T>
    void addFilter(const std::string& param1, const std::string& opperator, T num);

    template <typename T>
    void addFilter(const std::string& param1, const std::string& opperator, T num1, T num2);

    void setDepthfwl(const std::vector<float>& fwl);

    void addHCvolFilter();

    int getNumberOfActiveCells();


    std::tuple<int, int, int> gridDims(){ return std::make_tuple(nI, nJ, nK); };


private:

    int nI, nJ, nK;
    int activeReportStep;

    size_t nActive;

    bool activeFilter, celVolCalculated;

    std::vector<float> filteredFloatVect;
    std::vector<int> filteredIntVect;

    std::vector<float> PORV;
    std::vector<float> CELLVOL;
    std::vector<int> I, J, K;
    std::vector<bool> ActFilter;

    Opm::EclIO::EclFile initfile;
    std::optional<Opm::EclipseGrid> grid;
    std::optional<Opm::EclIO::ERst> rstfile;


    std::map<std::string, int> initParam;
    std::vector<std::string> initParamName;
    std::vector<Opm::EclIO::eclArrType> initParamType;
    std::vector<int> indInInitEclfile;

    std::map<std::string, int> solutionParam;
    std::vector<std::string> solutionParamName;
    std::vector<Opm::EclIO::eclArrType> solutionParamType;
    std::vector<int> indInRstEclfile;

    int nEqlnum=0;
    std::vector<float> FreeWaterlevel = {};

    void get_cell_volumes_from_grid();
    void initSolutionData(int rstep);

    bool hasInitParameter(const std::string &name) const;
    bool hasSolutionParameter(const std::string &name) const;

    const std::vector<float>& getInitFloat(const std::string& name);

    const std::vector<float>& getSolutionFloat(const std::string& name);

    template <typename T>
    const std::vector<T>& get_filter_param(const std::string& param1);

    template <typename T>
    void updateActiveFilter(const std::vector<T>& paramVect, const std::string& opperator, T value);

    template <typename T>
    void updateActiveFilter(const std::vector<T>& paramVect, const std::string& opperator, T value1, T value2);

};

#endif
