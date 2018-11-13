/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigCompletionData.h"

#include <map>
#include <vector>

class RigFractureGrid;
class RicWellPathFractureReportItem;
class RigWellPath;
class RigTransmissibilityCondenser;
class RigEclipseToStimPlanCalculator;

class RimEclipseCase;
class RimFracture;
class RimFractureTemplate;
class RimSimWellInView;
class RimWellPath;

class QTextStream;
class QString;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RicExportFractureCompletionsImpl
{
public:

    enum PressureDepletionWBHPSource
    {
        WBHP_FROM_SUMMARY,
        WBHP_FROM_USER_DEF
    };

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    struct PressureDepletionParameters
    {
        PressureDepletionParameters(bool                        performScaling = false,
                                    int                         pressureScalingTimeStep = 0,
                                    PressureDepletionWBHPSource wbhpSource = WBHP_FROM_SUMMARY,
                                    double                      userWBHP = 200.0)
            : performScaling(performScaling)
            , pressureScalingTimeStep(pressureScalingTimeStep)
            , wbhpSource(wbhpSource)
            , userWBHP(userWBHP)
        {}

        bool                        performScaling;
        int                         pressureScalingTimeStep;
        PressureDepletionWBHPSource wbhpSource;
        double                      userWBHP;
    };     

    static std::vector<RigCompletionData> generateCompdatValuesForWellPath(RimWellPath* wellPath,
                                                                           RimEclipseCase* caseToApply,
                                                                           std::vector<RicWellPathFractureReportItem>* fractureDataForReport,
                                                                           QTextStream* outputStreamForIntermediateResultsText,
                                                                           PressureDepletionParameters pdParams = PressureDepletionParameters());

    static std::vector<RigCompletionData> generateCompdatValuesForSimWell(RimEclipseCase* eclipseCase,
                                                                          const RimSimWellInView* well,
                                                                          QTextStream* outputStreamForIntermediateResultsText,
                                                                          PressureDepletionParameters pdParams = PressureDepletionParameters());

    static std::vector<RigCompletionData> generateCompdatValues(RimEclipseCase* caseToApply,
                                                                const QString& wellPathName,
                                                                const RigWellPath* wellPathGeometry,
                                                                const std::vector<const RimFracture*>& fractures,
                                                                std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
                                                                QTextStream* outputStreamForIntermediateResultsText,
                                                                PressureDepletionParameters pdParams = PressureDepletionParameters());

    static void getWellPressuresAndInitialProductionTimeStepFromSummaryData(const RimEclipseCase* caseToApply,
                                                                            const QString&        wellPathName,
                                                                            int                   currentTimeStep,
                                                                            int*                  initialTimeStep,
                                                                            double*               initialWellPressure,
                                                                            double*               currentWellPressure);

private:
    static std::vector<RigCompletionData> generateCompdatValuesConst(const RimEclipseCase* caseToApply,
                                                                     const QString& wellPathName,
                                                                     const RigWellPath* wellPathGeometry,
                                                                     const std::vector<const RimFracture*>& fractures,
                                                                     std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
                                                                     QTextStream* outputStreamForIntermediateResultsText,
                                                                     PressureDepletionParameters pdParams);

    static bool checkForStimPlanConductivity(const RimFractureTemplate* fracTemplate, const RimFracture* fracture);

    static void calculateInternalFractureTransmissibilities(const RigFractureGrid*        fractureGrid,
                                                            double                        cDarcyInCorrectUnit,
                                                            RigTransmissibilityCondenser& transCondenser);

    static void calculateFractureToWellTransmissibilities(const RimFractureTemplate*    fracTemplate,
                                                          const RigFractureGrid*        fractureGrid,
                                                          const RimFracture*            fracture,
                                                          double                        cDarcyInCorrectUnit,
                                                          const RigWellPath*            wellPathGeometry,
                                                          RigTransmissibilityCondenser& transCondenser);

    static std::map<size_t, double> calculateMatrixToWellTransmissibilities(RigTransmissibilityCondenser& transCondenser);

    static std::vector<RigCompletionData> generateCompdatValuesForFracture(const std::map<size_t, double>& matrixToWellTransmissibilites,
                                                                           const QString&                  wellPathName,
                                                                           const RimEclipseCase*           caseToApply,
                                                                           const RimFracture*              fracture,
                                                                           const RimFractureTemplate*      fracTemplate);

    static void computeNonDarcyFlowParameters(const RimFracture* fracture, std::vector<RigCompletionData>& allCompletionsForOneFracture);

    static double sumUpTransmissibilities(const std::vector<RigCompletionData>& allCompletionsForOneFracture);

    static void calculateAndSetReportItemData(const std::vector<RigCompletionData>& allCompletionsForOneFracture,
                                              const RigEclipseToStimPlanCalculator& calculator,
                                              RicWellPathFractureReportItem&        reportItem);

    static void outputIntermediateResultsText(QTextStream*                  outputStreamForIntermediateResultsText,
                                              const RimFracture*            fracture,
                                              RigTransmissibilityCondenser& transCondenser,
                                              const RigMainGrid*            mainGrid,
                                              const RigFractureGrid*        fractureGrid);
};
