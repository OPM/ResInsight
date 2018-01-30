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

#include <QString>

#include <vector>

class RimEclipseCase;

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

    IJKCellIndex(size_t globalCellIndex, const RimEclipseCase* eclipseCase);

    bool operator==(const IJKCellIndex& other) const
    {
        return m_localCellIndexI == other.m_localCellIndexI && m_localCellIndexJ == other.m_localCellIndexJ && m_localCellIndexK == other.m_localCellIndexK;
    }

    bool operator<(const IJKCellIndex& other) const
    {
        if (m_localCellIndexI != other.m_localCellIndexI) return m_localCellIndexI < other.m_localCellIndexI;
        if (m_localCellIndexJ != other.m_localCellIndexJ) return m_localCellIndexJ < other.m_localCellIndexJ;
        if (m_localCellIndexK != other.m_localCellIndexK) return m_localCellIndexK < other.m_localCellIndexK;
        return false;
    }

    size_t globalCellIndex() const;

    size_t localCellIndexI() const;
    size_t localCellIndexJ() const;
    size_t localCellIndexK() const;

    QString oneBasedLocalCellIndexString() const;

    QString lgrName() const;

private:
    size_t m_globalCellIndex;
    QString m_lgrName;

    size_t m_localCellIndexI;
    size_t m_localCellIndexJ;
    size_t m_localCellIndexK;
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
        CT_UNDEFINED
    };

    RigCompletionData(const QString wellName, const IJKCellIndex& cellIndex);
    ~RigCompletionData();
    RigCompletionData(const RigCompletionData& other);

    static RigCompletionData   combine(const std::vector<RigCompletionData>& completions);

    bool operator<(const RigCompletionData& other) const;
    RigCompletionData& operator=(const RigCompletionData& other);

    void        setFromFracture(double transmissibility, double skinFactor);
    
    void        setTransAndWPImultBackgroundDataFromFishbone(double transmissibility, 
                                                             double skinFactor, 
                                                             double diameter,
                                                             CellDirection direction, 
                                                             bool isMainBore);

    void        setTransAndWPImultBackgroundDataFromPerforation(double transmissibility, 
                                                                double skinFactor, 
                                                                double diameter,
                                                                CellDirection direction);

    void        setCombinedValuesExplicitTrans(double transmissibility, CompletionType completionType);
    
    void        setCombinedValuesImplicitTransWPImult(double wpimult, 
                                                      CellDirection celldirection, 
                                                      double skinFactor,
                                                      double wellDiameter, 
                                                      CompletionType completionType);

    void        addMetadata(const QString& name, const QString& comment);
    static bool isDefaultValue(double val);

    const std::vector<RigCompletionMetaData>& metadata() const;
    const QString&                            wellName() const;
    const IJKCellIndex&                       cellIndex() const;
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
    bool                                      readyForExport() const;

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
