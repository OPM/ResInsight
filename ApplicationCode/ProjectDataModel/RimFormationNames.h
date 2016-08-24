/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cvfObject.h"

class RigFormationNames;

class RimFormationNames : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFormationNames();
    ~RimFormationNames();

    void                        setFileName(const QString& fileName);
    const QString&              fileName();

    RigFormationNames*          formationNamesData() { return m_formationNamesData.p();}

    void                        readFormationNamesFile(QString * errorMessage);
    void                        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

protected:
    virtual void                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                initAfterRead() override;

private:
    void                        updateUiTreeName();

    caf::PdmField<QString>      m_formationNamesFileName;

    cvf::ref<RigFormationNames> m_formationNamesData;
};

class RigFormationNames: public cvf::Object
{
public:
    int formationIndexFromKLayerIdx(size_t Kidx) 
    {
        if(Kidx >= m_nameIndexPrKLayer.size()) return -1;
        return m_nameIndexPrKLayer[Kidx];
    }

    QString formationNameFromKLayerIdx(size_t Kidx)
    {
        int idx = formationIndexFromKLayerIdx(Kidx);
        if (idx >= m_formationNames.size()) return "";
        if (idx == -1) return "";

        return m_formationNames[idx];
    }

    void appendFormationRange(const QString& name, int kStartIdx, int kEndIdx)
    {
        CVF_ASSERT(kStartIdx <= kEndIdx);
        int nameIdx = static_cast<int>(m_formationNames.size());
        m_formationNames.push_back(name);
        if (kEndIdx >= static_cast<int>(m_nameIndexPrKLayer.size())) m_nameIndexPrKLayer.resize(kEndIdx + 1, -1);

        for (int kIdx = kStartIdx; kIdx <= kEndIdx; ++kIdx)
        {
            m_nameIndexPrKLayer[kIdx] = nameIdx;
        }
    }

private:

    std::vector<int> m_nameIndexPrKLayer;
    std::vector<QString> m_formationNames;
};
