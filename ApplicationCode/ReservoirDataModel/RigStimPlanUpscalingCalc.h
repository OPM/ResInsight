#pragma once

#include "RimEclipseCase.h"
#include "RimFracture.h"
#include "RimStimPlanFractureTemplate.h"


class RigStimPlanUpscalingCalc
{
public:
    explicit RigStimPlanUpscalingCalc(RimEclipseCase* caseToApply, RimFracture* fracture);

    std::vector<RigFracturedEclipseCellExportData> computeUpscaledPropertyFromStimPlan(QString resultName, QString resultUnit, size_t timeStepIndex);

private:
    std::pair<double, double>   flowAcrossLayersUpscaling(QString resultName, QString resultUnit, size_t timeStepIndex, RimDefines::UnitSystem unitSystem, size_t eclipseCellIndex);
    double                      computeHAupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigFractureCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers);
    double                      computeAHupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigFractureCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers);
    static double               arithmeticAverage(std::vector<double> values);

    static std::vector<RigFractureCell*>    getRowOfStimPlanCells(std::vector<RigFractureCell>& allStimPlanCells, size_t i);
    static std::vector<RigFractureCell*>    getColOfStimPlanCells(std::vector<RigFractureCell>& allStimPlanCells, size_t j);

private:
    RimEclipseCase*         m_case;
    RimFracture*            m_fracture;
    RimDefines::UnitSystem  m_unitForCalculation;

};

