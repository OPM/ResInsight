/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigCompletionDataGridCell.h"

#include <QString>

#include <vector>

//==================================================================================================
/// 
//==================================================================================================
enum WellConnectionState {
    OPEN,
    SHUT,
    AUTO,
};

//==================================================================================================
/// 
//==================================================================================================
enum CellDirection {
    DIR_I,
    DIR_J,
    DIR_K,
    DIR_UNDEF,
};

//==================================================================================================
/// 
//==================================================================================================
struct RigCompletionMetaData {
    RigCompletionMetaData(const QString& name, const QString& comment) : name(name), comment(comment) {}

    QString name;
    QString comment;
};

//==================================================================================================
/// 
//==================================================================================================
class RigCompletionData
{
public:
    enum CompletionType {
        FISHBONES,
        FRACTURE,
        PERFORATION,
        ICD,
        CT_UNDEFINED
    };

    RigCompletionData(const QString wellName, const RigCompletionDataGridCell& cellIndex, double orderingValue);
    ~RigCompletionData();

    RigCompletionData(const RigCompletionData& other);

    bool operator<(const RigCompletionData& other) const;
    RigCompletionData& operator=(const RigCompletionData& other);

    void        setFromFracture(double transmissibility, double skinFactor, double diameter);
    void        setSecondOrderingValue(double orderingValue);
    void        setDiameter(double diameter);
    void        setTransmissibility(double transmissibility);
    
    void        setTransAndWPImultBackgroundDataFromFishbone(double transmissibility, 
                                                             double skinFactor, 
                                                             double diameter,
                                                             CellDirection direction, 
                                                             bool isMainBore);

    void        setTransAndWPImultBackgroundDataFromPerforation(double transmissibility, 
                                                                double skinFactor, 
                                                                double diameter,
                                                                double dFactor,
                                                                double kh,
                                                                CellDirection direction);

    void        setCombinedValuesExplicitTrans(double         transmissibility,
                                               double         skinFactor,
                                               double         diameter,
                                               CellDirection  celldirection,
                                               CompletionType completionType);

    void        setCombinedValuesImplicitTransWPImult(double wpimult, 
                                                      double skinFactor,
                                                      double diameter, 
                                                      CellDirection celldirection, 
                                                      CompletionType completionType);

    bool        isNonDarcyFlow() const;
    void        setDFactor(double dFactor);
    void        setKh(double kh);

    void        addMetadata(const QString& name, const QString& comment);
    
    static double                             defaultValue();
    static bool                               isDefaultValue(double num);
    const std::vector<RigCompletionMetaData>& metadata() const;
    const QString&                            wellName() const;
    const RigCompletionDataGridCell&          completionDataGridCell() const;
    WellConnectionState                       connectionState() const;
    double                                    saturation() const;
    double                                    transmissibility() const;
    double                                    diameter() const; //TODO: should be ft or m
    double                                    kh() const;
    double                                    skinFactor() const;
    double                                    dFactor() const;
    CellDirection                             direction() const;
    size_t                                    count() const;
    double                                    wpimult() const;
    CompletionType                            completionType() const;
    bool                                      isMainBore() const;

    double                                  firstOrderingValue() const;
    double                                  secondOrderingValue() const;

    std::vector<RigCompletionMetaData>   m_metadata; 

private:
    QString                              m_wellName;
    RigCompletionDataGridCell            m_cellIndex;
    WellConnectionState                  m_connectionState;
    double                               m_saturation; //TODO: remove, always use default in Eclipse?
    double                               m_transmissibility;
    double                               m_diameter;
    double                               m_kh; //TODO: Remove, always use default in Eclipse?
    double                               m_skinFactor;
    double                               m_dFactor; //TODO: Remove, always use default in Eclipse?
    CellDirection                        m_direction;

    bool                                 m_isMainBore; //to use mainbore for Eclipse calculation

    size_t                               m_count; //TODO: Remove, usage replaced by WPImult
    double                               m_wpimult;

    CompletionType                       m_completionType;

    double                               m_firstOrderingValue;
    double                               m_secondOrderingValue;

private:
    static bool                          onlyOneIsDefaulted(double first, double second);
    static void                          copy(RigCompletionData& target, const RigCompletionData& from);
};
