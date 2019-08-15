/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RiaEclipseUnitTools.h"
#include "RigCaseRealizationParameters.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <memory>

class RifReaderRftInterface;
class RifSummaryReaderInterface;
class RimSummaryCaseCollection;

//==================================================================================================
//
// 
//
//==================================================================================================

class RimSummaryCase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryCase();
    ~RimSummaryCase() override;
    
    virtual QString     summaryHeaderFilename() const; 
    virtual QString     caseName() const = 0; 
    QString             shortName() const;
    
    RiaEclipseUnitTools::UnitSystemType unitsSystem();

    void                updateAutoShortName();
    void                updateOptionSensitivity();

    virtual void        createSummaryReaderInterface() = 0;
    virtual RifSummaryReaderInterface* summaryReader() = 0;
    virtual RifReaderRftInterface* rftReader();
    virtual QString     errorMessagesFromReader()               { return QString(); }

    virtual void        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) = 0;

    void                setSummaryHeaderFileName(const QString& fileName);

    bool                isObservedData();

    void                setCaseRealizationParameters(const std::shared_ptr<RigCaseRealizationParameters>& crlParameters);
    std::shared_ptr<RigCaseRealizationParameters> caseRealizationParameters() const;
    bool                hasCaseRealizationParameters() const;
    RimSummaryCaseCollection* ensemble() const;
    void                copyFrom(const RimSummaryCase& rhs);
    bool                operator<(const RimSummaryCase& rhs) const;

protected:
    void        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                updateTreeItemName();

    caf::PdmField<QString>          m_shortName;
    caf::PdmField<bool>             m_useAutoShortName;
    caf::PdmField<QString>          m_summaryHeaderFilename;
    bool                            m_isObservedData;
    
    std::shared_ptr<RigCaseRealizationParameters> m_crlParameters;

private:
    void        initAfterRead() override;
};
