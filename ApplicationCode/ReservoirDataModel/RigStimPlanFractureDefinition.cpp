/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigStimPlanFractureDefinition.h"

#include "RiaFractureDefines.h"
#include "RiaLogging.h"

#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigStatisticsMath.h"

#include "RivWellFracturePartMgr.h"

#include "cvfMath.h"

#include <cmath>


//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
size_t findMirrorXIndex(std::vector<double> xs);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const double RigStimPlanFractureDefinition::THRESHOLD_VALUE = 1e-5;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::RigStimPlanFractureDefinition() : 
    m_unitSet(RiaEclipseUnitTools::UNITS_UNKNOWN), m_topPerfTvd(HUGE_VAL), m_bottomPerfTvd(HUGE_VAL)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::~RigStimPlanFractureDefinition()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RigStimPlanFractureDefinition::unitSet() const
{
    return m_unitSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::xCount() const
{
    return m_Xs.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::yCount() const
{
    return m_Ys.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::minDepth() const
{
    return -minY();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::maxDepth() const
{
    return -maxY();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::topPerfTvd() const
{
    return m_topPerfTvd;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::bottomPerfTvd() const
{
    return m_bottomPerfTvd;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::minY() const
{
    return m_Ys[0];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::maxY() const
{
    return m_Ys.back();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setTvdToTopPerf(double topPerfTvd)
{
    m_topPerfTvd = topPerfTvd;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setTvdToBottomPerf(double bottomPerfTvd)
{
    m_bottomPerfTvd = bottomPerfTvd;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::generateXsFromFileXs(bool xMirrorMode)
{
    m_xMirrorMode = xMirrorMode;
    m_Xs.clear();

    if (m_xMirrorMode)
    {
        size_t mirrorIndex = findMirrorXIndex(m_fileXs);
        std::list<double> xs;

        // Mirror positive X values
        xs.push_back(m_fileXs[mirrorIndex]);
        for (size_t i = mirrorIndex + 1; i < m_fileXs.size(); i++)
        {
            xs.push_front(-m_fileXs[i]);
            xs.push_back(m_fileXs[i]);
        }
        m_Xs = std::vector<double>(xs.begin(), xs.end());
    }
    else
    {
        m_Xs = m_fileXs;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RigStimPlanFractureDefinition::generateDataLayoutFromFileDataLayout(std::vector<std::vector<double>> fileXYData)
{
    if (m_xMirrorMode)
    {
        std::vector<std::vector<double>> xyData;
        size_t mirrorIndex = findMirrorXIndex(m_fileXs);

        for (const auto& yData : fileXYData)
        {
            std::list<double> xValues;

            // Mirror positive X values
            xValues.push_back(yData[mirrorIndex]);
            for (size_t x = mirrorIndex + 1; x < yData.size(); x++)
            {
                xValues.push_front(yData[x]);
                xValues.push_back(yData[x]);
            }
            xyData.push_back(std::vector<double>(xValues.begin(), xValues.end()));
        }
        return xyData;
    }
    else
    {
        return fileXYData;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigStimPlanFractureDefinition::numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep)
{
    size_t xCount = m_Xs.size();

    if ( propertyValuesAtTimestep.size() != yCount())  return false;
    for ( std::vector<double> valuesAtDepthVector : propertyValuesAtTimestep )
    {
        if ( valuesAtDepthVector.size() != xCount ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStimPlanFractureDefinition::adjustedYCoordsAroundWellPathPosition(double wellPathIntersectionAtFractureDepth) const
{
    std::vector<double> yRelativeToWellPath;

    for ( const double& y : m_Ys )
    {
        double adjustedDepth = y + wellPathIntersectionAtFractureDepth;
        yRelativeToWellPath.push_back(adjustedDepth);
    }
    return yRelativeToWellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString> > RigStimPlanFractureDefinition::getStimPlanPropertyNamesUnits() const
{
    std::vector<std::pair<QString, QString> >  propertyNamesUnits;
    {
        for ( const RigStimPlanResultFrames& stimPlanDataEntry : this->m_stimPlanResults )
        {
            propertyNamesUnits.push_back(std::make_pair(stimPlanDataEntry.resultName, stimPlanDataEntry.unit));
        }
    }
    return propertyNamesUnits;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigFractureGrid> RigStimPlanFractureDefinition::createFractureGrid(const QString& resultName,
                                                                            int activeTimeStepIndex, 
                                                                            RiaEclipseUnitTools::UnitSystemType fractureTemplateUnit, 
                                                                            double wellPathIntersectionAtFractureDepth)
{
    std::vector<RigFractureCell> stimPlanCells;
    std::pair<size_t, size_t> wellCenterStimPlanCellIJ = std::make_pair(0, 0);

    bool wellCenterStimPlanCellFound = false;

    QString condUnit = RiaDefines::unitStringConductivity(fractureTemplateUnit);

    std::vector<std::vector<double>> conductivityValuesAtTimeStep = this->getDataAtTimeIndex(resultName, 
                                                                                             condUnit, 
                                                                                             activeTimeStepIndex);

    std::vector<double> yCoordsAtNodes = this->adjustedYCoordsAroundWellPathPosition(wellPathIntersectionAtFractureDepth);
    std::vector<double> xCoordsAtNodes = this->m_Xs;

    std::vector<double> xCoords;
    for ( int i = 0; i < static_cast<int>(xCoordsAtNodes.size()) - 1; i++ ) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 2);
    std::vector<double> depthCoords;
    for ( int i = 0; i < static_cast<int>(yCoordsAtNodes.size()) - 1; i++ ) depthCoords.push_back((yCoordsAtNodes[i] + yCoordsAtNodes[i + 1]) / 2);

    for ( int i = 0; i < static_cast<int>(xCoords.size()) - 1; i++ )
    {
        for ( int j = 0; j < static_cast<int>(depthCoords.size()) - 1; j++ )
        {
            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back(cvf::Vec3d(xCoords[i],     depthCoords[j],     0.0));
            cellPolygon.push_back(cvf::Vec3d(xCoords[i + 1], depthCoords[j],     0.0));
            cellPolygon.push_back(cvf::Vec3d(xCoords[i + 1], depthCoords[j + 1], 0.0));
            cellPolygon.push_back(cvf::Vec3d(xCoords[i],     depthCoords[j + 1], 0.0));

            RigFractureCell stimPlanCell(cellPolygon, i, j);
            if ( conductivityValuesAtTimeStep.size() > 0 ) //Assuming vector to be of correct length, or no values
            {
                stimPlanCell.setConductivityValue(conductivityValuesAtTimeStep[j + 1][i + 1]);
            }
            else
            {
                stimPlanCell.setConductivityValue(cvf::UNDEFINED_DOUBLE);
            }

            // The well path is intersecting the fracture at origo in the fracture coordinate system
            // Find the Stimplan cell where the well path is intersecting

            if ( cellPolygon[0].x() <= 0.0 && cellPolygon[1].x() >= 0.0 )
            {
                if ( cellPolygon[1].y() >= 0.0 && cellPolygon[2].y() <= 0.0 )
                {
                    wellCenterStimPlanCellIJ = std::make_pair(stimPlanCell.getI(), stimPlanCell.getJ());
                    RiaLogging::debug(QString("Setting wellCenterStimPlanCell at cell %1, %2").
                                      arg(QString::number(stimPlanCell.getI()), QString::number(stimPlanCell.getJ())));

                    wellCenterStimPlanCellFound = true;
                }
            }

            stimPlanCells.push_back(stimPlanCell);
        }
    }

    if ( !wellCenterStimPlanCellFound )
    {
        RiaLogging::error("Did not find stim plan cell at well crossing!");
    }

    cvf::ref<RigFractureGrid> m_fractureGrid = new RigFractureGrid;
    m_fractureGrid->setFractureCells(stimPlanCells);
    m_fractureGrid->setWellCenterFractureCellIJ(wellCenterStimPlanCellIJ);
    m_fractureGrid->setICellCount(this->m_Xs.size() - 2);
    m_fractureGrid->setJCellCount(this->adjustedYCoordsAroundWellPathPosition(wellPathIntersectionAtFractureDepth).size() - 2);

    return m_fractureGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStimPlanFractureDefinition::fractureGridResults(const QString& resultName,
                                                                       const QString& unitName,
                                                                       size_t timeStepIndex) const
{
    std::vector<double> fractureGridResults;
    const std::vector<std::vector<double>>& resultValuesAtTimeStep = this->getDataAtTimeIndex(resultName, 
                                                                                              unitName,
                                                                                              timeStepIndex);

    for ( int i = 0; i < static_cast<int>(xCount()) - 2; i++ )
    {
        for ( int j = 0; j < static_cast<int>(yCount()) - 2; j++ )
        {
            if ( j+1 < static_cast<int>(resultValuesAtTimeStep.size()) && i+1 < static_cast<int>(resultValuesAtTimeStep[j + 1].size()) )
            {
                fractureGridResults.push_back(resultValuesAtTimeStep[j + 1][i + 1]);
            }
            else
            {
                fractureGridResults.push_back(HUGE_VAL);
            }
        }
    }

    return fractureGridResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::createFractureTriangleGeometry(double wellPathIntersectionAtFractureDepth,
                                                                   RiaEclipseUnitTools::UnitSystem neededUnit, 
                                                                   const QString& fractureUserName, 
                                                                   std::vector<cvf::Vec3f>* vertices, 
                                                                   std::vector<cvf::uint>* triangleIndices)
{
    std::vector<double> xCoords = this->m_Xs;
    cvf::uint lenXcoords = static_cast<cvf::uint>(xCoords.size());

    std::vector<double> adjustedYs = this->adjustedYCoordsAroundWellPathPosition(wellPathIntersectionAtFractureDepth);

    if ( neededUnit == m_unitSet )
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(fractureUserName));
    }

    else if ( m_unitSet == RiaEclipseUnitTools::UNITS_METRIC && neededUnit == RiaEclipseUnitTools::UNITS_FIELD )
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(fractureUserName));
        for ( double& value : adjustedYs ) value = RiaEclipseUnitTools::meterToFeet(value);
        for ( double& value : xCoords )        value = RiaEclipseUnitTools::meterToFeet(value);
    }
    else if ( m_unitSet == RiaEclipseUnitTools::UNITS_FIELD && neededUnit == RiaEclipseUnitTools::UNITS_METRIC )
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(fractureUserName));
        for ( double& value : adjustedYs ) value = RiaEclipseUnitTools::feetToMeter(value);
        for ( double& value : xCoords )        value = RiaEclipseUnitTools::feetToMeter(value);
    }
    else
    {
        //Should never get here...
        RiaLogging::error(QString("Error: Could not convert units for fracture template %1").arg(fractureUserName));
        return;
    }

    for ( cvf::uint k = 0; k < adjustedYs.size(); k++ )
    {
        for ( cvf::uint i = 0; i < lenXcoords; i++ )
        {
            cvf::Vec3f node = cvf::Vec3f(xCoords[i], adjustedYs[k], 0);
            vertices->push_back(node);

            if ( i < lenXcoords - 1 && k < adjustedYs.size() - 1 )
            {
                if ( xCoords[i] < THRESHOLD_VALUE )
                {
                    //Upper triangle
                    triangleIndices->push_back(i + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + (k + 1)*lenXcoords);
                    //Lower triangle
                    triangleIndices->push_back(i + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + (k + 1)*lenXcoords);
                    triangleIndices->push_back((i)+(k + 1)*lenXcoords);

                }
                else
                {
                    //Upper triangle
                    triangleIndices->push_back(i + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + k*lenXcoords);
                    triangleIndices->push_back((i)+(k + 1)*lenXcoords);
                    //Lower triangle
                    triangleIndices->push_back((i + 1) + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + (k + 1)*lenXcoords);
                    triangleIndices->push_back((i)+ (k + 1)*lenXcoords);
                }

            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void sortPolygon(std::vector<cvf::Vec3f> &polygon)
{
    if (polygon.size() == 0) return;

    for (int i = 1; i < static_cast<int>(polygon.size()) - 1; i++)
    {
        cvf::Vec3f lastNode = polygon[i - 1];
        cvf::Vec3f node = polygon[i];
        cvf::Vec3f nextNode = polygon[i + 1];

        if (node.y() == nextNode.y())
        {
            if (lastNode.x() < node.x() && node.x() > nextNode.x())
            {
                polygon[i] = nextNode;
                polygon[i + 1] = node;
            }
            else if (lastNode.x() > node.x() && node.x() < nextNode.x())
            {
                polygon[i] = nextNode;
                polygon[i + 1] = node;
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RigStimPlanFractureDefinition::createFractureBorderPolygon(const QString& resultName, 
                                                                                   const QString& resultUnit, 
                                                                                   int activeTimeStepIndex, 
                                                                                   double wellPathIntersectionAtFractureDepth,
                                                                                   RiaEclipseUnitTools::UnitSystem neededUnit, 
                                                                                   const QString& fractureUserName)
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<std::vector<double>> dataAtTimeStep = this->getDataAtTimeIndex(resultName, resultUnit, activeTimeStepIndex);

    std::vector<double> adjustedYs = this->adjustedYCoordsAroundWellPathPosition(wellPathIntersectionAtFractureDepth);

    for ( int k = 0; k < static_cast<int>(dataAtTimeStep.size()); k++ )
    {
        for ( int i = 0; i < static_cast<int>(dataAtTimeStep[k].size()); i++ )
        {
            if ( (dataAtTimeStep[k])[i] < 1e-7 )  //polygon should consist of nodes with value 0
            {
                if ( (i > 0) && ((dataAtTimeStep[k])[(i - 1)] > 1e-7) ) //side neighbour cell different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(this->m_Xs[i]),
                                                 static_cast<float>(adjustedYs[k]), 0.0f));
                }
                else if ( (k < static_cast<int>(dataAtTimeStep.size()) - 1) && ((dataAtTimeStep[k + 1])[(i)] > 1e-7) )//cell below different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(this->m_Xs[i]),
                                                 static_cast<float>(adjustedYs[k]), 0.0f));
                }
                else if ( (k > 0) && ((dataAtTimeStep[k - 1])[(i)] > 1e-7) )//cell above different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(this->m_Xs[i]),
                                                 static_cast<float>(adjustedYs[k]), 0.0f));
                }
            }
        }
    }

    sortPolygon(polygon);

    std::vector<cvf::Vec3f> negPolygon;
    for ( const cvf::Vec3f& node : polygon )
    {
        cvf::Vec3f negNode = node;
        negNode.x() = -negNode.x();
        negPolygon.insert(negPolygon.begin(), negNode);
    }

    for ( const cvf::Vec3f& negNode : negPolygon )
    {
        polygon.push_back(negNode);
    }

    //Adding first point last - to close the polygon
    if ( polygon.size()>0 ) polygon.push_back(polygon[0]);


    if ( neededUnit == m_unitSet )
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(fractureUserName));
    }

    else if ( m_unitSet == RiaEclipseUnitTools::UNITS_METRIC && neededUnit == RiaEclipseUnitTools::UNITS_FIELD )
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(fractureUserName));
        for ( cvf::Vec3f& node : polygon )
        {
            float x = RiaEclipseUnitTools::meterToFeet(node.x());
            float y = RiaEclipseUnitTools::meterToFeet(node.y());
            float z = RiaEclipseUnitTools::meterToFeet(node.z());
            node = cvf::Vec3f(x, y, z);
        }
    }
    else if ( m_unitSet == RiaEclipseUnitTools::UNITS_FIELD && neededUnit == RiaEclipseUnitTools::UNITS_METRIC )
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(fractureUserName));
        for ( cvf::Vec3f& node : polygon )
        {
            float x = RiaEclipseUnitTools::feetToMeter(node.x());
            float y = RiaEclipseUnitTools::feetToMeter(node.y());
            float z = RiaEclipseUnitTools::feetToMeter(node.z());
            node = cvf::Vec3f(x, y, z);
        }
    }
    else
    {
        //Should never get here...
        RiaLogging::error(QString("Error: Could not convert units for fracture template %1").arg(fractureUserName));
    }


    return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigStimPlanFractureDefinition::timeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::addTimeStep(double time)
{
    if (!timeStepExists(time)) m_timeSteps.push_back(time);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigStimPlanFractureDefinition::timeStepExists(double timeStepValueToCheck)
{
    for (double timeStep : m_timeSteps)
    {
        if (fabs(timeStepValueToCheck - timeStep) < THRESHOLD_VALUE) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::getTimeStepIndex(double timeStepValue)
{
    size_t index = 0;
    while (index < m_timeSteps.size())
    {
        if (fabs(m_timeSteps[index] - timeStepValue) < 1e-4)
        {
            return index;
        }
        index++;
    }
    return -1; //returns -1 if not found
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::totalNumberTimeSteps()
{
    return m_timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::resultIndex(const QString& resultName, const QString& unit) const
{
    
    for (size_t i = 0; i < m_stimPlanResults.size(); i++)
    {
        if (m_stimPlanResults[i].resultName == resultName && m_stimPlanResults[i].unit == unit)
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setDataAtTimeValue(QString resultName, QString unit, std::vector<std::vector<double>> data, double timeStepValue)
{
    size_t resIndex = resultIndex(resultName, unit);

    if (resIndex != cvf::UNDEFINED_SIZE_T)
    {
        m_stimPlanResults[resIndex].parameterValues[getTimeStepIndex(timeStepValue)] = data;
    }
    else
    {
        RigStimPlanResultFrames resultData;

        resultData.resultName = resultName;
        resultData.unit = unit;

        std::vector<std::vector<std::vector<double>>>  values(m_timeSteps.size());
        resultData.parameterValues = values;
        resultData.parameterValues[getTimeStepIndex(timeStepValue)] = data;

        m_stimPlanResults.push_back(resultData);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>& RigStimPlanFractureDefinition::getDataAtTimeIndex(const QString& resultName, const QString& unit, size_t timeStepIndex) const
{
    size_t resIndex = resultIndex(resultName, unit);

    if (resIndex != cvf::UNDEFINED_SIZE_T)
    {
        if (timeStepIndex < m_stimPlanResults[resIndex].parameterValues.size())
        {
            return m_stimPlanResults[resIndex].parameterValues[timeStepIndex];
        }
    }

    RiaLogging::error("Requested parameter does not exists in stimPlan data");

    static std::vector<std::vector<double>> emptyVector;
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::appendDataToResultStatistics(const QString& resultName, const QString& unit,
                                                                  MinMaxAccumulator& minMaxAccumulator,
                                                                  PosNegAccumulator& posNegAccumulator) const
{
    size_t resIndex = resultIndex(resultName, unit);
    if (resIndex == cvf::UNDEFINED_SIZE_T) return;

    for (const auto& timeValues : m_stimPlanResults[resIndex].parameterValues)
    {
        for (const auto& values : timeValues)
        {
            minMaxAccumulator.addData(values);
            posNegAccumulator.addData(values);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RigStimPlanFractureDefinition::conductivityResultNames() const
{
    QStringList resultNames;

    for (const auto& stimPlanResult : m_stimPlanResults)
    {
        if (stimPlanResult.resultName.contains("conductivity", Qt::CaseInsensitive))
        {
            resultNames.push_back(stimPlanResult.resultName);
        }
    }

    return resultNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t findMirrorXIndex(std::vector<double> xs)
{
    size_t mirrorIndex = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < xs.size(); i++) 
    {
        if (xs[i] > -RigStimPlanFractureDefinition::THRESHOLD_VALUE)
        {
            mirrorIndex = i;
            break;
        }
    }

    return mirrorIndex;
}
