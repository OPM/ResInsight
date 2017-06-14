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

#include <QDebug>

#include "cvfMath.h"
#include "RivWellFracturePartMgr.h"
#include "RigFractureCell.h"
#include "RiaLogging.h"
#include "RigFractureGrid.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanResultFrames::RigStimPlanResultFrames()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::RigStimPlanFractureDefinition() : unitSet(RiaEclipseUnitTools::UNITS_UNKNOWN)
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
std::vector<double> RigStimPlanFractureDefinition::getNegAndPosXcoords() const
{
    std::vector<double> allXcoords;
    for ( const double& xCoord : gridXs )
    {
        if ( xCoord > 1e-5 )
        {
            double negXcoord = -xCoord;
            allXcoords.insert(allXcoords.begin(), negXcoord);
        }
    }
    for ( const double& xCoord : gridXs )
    {
        allXcoords.push_back(xCoord);
    }

    return allXcoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigStimPlanFractureDefinition::numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep)
{
    size_t depths = this->depths.size();
    size_t gridXvalues = this->gridXs.size();

    if ( propertyValuesAtTimestep.size() != depths )  return false;
    for ( std::vector<double> valuesAtDepthVector : propertyValuesAtTimestep )
    {
        if ( valuesAtDepthVector.size() != gridXvalues ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStimPlanFractureDefinition::adjustedDepthCoordsAroundWellPathPosition(double wellPathDepthAtFracture) const
{
    std::vector<double> depthRelativeToWellPath;

    for ( const double& depth : this->depths )
    {
        double adjustedDepth = depth - wellPathDepthAtFracture;
        adjustedDepth = -adjustedDepth;
        depthRelativeToWellPath.push_back(adjustedDepth);
    }
    return depthRelativeToWellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString> > RigStimPlanFractureDefinition::getStimPlanPropertyNamesUnits() const
{
    std::vector<std::pair<QString, QString> >  propertyNamesUnits;
    {
        std::vector<RigStimPlanResultFrames > allStimPlanData = this->m_stimPlanResults;
        for ( RigStimPlanResultFrames stimPlanDataEntry : allStimPlanData )
        {
            propertyNamesUnits.push_back(std::make_pair(stimPlanDataEntry.resultName, stimPlanDataEntry.unit));
        }
    }
    return propertyNamesUnits;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RigStimPlanFractureDefinition::getMirroredDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const
{
    std::vector<std::vector<double>> notMirrordedData = this->getDataAtTimeIndex(resultName, unitName, timeStepIndex);
    std::vector<std::vector<double>> mirroredData;

    for ( std::vector<double> depthData : notMirrordedData )
    {
        std::vector<double> mirrordDepthData = RivWellFracturePartMgr::mirrorDataAtSingleDepth(depthData);
        mirroredData.push_back(mirrordDepthData);
    }

    return mirroredData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigFractureGrid> RigStimPlanFractureDefinition::createFractureGrid(const QString& resultNameFromColors, const QString& resultUnitFromColors, int m_activeTimeStepIndex, RiaEclipseUnitTools::UnitSystemType fractureTemplateUnit, double m_wellPathDepthAtFracture)
{
    std::vector<RigFractureCell> stimPlanCells;
    std::pair<size_t, size_t> wellCenterStimPlanCellIJ = std::make_pair(0, 0);

    bool wellCenterStimPlanCellFound = false;

    std::vector<std::vector<double>> displayPropertyValuesAtTimeStep = this->getMirroredDataAtTimeIndex(resultNameFromColors, resultUnitFromColors, m_activeTimeStepIndex);

    QString condUnit;
    if ( fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC ) condUnit = "md-m";
    if ( fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD )  condUnit = "md-ft";
    std::vector<std::vector<double>> conductivityValuesAtTimeStep = this->getMirroredDataAtTimeIndex("CONDUCTIVITY", condUnit, m_activeTimeStepIndex);

    std::vector<double> depthCoordsAtNodes = this->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture);
    std::vector<double> xCoordsAtNodes = this->getNegAndPosXcoords();

    std::vector<double> xCoords;
    for ( int i = 0; i < xCoordsAtNodes.size() - 1; i++ ) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 2);
    std::vector<double> depthCoords;
    for ( int i = 0; i < depthCoordsAtNodes.size() - 1; i++ ) depthCoords.push_back((depthCoordsAtNodes[i] + depthCoordsAtNodes[i + 1]) / 2);

    for ( int i = 0; i < xCoords.size() - 1; i++ )
    {
        for ( int j = 0; j < depthCoords.size() - 1; j++ )
        {
            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j]), 0.0));
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j]), 0.0));
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j + 1]), 0.0));
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j + 1]), 0.0));

            RigFractureCell stimPlanCell(cellPolygon, i, j);
            if ( conductivityValuesAtTimeStep.size() > 0 ) //Assuming vector to be of correct length, or no values
            {
                stimPlanCell.setConductivityValue(conductivityValuesAtTimeStep[j + 1][i + 1]);
            }
            else
            {
                stimPlanCell.setConductivityValue(cvf::UNDEFINED_DOUBLE);
            }

            if ( displayPropertyValuesAtTimeStep.size() > 0 )
            {
                stimPlanCell.setDisplayValue(displayPropertyValuesAtTimeStep[j + 1][i + 1]);
            }
            else
            {
                stimPlanCell.setDisplayValue(cvf::UNDEFINED_DOUBLE);
            }

            if ( cellPolygon[0].x() < 0.0 && cellPolygon[1].x() > 0.0 )
            {
                if ( cellPolygon[1].y() > 0.0 && cellPolygon[2].y() < 0.0 )
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
    m_fractureGrid->setICellCount(this->getNegAndPosXcoords().size() - 2);
    m_fractureGrid->setJCellCount(this->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture).size() - 2);

    return m_fractureGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::createFractureTriangleGeometry(double m_wellPathDepthAtFracture, 
                                                                   RiaEclipseUnitTools::UnitSystem neededUnit, 
                                                                   const QString& fractureUserName, 
                                                                   std::vector<cvf::Vec3f>* vertices, 
                                                                   std::vector<cvf::uint>* triangleIndices)
{
    std::vector<double> xCoords = this->getNegAndPosXcoords();
    cvf::uint lenXcoords = static_cast<cvf::uint>(xCoords.size());

    std::vector<double> adjustedDepths = this->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture);

    if ( neededUnit == unitSet )
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(fractureUserName));
    }

    else if ( unitSet == RiaEclipseUnitTools::UNITS_METRIC && neededUnit == RiaEclipseUnitTools::UNITS_FIELD )
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(fractureUserName));
        for ( double& value : adjustedDepths ) value = RiaEclipseUnitTools::meterToFeet(value);
        for ( double& value : xCoords )        value = RiaEclipseUnitTools::meterToFeet(value);
    }
    else if ( unitSet == RiaEclipseUnitTools::UNITS_FIELD && neededUnit == RiaEclipseUnitTools::UNITS_METRIC )
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(fractureUserName));
        for ( double& value : adjustedDepths ) value = RiaEclipseUnitTools::feetToMeter(value);
        for ( double& value : xCoords )        value = RiaEclipseUnitTools::feetToMeter(value);
    }
    else
    {
        //Should never get here...
        RiaLogging::error(QString("Error: Could not convert units for fracture template %1").arg(fractureUserName));
        return;
    }

    for ( cvf::uint k = 0; k < adjustedDepths.size(); k++ )
    {
        for ( cvf::uint i = 0; i < lenXcoords; i++ )
        {
            cvf::Vec3f node = cvf::Vec3f(xCoords[i], adjustedDepths[k], 0);
            vertices->push_back(node);

            if ( i < lenXcoords - 1 && k < adjustedDepths.size() - 1 )
            {
                if ( xCoords[i] < 1e-5 )
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

    for (int i = 1; i < polygon.size() - 1; i++)
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
std::vector<cvf::Vec3f> RigStimPlanFractureDefinition::createFractureBorderPolygon(const QString& resultName, const QString& resultUnit, int m_activeTimeStepIndex, double m_wellPathDepthAtFracture, RiaEclipseUnitTools::UnitSystem neededUnit, const QString& fractureUserName)
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<std::vector<double>> dataAtTimeStep = this->getDataAtTimeIndex(resultName, resultUnit, m_activeTimeStepIndex);

    std::vector<double> adjustedDepths = this->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture);

    for ( int k = 0; k < dataAtTimeStep.size(); k++ )
    {
        for ( int i = 0; i < dataAtTimeStep[k].size(); i++ )
        {
            if ( (dataAtTimeStep[k])[i] < 1e-7 )  //polygon should consist of nodes with value 0
            {
                if ( (i > 0) && ((dataAtTimeStep[k])[(i - 1)] > 1e-7) ) //side neighbour cell different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(this->gridXs[i]),
                                                 static_cast<float>(adjustedDepths[k]), 0.0f));
                }
                else if ( (k < dataAtTimeStep.size() - 1) && ((dataAtTimeStep[k + 1])[(i)] > 1e-7) )//cell below different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(this->gridXs[i]),
                                                 static_cast<float>(adjustedDepths[k]), 0.0f));
                }
                else if ( (k > 0) && ((dataAtTimeStep[k - 1])[(i)] > 1e-7) )//cell above different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(this->gridXs[i]),
                                                 static_cast<float>(adjustedDepths[k]), 0.0f));
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


    if ( neededUnit == unitSet )
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(fractureUserName));
    }

    else if ( unitSet == RiaEclipseUnitTools::UNITS_METRIC && neededUnit == RiaEclipseUnitTools::UNITS_FIELD )
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
    else if ( unitSet == RiaEclipseUnitTools::UNITS_FIELD && neededUnit == RiaEclipseUnitTools::UNITS_METRIC )
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
bool RigStimPlanFractureDefinition::timeStepExisist(double timeStepValueToCheck)
{
    for (double timeStep : timeSteps)
    {
        if (abs(timeStepValueToCheck - timeStep) < 1e-5) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::reorderYgridToDepths()
{
    std::vector<double> depthsInIncreasingOrder;
    for (double gridYvalue : gridYs)
    {
        depthsInIncreasingOrder.insert(depthsInIncreasingOrder.begin(), gridYvalue);
    }
    depths = depthsInIncreasingOrder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::getTimeStepIndex(double timeStepValue)
{
    size_t index = 0;
    while (index < timeSteps.size())
    {
        if (abs(timeSteps[index] - timeStepValue) < 1e-4)
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
    return timeSteps.size();
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

        std::vector<std::vector<std::vector<double>>>  values(timeSteps.size());
        resultData.parameterValues = values;
        resultData.parameterValues[getTimeStepIndex(timeStepValue)] = data;

        m_stimPlanResults.push_back(resultData);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RigStimPlanFractureDefinition::getDataAtTimeIndex(const QString& resultName, const QString& unit, size_t timeStepIndex) const
{
    size_t resIndex = resultIndex(resultName, unit);

    if (resIndex != cvf::UNDEFINED_SIZE_T)
    {
        if (timeStepIndex < m_stimPlanResults[resIndex].parameterValues.size())
        {
            return m_stimPlanResults[resIndex].parameterValues[timeStepIndex];
        }
    }

    qDebug() << "ERROR: Requested parameter does not exists in stimPlan data";
    std::vector<std::vector<double>> emptyVector;
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::computeMinMax(const QString& resultName, const QString& unit, double* minValue, double* maxValue) const
{
    CVF_ASSERT(minValue && maxValue);

    size_t resIndex = resultIndex(resultName, unit);
    if (resIndex == cvf::UNDEFINED_SIZE_T) return;

    for (auto timeValues : m_stimPlanResults[resIndex].parameterValues)
    {
        for (auto values : timeValues)
        {
            for (auto resultValue : values)
            {
                if (resultValue < *minValue)
                {
                    *minValue = resultValue;
                }

                if (resultValue > *maxValue)
                {
                    *maxValue = resultValue;
                }
            }
        }
    }
}

