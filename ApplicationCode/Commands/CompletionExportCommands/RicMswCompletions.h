/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicMswSubSegment.h"

#include "RigCompletionData.h"

#include "RimWellPathAicdParameters.h"

#include "cvfBase.h"
#include "cvfMath.h"

#include <QString>
#include <memory>

class RimWellPathValve;

//==================================================================================================
/// 
//==================================================================================================
class RicMswCompletion
{
public:
    RicMswCompletion(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);

    virtual RigCompletionData::CompletionType    completionType() const = 0;

    const QString&                       label() const;
    size_t                               index() const;
    int                                  branchNumber() const;
    void                                 setBranchNumber(int branchNumber);

    void                                 addSubSegment(std::shared_ptr<RicMswSubSegment> subSegment);

    std::vector<std::shared_ptr<RicMswSubSegment>>&       subSegments();
    const std::vector<std::shared_ptr<RicMswSubSegment>>& subSegments() const;

    void                                 setLabel(const QString& label);

private:
    QString                              m_label;
    size_t                               m_index;
    int                                  m_branchNumber;

    std::vector<std::shared_ptr<RicMswSubSegment>> m_subSegments;
};

class RicMswFishbones : public RicMswCompletion
{
public:
    RicMswFishbones(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT)
        : RicMswCompletion(label, index, branchNumber)
    {}

    RigCompletionData::CompletionType completionType() const override { return RigCompletionData::FISHBONES; }
};

//==================================================================================================
///
//==================================================================================================
class RicMswFracture : public RicMswCompletion
{
public:
    RicMswFracture(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforation : public RicMswCompletion
{
public:
    RicMswPerforation(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswValve : public RicMswCompletion
{
public:
    RicMswValve(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    virtual ~RicMswValve() {}
};

//==================================================================================================
///
//==================================================================================================
class RicMswICD : public RicMswValve
{
public:
    RicMswICD(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    double flowCoefficient() const;
    double area() const;
    void   setFlowCoefficient(double icdFlowCoefficient);
    void   setArea(double icdArea);
private:
    double m_flowCoefficient;
    double m_area;
};

//==================================================================================================
///
//==================================================================================================
class RicMswFishbonesICD : public RicMswICD
{
public:
    RicMswFishbonesICD(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforationICD : public RicMswICD
{
public:
    RicMswPerforationICD(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforationAICD : public RicMswValve
{
public:
    RicMswPerforationAICD(const QString& label, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);
    RigCompletionData::CompletionType completionType() const override;

    bool   isValid() const;
    void   setIsValid(bool valid);
    bool   isOpen() const;
    void   setIsOpen(bool deviceOpen);
    double length() const;
    void   setLength(double length);

    const std::array<double, AICD_NUM_PARAMS>& values() const;
    std::array<double, AICD_NUM_PARAMS>&       values();

private:
    bool                                m_valid;
    bool                                m_deviceOpen;
    std::array<double, AICD_NUM_PARAMS> m_parameters;
    double                              m_length;
};