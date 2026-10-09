/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimWorkflowDefinition.h"

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QDateTime>
#include <QJsonObject>
#include <QMap>

#include <expected>
#include <memory>
#include <vector>

class RimWorkflowJob;
class QTemporaryDir;

//==================================================================================================
/// A taskmaestro workflow: a YAML folder, a workflow registered by an installed package, or a new
/// workflow that has not been saved yet. Editable workflows keep their definition in memory with
/// undo/redo until saved.
//==================================================================================================
class RimWorkflow : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class Source
    {
        YamlDirectory,
        Registered,
        Unsaved
    };

    struct RunSource
    {
        QStringList baseArgs{};
        QString     extraPythonPath{};
    };

    RimWorkflow();
    ~RimWorkflow() override;

    QString     name() const;
    void        setWorkflowDirectory( const QString& directory );
    QString     workflowDirectory() const;
    QJsonObject graph() const;
    QString     loadError() const;

    bool loadFromDirectory( QString* errorMessage = nullptr );
    bool loadFromRegistered( const QJsonObject& catalogEntry, QString* errorMessage = nullptr );
    void setUnsavedDefinition( const RimWorkflowDefinition& definition,
                               const QString&               sourceDirectory = {},
                               const QString&               targetDirectory = {} );

    Source                       source() const;
    QString                      registeredId() const;
    QString                      sourceDirectory() const;
    const RimWorkflowDefinition& definition() const;
    bool                         isEditable() const;
    QString                      readOnlyReason() const;
    bool                         isDirty() const;
    bool                         isLocked() const;
    int                          revision() const;

    // Editing
    std::expected<void, QString> applyEdit( const std::expected<RimWorkflowDefinition, QString>& result,
                                            const QMap<QString, QString>&                        renames = {} );
    bool                         canUndo() const;
    bool                         canRedo() const;
    void                         undo();
    void                         redo();

    // Saving and running
    std::expected<void, QString>      save();
    QStringList                       saveInPlaceWarnings() const;
    std::expected<void, QString>      saveAs( const QString& directory );
    std::expected<QString, QString>   writeEditDirectorySync();
    std::expected<QString, QString>   editDirectory();
    void                              setEditDirectoryRevision( int revision );
    QByteArray                        definitionJsonForSave() const;
    std::expected<RunSource, QString> prepareRunSource();
    QJsonObject                       inputValuesForSave() const;

    // Validation of the current revision (static checks plus `taskmaestro workflow describe`)
    void                          setValidationResult( int revision, const std::vector<RimWorkflowIssue>& issues, bool describeSucceeded );
    std::vector<RimWorkflowIssue> validationIssues() const;
    bool                          isValidationCurrent() const;
    bool                          lastValidationFailed() const;

    std::vector<RimWorkflowJob*> jobs() const;
    void                         addJob( RimWorkflowJob* job );
    void                         syncJobs( const QMap<QString, QString>& renames = {} );

    static QString findPythonExecutable();

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    struct UndoEntry
    {
        RimWorkflowDefinition  definition;
        QMap<QString, QString> renames;
    };

    bool setLoadError( const QString& message, QString* errorMessage );
    void setDefinition( const RimWorkflowDefinition& definition, Source source );
    void definitionChanged( const QMap<QString, QString>& renames );
    void notifyDefinitionChanged();
    void rebuildGraph();
    void updateUiName();
    void ensureDefaultJob();

    std::expected<QJsonObject, QString> saveToDirectory( const QString& directory, bool backup );

    void                          recordModificationTime();
    static QMap<QString, QString> invertedRenames( const QMap<QString, QString>& renames );

private:
    caf::PdmField<QString>                   m_name;
    caf::PdmField<QString>                   m_description;
    caf::PdmField<caf::FilePath>             m_workflowDirectory;
    caf::PdmField<QString>                   m_registeredId;
    caf::PdmField<bool>                      m_editable;
    caf::PdmField<QString>                   m_readOnlyReason;
    caf::PdmField<QString>                   m_loadError;
    caf::PdmChildArrayField<RimWorkflowJob*> m_jobs;
    QJsonObject                              m_graph;

    Source                         m_source = Source::YamlDirectory;
    QString                        m_sourceDirectory;
    RimWorkflowDefinition          m_definition;
    RimWorkflowDefinition          m_savedDefinition;
    int                            m_revision = 0;
    std::vector<UndoEntry>         m_undoStack;
    std::vector<UndoEntry>         m_redoStack;
    std::vector<RimWorkflowIssue>  m_validationIssues;
    int                            m_validationRevision = -1;
    bool                           m_describeSucceeded  = true;
    QDateTime                      m_loadedModificationTime;
    std::unique_ptr<QTemporaryDir> m_editDirectory;
    int                            m_editDirectoryRevision = -1;
};
