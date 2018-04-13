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

#include <vector>
#include <time.h>
#include <thread>

#define DEFAULT_DIALOG_WIDTH        550
#define DEFAULT_DIALOG_INIT_HEIGHT  150


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::RicSummaryCaseRestartDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    // Create widgets
    m_currentSummaryFileLayout = new QGridLayout();
    m_summaryReadAllBtn = new QRadioButton(this);
    m_summarySeparateCasesBtn = new QRadioButton(this);
    m_summaryNotReadBtn = new QRadioButton(this);
    m_gridSeparateCasesBtn = new QRadioButton(this);
    m_gridNotReadBtn = new QRadioButton(this);
    m_applyToAllCheckBox = new QCheckBox(this);

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
    m_applyToAllCheckBox->setText("Apply to All Files");

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGroupBox* currentFileGroup = new QGroupBox("Current Summary File");
    m_currentSummaryFileLayout = new QGridLayout();
    currentFileGroup->setLayout(m_currentSummaryFileLayout);

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
    buttonsLayout->addWidget(m_applyToAllCheckBox);
    buttonsLayout->addWidget(m_buttons);

    dialogLayout->addWidget(currentFileGroup);
    dialogLayout->addWidget(summaryFilesGroup);
    dialogLayout->addWidget(m_gridFilesGroup);
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
RicSummaryCaseRestartDialogResult RicSummaryCaseRestartDialog::openDialog(const QString& summaryHeaderFile,
                                                                          bool showApplyToAllWidget,
                                                                          bool buildGridCaseFileList,
                                                                          ImportOptions defaultSummaryImportOption,
                                                                          ImportOptions defaultGridImportOption,
                                                                          RicSummaryCaseRestartDialogResult *lastResult,
                                                                          QWidget *parent)
{
    RicSummaryCaseRestartDialog  dialog(parent);

    RifRestartFileInfo currentFileInfo = dialog.getFileInfo(summaryHeaderFile);
    if (!currentFileInfo.valid())
    {
        return RicSummaryCaseRestartDialogResult();
    }

    RifReaderEclipseSummary reader;
    std::vector<RifRestartFileInfo> originFileInfos = reader.getRestartFiles(summaryHeaderFile);

    // If no restart files are found, do not show dialog
    if (originFileInfos.empty())
    {
        QString gridCaseFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(summaryHeaderFile);
        return RicSummaryCaseRestartDialogResult(true, NOT_IMPORT, NOT_IMPORT, QStringList({ summaryHeaderFile }), QStringList({ gridCaseFile }), false);
    }

    RicSummaryCaseRestartDialogResult dialogResult;
    if (lastResult && lastResult->applyToAll)
    {
        dialogResult = *lastResult;
        dialogResult.summaryFiles.clear();
        dialogResult.gridFiles.clear();
    }
    else
    {
        dialog.setWindowTitle("Restart Files");

        dialog.appendFileInfoToGridLayout(*dialog.m_currentSummaryFileLayout, currentFileInfo);
        for (const auto& ofi : originFileInfos)
        {
            dialog.appendFileInfoToGridLayout(*dialog.m_summaryFilesLayout, ofi);
        }

        switch (defaultSummaryImportOption)
        {
        case ImportOptions::IMPORT_ALL:       dialog.m_summaryReadAllBtn->setChecked(true); break;
        case ImportOptions::SEPARATE_CASES:   dialog.m_summarySeparateCasesBtn->setChecked(true); break;
        case ImportOptions::NOT_IMPORT:       dialog.m_summaryNotReadBtn->setChecked(true); break;
        }

        dialog.m_gridFilesGroup->setVisible(buildGridCaseFileList);
        if (buildGridCaseFileList)
        {
            bool gridFilesAdded = false;
            for (const auto& ofi : originFileInfos)
            {
                QString gridFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(ofi.fileName);
                if (QFileInfo(gridFile).exists())
                {
                    dialog.appendFileInfoToGridLayout(*dialog.m_gridFilesLayout, RifRestartFileInfo(gridFile, ofi.startDate, ofi.endDate));
                    gridFilesAdded = true;
                }
            }
            if (!gridFilesAdded) dialog.m_gridFilesGroup->setVisible(false);

            switch (defaultGridImportOption)
            {
            case ImportOptions::SEPARATE_CASES:   dialog.m_gridSeparateCasesBtn->setChecked(true); break;
            case ImportOptions::NOT_IMPORT:       dialog.m_gridNotReadBtn->setChecked(true); break;
            }
        }
        dialog.m_applyToAllCheckBox->setVisible(showApplyToAllWidget);
        dialog.resize(DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_INIT_HEIGHT);
        dialog.exec();

        dialogResult = RicSummaryCaseRestartDialogResult(dialog.result() == QDialog::Accepted,
                                                         dialog.selectedSummaryImportOption(),
                                                         dialog.selectedGridImportOption(),
                                                         {},
                                                         {},
                                                         dialog.applyToAllSelected());
    }

    if (!dialogResult.ok)
    {
        return RicSummaryCaseRestartDialogResult(false, NOT_IMPORT, NOT_IMPORT, QStringList(), QStringList(), false);
    }

    dialogResult.summaryFiles.push_back(RiaFilePathTools::toInternalSeparator(summaryHeaderFile));
    if (dialogResult.summaryImportOption == SEPARATE_CASES)
    {
        for (const auto& ofi : originFileInfos)
        {
            dialogResult.summaryFiles.push_back(RiaFilePathTools::toInternalSeparator(ofi.fileName));
        }
    }

    if (buildGridCaseFileList)
    {
        QString gridCaseFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(summaryHeaderFile);
        dialogResult.gridFiles.push_back(gridCaseFile);

        if (dialogResult.gridImportOption == SEPARATE_CASES)
        {
            for (const auto& ofi : originFileInfos)
            {
                QString gridFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(ofi.fileName);
                if (buildGridCaseFileList && !gridCaseFile.isEmpty()) dialogResult.gridFiles.push_back(gridFile);
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
void RicSummaryCaseRestartDialog::appendFileInfoToGridLayout(QGridLayout& gridLayout, const RifRestartFileInfo& fileInfo)
{
    QDateTime startDate = QDateTime::fromTime_t(fileInfo.startDate);
    QString startDateString = startDate.toString(RimTools::dateFormatString());
    QDateTime endDate = QDateTime::fromTime_t(fileInfo.endDate);
    QString endDateString = endDate.toString(RimTools::dateFormatString());
    int rowCount = gridLayout.rowCount();

    QLabel* fileNameLabel = new QLabel();
    QLabel* dateLabel = new QLabel();
    fileNameLabel->setText(QFileInfo(fileInfo.fileName).fileName());
    dateLabel->setText(startDateString + " - " + endDateString);

    fileNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    gridLayout.addWidget(fileNameLabel, rowCount, 0);
    gridLayout.addWidget(dateLabel, rowCount, 1);

    // Full path in tooltip
    fileNameLabel->setToolTip(fileInfo.fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifRestartFileInfo> RicSummaryCaseRestartDialog::getRestartFiles(const QString& summaryHeaderFile)
{
    RifReaderEclipseSummary reader;
    return reader.getRestartFiles(summaryHeaderFile);
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

