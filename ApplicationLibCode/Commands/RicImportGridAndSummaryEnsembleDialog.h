/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Summary/RiaSummaryDefines.h"

#include <QDialog>
#include <QMap>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeView;

struct RicImportGridAndSummaryEnsembleDialogResult
{
    bool                             ok;
    QStringList                      gridFiles;
    QStringList                      summaryFiles;
    QString                          rootDir;
    QString                          pathFilter;
    RiaDefines::EnsembleGroupingMode groupingMode;
    bool                             createGridEnsemble;
    bool                             createSummaryEnsemble;
};

//==================================================================================================
///
//==================================================================================================
class RicImportGridAndSummaryEnsembleDialog : public QDialog
{
    Q_OBJECT

public:
    static RicImportGridAndSummaryEnsembleDialogResult runDialog( QWidget* parent, bool defaultGridChecked, bool defaultSummaryChecked );

private:
    explicit RicImportGridAndSummaryEnsembleDialog( QWidget* parent );

    QString cleanPathFilter() const;
    QString rootDirWithSeparator() const;
    QString pathFilterWithoutRoot() const;
    void    updateEffectiveFilter();
    void    setOkButtonEnabled( bool enabled );

    QStringList findMatchingFiles( const QStringList& extensions );

    void        updateFileListWidget();
    void        clearFileList();
    QStringList checkForMultipleFilenamesAndUncheckOutliers();

    RiaDefines::EnsembleGroupingMode ensembleGroupingMode() const;

private slots:
    void slotPathFilterChanged( const QString& text );
    void slotBrowseClicked();
    void slotUseRealizationStarClicked();
    void slotSearchClicked();
    void slotFilterTreeViewClicked();
    void slotOkClicked();
    void slotCancelClicked();
    void showEvent( QShowEvent* event ) override;

private:
    struct RealizationFiles
    {
        QString gridFile;
        QString summaryFile;
    };

    QComboBox*   m_pathFilterField;
    QPushButton* m_browseButton;
    QCheckBox*   m_useRealizationStarCheckBox;
    QComboBox*   m_ensembleGroupingMode;
    QLabel*      m_effectiveFilterLabel;
    QPushButton* m_searchButton;

    QCheckBox* m_createGridEnsembleCheckBox;
    QCheckBox* m_createSummaryEnsembleCheckBox;

    QGroupBox*   m_outputGroup;
    QLineEdit*   m_treeFilterLineEdit;
    QPushButton* m_treeFilterButton;
    QTreeView*   m_fileTreeView;

    QDialogButtonBox* m_buttons;

    QStandardItemModel m_filePathModel;

    QMap<QString, RealizationFiles> m_foundRealizations;

    bool m_blockItemUpdates;
};
