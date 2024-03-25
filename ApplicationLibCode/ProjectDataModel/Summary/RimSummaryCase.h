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

#include "RiaDefines.h"
#include "RigCaseRealizationParameters.h"

#include "RimCaseDisplayNameTools.h"

#include "cafFilePath.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <memory>

class RifReaderRftInterface;
class RifSummaryReaderInterface;
class RimSummaryCaseCollection;
class RimSummaryAddressCollection;

//==================================================================================================
//
//
//
//==================================================================================================

class RimSummaryCase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> nameChanged;

    using DisplayNameEnum = caf::AppEnum<RimCaseDisplayNameTools::DisplayName>;

public:
    RimSummaryCase();
    ~RimSummaryCase() override;

    virtual QString                      summaryHeaderFilename() const;
    QString                              displayCaseName() const;
    QString                              nativeCaseName() const;
    void                                 setCustomCaseName( const QString& caseName );
    void                                 updateAutoShortName();
    RimCaseDisplayNameTools::DisplayName displayNameType() const;
    void                                 setDisplayNameOption( RimCaseDisplayNameTools::DisplayName displayNameOption );

    void setCaseId( int caseId );
    int  caseId() const;

    void refreshMetaData();
    void onCalculationUpdated();

    virtual void                       createSummaryReaderInterface() = 0;
    virtual void                       createRftReaderInterface() {}
    virtual RifSummaryReaderInterface* summaryReader() = 0;
    virtual RifReaderRftInterface*     rftReader();
    virtual QString                    errorMessagesFromReader();

    void setSummaryHeaderFileName( const QString& fileName );

    caf::AppEnum<RiaDefines::EclipseUnitSystem> unitsSystem();

    bool isObservedData() const;

    bool showRealizationDataSources() const;
    void setShowRealizationDataSource( bool enable );

    void setCaseRealizationParameters( const std::shared_ptr<RigCaseRealizationParameters>& crlParameters );
    std::shared_ptr<RigCaseRealizationParameters> caseRealizationParameters() const;
    bool                                          hasCaseRealizationParameters() const;
    RimSummaryCaseCollection*                     ensemble() const;
    void                                          copyFrom( const RimSummaryCase& rhs );
    bool                                          operator<( const RimSummaryCase& rhs ) const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    caf::PdmFieldHandle* userDescriptionField() override;

    void updateTreeItemName();

    virtual QString caseName() const = 0;

    void initAfterRead() override;

private:
    void buildChildNodes();

protected:
    caf::PdmField<QString>         m_displayName;
    caf::PdmField<DisplayNameEnum> m_displayNameOption;
    caf::PdmField<caf::FilePath>   m_summaryHeaderFilename;

    caf::PdmField<bool> m_showSubNodesInTree;

    caf::PdmChildField<RimSummaryAddressCollection*> m_dataVectorFolders;

    bool               m_isObservedData;
    caf::PdmField<int> m_caseId;

    std::shared_ptr<RigCaseRealizationParameters> m_crlParameters;

    caf::PdmField<bool> m_useAutoShortName_OBSOLETE;

    static const QString DEFAULT_DISPLAY_NAME;
};
