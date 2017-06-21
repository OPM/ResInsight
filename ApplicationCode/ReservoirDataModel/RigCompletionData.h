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

#include "cvfBase.h"
#include "cvfObject.h"

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
class IJKCellIndex {
public:
    IJKCellIndex() {};
    IJKCellIndex(size_t i, size_t j, size_t k) : i(i), j(j), k(k) {};
    IJKCellIndex(const IJKCellIndex& other)
    {
        i = other.i;
        j = other.j;
        k = other.k;
    }

    bool operator==(const IJKCellIndex& other) const
    {
        return i == other.i && j == other.j && k == other.k;
    }

    bool operator<(const IJKCellIndex& other) const
    {
        if (i != other.i) return i < other.i;
        if (j != other.j) return j < other.j;
        if (k != other.k) return k < other.k;
        return false;
    }

    size_t i;
    size_t j;
    size_t k;
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
class RigCompletionData : public cvf::Object
{
public:
    enum CompletionType {
        FISHBONES,
        FRACTURE,
        PERFORATION,
    };

    RigCompletionData(const QString wellName, const IJKCellIndex& cellIndex);
    ~RigCompletionData();
    RigCompletionData(const RigCompletionData& other);

    static RigCompletionData   combine(const std::vector<RigCompletionData>& completions);

    bool operator<(const RigCompletionData& other) const;
    RigCompletionData& operator=(const RigCompletionData& other);

    void                                 setFromFracture(double transmissibility, double skinFactor);
   
    void setTransAndWPImultBackgroundDataFromFishbone(double transmissibility, 
                                                      double skinFactor, 
                                                      double diameter, 
                                                      CellDirection direction,
                                                      bool isMainBore);

    void setTransAndWPImultBackgroundDataFromPerforation(double transmissibility, 
                                                         double skinFactor, 
                                                         double diameter, 
                                                         CellDirection direction);

    void                                 setCombinedValuesExplicitTrans(double transmissibility,
                                                                        CompletionType completionType);
    void                                 setCombinedValuesImplicitTransWPImult(double wpimult, 
                                                                               CellDirection celldirection, 
                                                                               double skinFactor, 
                                                                               double wellDiameter, 
                                                                               CompletionType completionType);

    void                                 addMetadata(const QString& name, const QString& comment);
    static bool                          isDefaultValue(double val);

    const std::vector<RigCompletionMetaData>& metadata() const { return m_metadata; }
    const QString&                            wellName() const { return m_wellName; }
    const IJKCellIndex&                       cellIndex() const { return m_cellIndex; }
    WellConnectionState                       connectionState() const { return m_connectionState; }
    double                                    saturation() const { return m_saturation; }
    double                                    transmissibility() const { return m_transmissibility; }
    double                                    diameter() const { return m_diameter; } //TODO: should be ft or m
    double                                    kh() const { return m_kh; }
    double                                    skinFactor() const { return m_skinFactor; }
    double                                    dFactor() const { return m_dFactor; }
    CellDirection                             direction() const { return m_direction; }
    size_t                                    count() const { return m_count; }
    double                                    wpimult() const { return m_wpimult; }
    CompletionType                            completionType() const { return m_completionType; }
    bool                                      isMainBore() const { return m_isMainBore; }
    bool                                      readyForExport() const { return m_readyForExport; }

    std::vector<RigCompletionMetaData>   m_metadata; 

private:
    QString                              m_wellName;
    IJKCellIndex                         m_cellIndex;
    WellConnectionState                  m_connectionState;
    double                               m_saturation; //TODO: remove, always use default in Eclipse?
    double                               m_transmissibility;
    double                               m_diameter;
    double                               m_kh; //TODO: Remove, always use default in Eclipse?
    double                               m_skinFactor;
    double                               m_dFactor; //TODO: Remove, always use default in Eclipse?
    CellDirection                        m_direction;

    bool                                 m_isMainBore; //to use mainbore for Eclipse calculation
    bool                                 m_readyForExport;

    size_t                               m_count; //TODO: Remove, usage replaced by WPImult
    double                               m_wpimult;

    CompletionType                       m_completionType;

private:
    static bool                          onlyOneIsDefaulted(double first, double second);
    static void                          copy(RigCompletionData& target, const RigCompletionData& from);
};
