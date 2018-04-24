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

#include "RicSummaryCaseRestartDialog.h"
#include "ExportCommands/RicSnapshotViewToClipboardFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "ExportCommands/RicSnapshotFilenameGenerator.h"

#include "RiaApplication.h"
#include "RiaFilePathTools.h"
#include "RiaLogging.h"

#include "RifReaderEclipseSummary.h"
#include "RifEclipseSummaryTools.h"

#include "RimEclipseView.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimTools.h"

#include "RiuMainPlotWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTools.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QGroupBox>
#include <QListWidget>
#include <QAbstractItemView>
#include <QMenu>
#include <QDateTime>

#include <cvfAssert.h>

#include <vector>
#include <time.h>
#include <thread>

#define DEFAULT_DIALOG_WIDTH        550
#define DEFAULT_DIALOG_INIT_HEIGHT  150


//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RifRestartFileInfo>> removeRootPath(const std::vector<std::vector<RifRestartFileInfo>>& fileInfoLists)
{
    // Find common root path among all paths
    QStringList allPaths;
    for (const auto& fileInfoList : fileInfoLists)
    {
        for (const auto fi : fileInfoList) allPaths.push_back(fi.fileName);
    }
    QString commonRoot = RiaFilePathTools::commonRootPath(allPaths);
    int commonRootSize = commonRoot.size();

    // Build output lists
    std::vector<std::vector<RifRestartFileInfo>> output;
    for (const auto& fileInfoList : fileInfoLists)
    {
        std::vector<RifRestartFileInfo> currList;

        for (auto& fi : fileInfoList)
        {
            RifRestartFileInfo newFi = fi;
            newFi.fileName.remove(0, commonRootSize);
            currList.push_back(newFi);
        }
        output.push_back(currList);
    }
    return output;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::RicSummaryCaseRestartDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    // Create widgets
    m_currentFilesLayout = new QGridLayout();
    m_summaryReadAllBtn = new QRadioButton(this);
    m_summarySeparateCasesBtn = new QRadioButton(this);
    m_summaryNotReadBtn = new QRadioButton(this);
    m_gridSeparateCasesBtn = new QRadioButton(this);
    m_gridNotReadBtn = new QRadioButton(this);
    m_applyToAllCheckBox = new QCheckBox(this);
    m_warnings = new QListWidget(this);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to signals
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(slotDialogOkClicked()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogCancelClicked()));

    // Set widget properties
    m_summaryReadAllBtn->setText("Unified");
    m_summarySeparateCasesBtn->setText("Separate Cases");
    m_summaryNotReadBtn->setText("Skip");
    m_gridSeparateCasesBtn->setText("Separate Cases");
    m_gridNotReadBtn->setText("Skip");
    m_applyToAllCheckBox->setText("Apply Settings to Remaining Files");
    m_applyToAllCheckBox->setLayoutDirection(Qt::RightToLeft);

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    m_currentFilesGroup = new QGroupBox("Current Summary File");
    m_currentFilesLayout = new QGridLayout();
    m_currentFilesGroup->setLayout(m_currentFilesLayout);

    // Summary files
    QGroupBox* summaryFilesGroup = new QGroupBox("Found Origin Summary Files");
    {
        QVBoxLayout* filesGroupLayout = new QVBoxLayout();
        summaryFilesGroup->setLayout(filesGroupLayout);

        m_summaryFilesLayout = new QGridLayout();
        filesGroupLayout->addLayout(m_summaryFilesLayout);
        m_summaryFilesLayout->setContentsMargins(0, 0, 0, 20);

        QGroupBox* optionsGroup = new QGroupBox("Import Options");
        QVBoxLayout* optionsLayout = new QVBoxLayout();
        optionsGroup->setLayout(optionsLayout);
        optionsLayout->addWidget(m_summaryReadAllBtn);
        optionsLayout->addWidget(m_summarySeparateCasesBtn);
        optionsLayout->addWidget(m_summaryNotReadBtn);
        filesGroupLayout->addWidget(optionsGroup);
    }

    // Grid files
    m_gridFilesGroup = new QGroupBox("Found Origin Grid Files");
    {
        QVBoxLayout* filesGroupLayout = new QVBoxLayout();
        m_gridFilesGroup->setLayout(filesGroupLayout);

        m_gridFilesLayout = new QGridLayout();
        filesGroupLayout->addLayout(m_gridFilesLayout);
        m_gridFilesLayout->setContentsMargins(0, 0, 0, 20);

        QGroupBox* optionsGroup = new QGroupBox("Import Options");
        QVBoxLayout* optionsLayout = new QVBoxLayout();
        optionsGroup->setLayout(optionsLayout);
        optionsLayout->addWidget(m_gridSeparateCasesBtn);
        optionsLayout->addWidget(m_gridNotReadBtn);
        filesGroupLayout->addWidget(optionsGroup);
    }

    // Apply to all checkbox and buttons
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(m_applyToAllCheckBox);
    buttonsLayout->addWidget(m_buttons);

    dialogLayout->addWidget(m_currentFilesGroup);
    dialogLayout->addWidget(summaryFilesGroup);
    dialogLayout->addWidget(m_gridFilesGroup);
    dialogLayout->addWidget(m_warnings);
    dialogLayout->addLayout(buttonsLayout);

    setLayout(dialogLayout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::~RicSummaryCaseRestartDialog()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialogResult RicSummaryCaseRestartDialog::openDialog(const std::pair<QString /*sum*/, QString /*grid*/>& initialFiles,
                                                                          bool showApplyToAllWidget,
                                                                          ImportOptions defaultSummaryImportOption,
                                                                          ImportOptions defaultGridImportOption,
                                                                          RicSummaryCaseRestartDialogResult *lastResult,
                                                                          QWidget *parent)
{
    RicSummaryCaseRestartDialog  dialog(parent);
    QString initialSummaryFile = initialFiles.first;
    QString initialGridFile = initialFiles.second;
    bool handleGridFile = !initialGridFile.isEmpty();

    // If only grid file is present, return
    if (initialSummaryFile.isEmpty() && !initialGridFile.isEmpty())
    {
        return RicSummaryCaseRestartDialogResult(RicSummaryCaseRestartDialogResult::OK,
                                                 defaultSummaryImportOption,
                                                 defaultGridImportOption,
                                                 {},
                                                 QStringList({ initialGridFile }),
                                                 lastResult && lastResult->applyToAll);
    }

    RifRestartFileInfo currentFileInfo = dialog.getFileInfo(initialSummaryFile);
    if (!currentFileInfo.valid())
    {
        return RicSummaryCaseRestartDialogResult();
    }

    RifReaderEclipseSummary reader;
    bool hasWarnings = false;
    std::vector<RifRestartFileInfo> originFileInfos = reader.getRestartFiles(initialSummaryFile, &hasWarnings);

    // If no restart files are found and no warnings, do not show dialog
    if (originFileInfos.empty() &&!hasWarnings)
    {
        return RicSummaryCaseRestartDialogResult(RicSummaryCaseRestartDialogResult::OK, NOT_IMPORT, NOT_IMPORT, QStringList({ initialSummaryFile }), QStringList({ initialGridFile }), false);
    }

    RicSummaryCaseRestartDialogResult dialogResult;
    if (lastResult && lastResult->applyToAll)
    {
        dialogResult = *lastResult;
        dialogResult.summaryFiles.clear();
        dialogResult.gridFiles.clear();

        if (hasWarnings)
        {
            for (const QString& warning : reader.warnings()) RiaLogging::error(warning);
        }
    }
    else
    {
        std::vector<RifRestartFileInfo> currentFileInfos;
        std::vector<RifRestartFileInfo> originSummaryFileInfos;
        std::vector<RifRestartFileInfo> originGridFileInfos;

        // Grid file
        if (handleGridFile)
        {
            dialog.m_currentFilesGroup->setTitle("Current Grid and Summary Files");
            currentFileInfos.push_back(RifRestartFileInfo(initialGridFile, currentFileInfo.startDate, currentFileInfo.endDate));

            for (const auto& ofi : originFileInfos)
            {
                QString gridFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(ofi.fileName);
                if (QFileInfo(gridFile).exists())
                {
                    originGridFileInfos.push_back(RifRestartFileInfo(gridFile, ofi.startDate, ofi.endDate));
                }
            }
        }

        currentFileInfos.push_back(currentFileInfo);
        for (const auto& ofi : originFileInfos)
        {
            originSummaryFileInfos.push_back(ofi);
        }

        // Set default import options
        switch (defaultSummaryImportOption)
        {
        case ImportOptions::IMPORT_ALL:       dialog.m_summaryReadAllBtn->setChecked(true); break;
        case ImportOptions::SEPARATE_CASES:   dialog.m_summarySeparateCasesBtn->setChecked(true); break;
        case ImportOptions::NOT_IMPORT:       dialog.m_summaryNotReadBtn->setChecked(true); break;
        }

        if (handleGridFile)
        {
            switch (defaultGridImportOption)
            {
            case ImportOptions::SEPARATE_CASES:   dialog.m_gridSeparateCasesBtn->setChecked(true); break;
            case ImportOptions::NOT_IMPORT:       dialog.m_gridNotReadBtn->setChecked(true); break;
            }
        }

        // Remove common root path
        std::vector<std::vector<RifRestartFileInfo>> fileInfosNoRoot = removeRootPath(
            {
                currentFileInfos, originSummaryFileInfos, originGridFileInfos
            }
        );

        // Populate file list widgets
        dialog.populateFileList(dialog.m_currentFilesLayout, fileInfosNoRoot[0]);
        dialog.populateFileList(dialog.m_summaryFilesLayout, fileInfosNoRoot[1]);
        dialog.populateFileList(dialog.m_gridFilesLayout, fileInfosNoRoot[2]);

        // Display warnings if any
        dialog.displayWarningsIfAny(reader.warnings());

        // Set properties and show dialog
        dialog.setWindowTitle("Restart Files");
        dialog.m_applyToAllCheckBox->setVisible(showApplyToAllWidget);
        dialog.resize(DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_INIT_HEIGHT);
        dialog.exec();

        RicSummaryCaseRestartDialogResult::Status status = RicSummaryCaseRestartDialogResult::OK;
        if (dialog.result() == QDialog::Rejected)
        {
            status = RicSummaryCaseRestartDialogResult::CANCELLED;
        }

        dialogResult = RicSummaryCaseRestartDialogResult(status,
                                                         dialog.selectedSummaryImportOption(),
                                                         dialog.selectedGridImportOption(),
                                                         {},
                                                         {},
                                                         dialog.applyToAllSelected());
    }

    if (dialogResult.status != RicSummaryCaseRestartDialogResult::OK)
    {
        return RicSummaryCaseRestartDialogResult(dialogResult.status, NOT_IMPORT, NOT_IMPORT, QStringList(), QStringList(), false);
    }

    dialogResult.summaryFiles.push_back(RiaFilePathTools::toInternalSeparator(initialSummaryFile));
    if (dialogResult.summaryImportOption == SEPARATE_CASES)
    {
        for (const auto& ofi : originFileInfos)
        {
            dialogResult.summaryFiles.push_back(RiaFilePathTools::toInternalSeparator(ofi.fileName));
        }
    }

    if (handleGridFile)
    {
        dialogResult.gridFiles.push_back(initialGridFile);

        if (dialogResult.gridImportOption == SEPARATE_CASES)
        {
            for (const auto& ofi : originFileInfos)
            {
                QString gridFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(ofi.fileName);
                if (handleGridFile) dialogResult.gridFiles.push_back(gridFile);
            }
        }
    }
    return dialogResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::ImportOptions RicSummaryCaseRestartDialog::selectedSummaryImportOption() const
{
    return
        m_summaryReadAllBtn->isChecked() ?          IMPORT_ALL :
        m_summarySeparateCasesBtn->isChecked() ?    SEPARATE_CASES :
                                                    NOT_IMPORT;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::ImportOptions RicSummaryCaseRestartDialog::selectedGridImportOption() const
{
    return
        m_gridSeparateCasesBtn->isChecked() ? SEPARATE_CASES : NOT_IMPORT;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCaseRestartDialog::applyToAllSelected() const
{
    return m_applyToAllCheckBox->isChecked();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::populateFileList(QGridLayout* gridLayout, const std::vector<RifRestartFileInfo>& fileInfos)
{
    if (fileInfos.empty())
    {
        QWidget* parent = gridLayout->parentWidget();
        if (parent) parent->setVisible(false);
    }

    for (const auto& fileInfo : fileInfos)
    {
        appendFileInfoToGridLayout(gridLayout, fileInfo);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::appendFileInfoToGridLayout(QGridLayout* gridLayout, const RifRestartFileInfo& fileInfo)
{
    CVF_ASSERT(gridLayout);

    QDateTime startDate = QDateTime::fromTime_t(fileInfo.startDate);
    QString startDateString = startDate.toString(RimTools::dateFormatString());
    QDateTime endDate = QDateTime::fromTime_t(fileInfo.endDate);
    QString endDateString = endDate.toString(RimTools::dateFormatString());
    int rowCount = gridLayout->rowCount();

    QLabel* fileNameLabel = new QLabel();
    QLabel* dateLabel = new QLabel();
    fileNameLabel->setText(fileInfo.fileName);
    dateLabel->setText(startDateString + " - " + endDateString);

    fileNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    gridLayout->addWidget(fileNameLabel, rowCount, 0);
    gridLayout->addWidget(dateLabel, rowCount, 1);

    // Full path in tooltip
    fileNameLabel->setToolTip(fileInfo.fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RicSummaryCaseRestartDialog::getFileInfo(const QString& summaryHeaderFile)
{
    RifReaderEclipseSummary reader;
    return reader.getFileInfo(summaryHeaderFile);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::displayWarningsIfAny(const QStringList& warnings)
{
    m_warnings->setVisible(!warnings.isEmpty());
    for (const auto& warning : warnings)
    {
        QListWidgetItem* item = new QListWidgetItem(warning, m_warnings);
        item->setForeground(Qt::red);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotDialogOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotDialogCancelClicked()
{
    reject();
}

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

