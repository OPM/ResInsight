#include "RigStimPlanUpscalingCalc.h"
#include "RigMainGrid.h"
#include "RimEclipseCase.h"
#include "RigFractureTransCalc.h"
#include "RigEclipseCaseData.h"
#include "RigCellGeometryTools.h"
#include "RigFractureCell.h"
#include "RiaLogging.h"
#include "RigFractureGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanUpscalingCalc::RigStimPlanUpscalingCalc(RimEclipseCase* caseToApply, RimFracture* fracture)
{
    m_case = caseToApply;
    m_fracture = fracture;


    //Set correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = m_case->eclipseCaseData()->unitsType();

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating transmissibilities in metric units"));
        m_unitForCalculation = RimUnitSystem::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating transmissibilities in field units"));
        m_unitForCalculation = RimUnitSystem::UNITS_FIELD;
    }
    else
    {
        //TODO: How to handle lab units for eclipse case?
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        RiaLogging::error(QString("Export will be in metric units, but results might be wrong."));
        m_unitForCalculation = RimUnitSystem::UNITS_METRIC;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RigStimPlanUpscalingCalc::flowAcrossLayersUpscaling(QString resultName, QString resultUnit, size_t timeStepIndex, RimUnitSystem::UnitSystem unitSystem, size_t eclipseCellIndex)
{
    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->fractureTemplate()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->fractureTemplate());
    }
    else return std::make_pair(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);

    std::vector<RigFractureCell> stimPlanCells = fracTemplateStimPlan->fractureGrid()->fractureCells();



    cvf::Vec3d localX, localY, localZ; //Not used in calculation here, but needed for function to find planCellPolygons
    std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;

    const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
    cvf::Vec3d hexCorners[8];
    mainGrid->cellCornerVertices(eclipseCellIndex, hexCorners);



    bool isPlanIntersected = RigFractureTransCalc::planeCellIntersectionPolygons(hexCorners, m_fracture->transformMatrix(), planeCellPolygons);
    if (!isPlanIntersected || planeCellPolygons.size() == 0) return  std::make_pair(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);

    //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon/stimPlan mesh already is located)
    cvf::Mat4f invertedTransMatrix = m_fracture->transformMatrix().getInverted();
    for (std::vector<cvf::Vec3d> & planeCellPolygon : planeCellPolygons)
    {
        for (cvf::Vec3d& v : planeCellPolygon)
        {
            v.transformPoint(static_cast<cvf::Mat4d>(invertedTransMatrix));
        }
    }

    cvf::Vec3d directionAcrossLayers;
    cvf::Vec3d directionAlongLayers;
    directionAcrossLayers = cvf::Vec3d(0.0, -1.0, 0.0);
    directionAlongLayers = cvf::Vec3d(1.0, 0.0, 0.0);

    std::vector<cvf::Vec3f> fracPolygon = m_fracture->fractureTemplate()->fractureBorderPolygon(unitSystem);
    std::vector<std::vector<cvf::Vec3d> > polygonsDescribingFractureInCell;

    std::vector<double> upscaledConductivitiesHA;
    std::vector<double> upscaledConductivitiesAH;

    for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
    {
        double condHA = computeHAupscale(fracTemplateStimPlan, stimPlanCells, planeCellPolygon, directionAlongLayers, directionAcrossLayers);
        upscaledConductivitiesHA.push_back(condHA);

        double condAH = computeAHupscale(fracTemplateStimPlan, stimPlanCells, planeCellPolygon, directionAlongLayers, directionAcrossLayers);
        upscaledConductivitiesAH.push_back(condAH);
    }

    return std::make_pair(arithmeticAverage(upscaledConductivitiesHA), arithmeticAverage(upscaledConductivitiesAH));
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanUpscalingCalc::computeHAupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigFractureCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers)
{
    std::vector<double> DcolSum;
    std::vector<double> lavgCol;
    std::vector<double> CondHarmCol;

    for (size_t j = 0; j < fracTemplateStimPlan->fractureGrid()->iCellCount(); j++)
    {
        std::vector<double> conductivitiesInStimPlanCells;
        std::vector<double> lengthsLiOfStimPlanCol;
        std::vector<double> heightsDiOfStimPlanCells;

        std::vector<RigFractureCell*> stimPlanCellsCol = getColOfStimPlanCells(stimPlanCells, j);
        for (RigFractureCell* stimPlanCell : stimPlanCellsCol)
        {
            if (stimPlanCell->getConductivtyValue() > 1e-7)
            {
                std::vector<std::vector<cvf::Vec3d> >clippedStimPlanPolygons = RigCellGeometryTools::intersectPolygons(stimPlanCell->getPolygon(), planeCellPolygon);
                if (clippedStimPlanPolygons.size() > 0)
                {
                    for (auto clippedStimPlanPolygon : clippedStimPlanPolygons)
                    {
                        conductivitiesInStimPlanCells.push_back(stimPlanCell->getConductivtyValue());
                        lengthsLiOfStimPlanCol.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAlongLayers, clippedStimPlanPolygon));
                        heightsDiOfStimPlanCells.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAcrossLayers, clippedStimPlanPolygon));
                    }
                }
            }
        }
        //Regne ut average
        double sumDiDivCondLi = 0.0;
        double sumDi = 0.0;
        double sumLiDi = 0.0;
        for (size_t i = 0; i < conductivitiesInStimPlanCells.size(); i++)
        {
            sumDiDivCondLi += heightsDiOfStimPlanCells[i] / (conductivitiesInStimPlanCells[i] * lengthsLiOfStimPlanCol[i]);
            sumDi += heightsDiOfStimPlanCells[i];
            sumLiDi += heightsDiOfStimPlanCells[i] * lengthsLiOfStimPlanCol[i];
        }

        if (sumDiDivCondLi != 0)
        {
            DcolSum.push_back(sumDi);
            double lAvgValue = sumLiDi / sumDi;
            lavgCol.push_back(lAvgValue);
            double harmMeanForCol = (sumDi / lAvgValue) * (1 / sumDiDivCondLi);
            CondHarmCol.push_back(harmMeanForCol); 
        }
    }

    //Do arithmetic upscaling based on harmonic upscaled values for coloums
    double sumCondHLiDivDi = 0.0;
    double sumLi = 0.0;
    double sumDiLi = 0.0;
    for (size_t i = 0; i < CondHarmCol.size(); i++)
    {
        sumLi += lavgCol[i];
        sumDiLi += DcolSum[i] * lavgCol[i];
        sumCondHLiDivDi += CondHarmCol[i] * lavgCol[i] / DcolSum[i];
    }
    double Davg = sumDiLi / sumLi;
    double condHA = (Davg / sumLi) * sumCondHLiDivDi;

    return condHA;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanUpscalingCalc::computeAHupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigFractureCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers)
{
    std::vector<double> DrowAvg;
    std::vector<double> liRowSum;
    std::vector<double> CondAritRow;
    
    for (size_t j = 0; j < fracTemplateStimPlan->fractureGrid()->jCellCount(); j++)
    {
        std::vector<double> conductivitiesInStimPlanCells;
        std::vector<double> lengthsLiOfStimPlanCol;
        std::vector<double> heightsDiOfStimPlanCells;

        std::vector<RigFractureCell*> stimPlanCellsCol = getRowOfStimPlanCells(stimPlanCells, j);
        for (RigFractureCell* stimPlanCell : stimPlanCellsCol)
        {
            if (stimPlanCell->getConductivtyValue() > 1e-7)
            {
                std::vector<std::vector<cvf::Vec3d> >clippedStimPlanPolygons = RigCellGeometryTools::intersectPolygons(stimPlanCell->getPolygon(), planeCellPolygon);
                if (clippedStimPlanPolygons.size() > 0)
                {
                    for (auto clippedStimPlanPolygon : clippedStimPlanPolygons)
                    {
                        conductivitiesInStimPlanCells.push_back(stimPlanCell->getConductivtyValue());
                        lengthsLiOfStimPlanCol.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAlongLayers, clippedStimPlanPolygon));
                        heightsDiOfStimPlanCells.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAcrossLayers, clippedStimPlanPolygon));
                    }
                }
            }
        }
        //Calculate sums needed for (arithmetic) average for coloum
        double sumCondLiDivDi = 0.0;
        double sumDi = 0.0;
        double sumLiDi = 0.0;
        double sumLi = 0.0;
        for (size_t i = 0; i < conductivitiesInStimPlanCells.size(); i++)
        {
            sumCondLiDivDi += (conductivitiesInStimPlanCells[i] * lengthsLiOfStimPlanCol[i]) / heightsDiOfStimPlanCells[i];
            sumDi += heightsDiOfStimPlanCells[i];
            sumLiDi += heightsDiOfStimPlanCells[i] * lengthsLiOfStimPlanCol[i];
            sumLi += lengthsLiOfStimPlanCol[i];
        }

        if (sumCondLiDivDi != 0)
        {
            //Calculating art avg
            double dAvg = sumLiDi / sumLi;
            DrowAvg.push_back(dAvg);
            liRowSum.push_back(sumLi);
            CondAritRow.push_back(dAvg / sumLi * sumCondLiDivDi);
        }
    }

    //Do harmonic upscaling based on arithmetric upscaled values for coloums
    double sumDiDivCondALi = 0.0;
    double sumDi = 0.0;
    double sumDiLi = 0.0;
    for (size_t i = 0; i < CondAritRow.size(); i++)
    {
        sumDi += DrowAvg[i];
        sumDiLi += DrowAvg[i] * liRowSum[i];
        sumDiDivCondALi += DrowAvg[i] / (CondAritRow[i] * liRowSum[i]);

    }
    double Lavg = sumDiLi / sumDi;
    double condAH = (sumDi / Lavg) * (1 / sumDiDivCondALi);

    return condAH;



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanUpscalingCalc::arithmeticAverage(std::vector<double> values)
{
    if (values.size() == 0) return cvf::UNDEFINED_DOUBLE;

    double sumValue = 0.0;
    size_t numberOfValues = 0;
    for (double value : values)
    {
        sumValue += value;
        numberOfValues++;
    }

    return sumValue / numberOfValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigFracturedEclipseCellExportData>  RigStimPlanUpscalingCalc::computeUpscaledPropertyFromStimPlan( QString resultName, QString resultUnit, size_t timeStepIndex)
{
    std::vector<RigFracturedEclipseCellExportData> fracDataVec;

    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->fractureTemplate()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->fractureTemplate());
    }
    else 
    {
        return fracDataVec;
    }

    std::vector<std::vector<cvf::Vec3d> > stimPlanCellsAsPolygons;
    std::vector<double> stimPlanParameterValues;
    fracTemplateStimPlan->getStimPlanDataAsPolygonsAndValues(stimPlanCellsAsPolygons, stimPlanParameterValues, resultName, resultUnit, timeStepIndex);

    //TODO: A lot of common code with function above... Can be cleaned up...?

    std::vector<size_t> fracCells = m_fracture->getPotentiallyFracturedCells(m_case->eclipseCaseData()->mainGrid());

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = m_case->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = m_case->eclipseCaseData()->activeCellInfo(porosityModel);

    for (size_t fracCell : fracCells)
    {
        //TODO: Lage ny classe for å holde upscaledData
        RigFracturedEclipseCellExportData fracData;
        fracData.reservoirCellIndex = fracCell;
            
        std::pair<double, double> upscaledCondFlowAcrossLayers = flowAcrossLayersUpscaling(resultName, resultUnit, timeStepIndex, m_unitForCalculation, fracCell);
                
        double upscaledStimPlanValueHA = upscaledCondFlowAcrossLayers.first;
        double upscaledStimPlanValueAH = upscaledCondFlowAcrossLayers.second;
        
        if (upscaledStimPlanValueHA != cvf::UNDEFINED_DOUBLE)
        {
            fracData.upscaledStimPlanValueHA = upscaledStimPlanValueHA;
            fracData.upscaledStimPlanValueAH = upscaledStimPlanValueAH;

            fracDataVec.push_back(fracData);
        }
    }

    return fracDataVec;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigFractureCell*> RigStimPlanUpscalingCalc::getRowOfStimPlanCells(std::vector<RigFractureCell>& allStimPlanCells, size_t i)
{
    std::vector<RigFractureCell*> stimPlanCellRow;

    for (RigFractureCell stimPlanCell : allStimPlanCells)
    {
        if (stimPlanCell.getI() == i)
        {
            stimPlanCellRow.push_back(&stimPlanCell);
        }
    }

    return stimPlanCellRow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigFractureCell*> RigStimPlanUpscalingCalc::getColOfStimPlanCells(std::vector<RigFractureCell>& allStimPlanCells, size_t j)
{
    std::vector<RigFractureCell*> stimPlanCellCol;

    for (RigFractureCell stimPlanCell : allStimPlanCells)
    {
        if (stimPlanCell.getJ() == j)
        {
            stimPlanCellCol.push_back(&stimPlanCell);
        }
    }

    return stimPlanCellCol;
}

