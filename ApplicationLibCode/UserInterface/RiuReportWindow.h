/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifReportTableFormatter.h"

#include "RimPlotAxisProperties.h"

#include "cafPdmPointer.h"

#include <QWidget>

#include <map>
#include <vector>

class RiaReportDataProducer;

class QComboBox;
class QLabel;
class QSpinBox;
class QStackedWidget;
class QTabWidget;
class QTableView;
class QTimer;
class QToolButton;

namespace caf
{
class PdmObjectHandle;
}

//==================================================================================================
/// Shows report data from a RiaReportDataProducer as tables, with one tab per report section. The
/// data can be copied or exported as CSV or plain text.
//==================================================================================================
class RiuReportWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RiuReportWindow( QWidget* parent = nullptr );

    // Returns the object itself or the first ancestor implementing RiaReportDataProducer
    static caf::PdmObjectHandle* findProducerObject( caf::PdmObjectHandle* object );

    void                  setProducerObject( caf::PdmObjectHandle* producerObject );
    caf::PdmObjectHandle* producerObject() const;

    // Sets the producer and reads the data immediately
    void showProducerObject( caf::PdmObjectHandle* producerObject );

    void refresh();

    // Refreshes the window if it is visible and shows data for the given object
    void scheduleRefreshIfShowing( const caf::PdmObjectHandle* object );

protected:
    void showEvent( QShowEvent* event ) override;

private:
    QWidget* createBottomBar();
    void     rebuildContent();
    void     updateSourceLabel();
    void     selectProducerObject();

    QWidget* createSectionWidget( const RiaReportSection& section );
    QWidget* createTableWidget( const RiaReportTable& table, const QString& viewStateKey );
    void     saveTableViewStates();

    void copyCurrentTab();
    void copyToClipboard( const std::vector<RiaReportSection>& sections );
    void exportToFile();

    void onOutputFormatChanged();
    void onFormatOptionsChanged();
    void loadSettings();
    void saveSettings();
    void onCurrentTabChanged( int index );
    void loadAllSections();

    static void showColumnMenu( QTableView* tableView, const QPoint& position );

    static void copySelectedCells( QTableView* tableView );

    RifReportFormatOptions                  formatOptions() const;
    RimPlotAxisProperties::NumberFormatType numberFormat() const;
    RifReportTableFormatter::Format         textFormat() const;
    RiaReportDataProducer*                  producer() const;

    QString themeStyleSheet() const;

private:
    caf::PdmPointer<caf::PdmObjectHandle> m_producerObject;
    QString                               m_title;
    std::vector<RiaReportSection>         m_sections;
    bool                                  m_isRefreshPending;

    QLabel*         m_titleLabel;
    QLabel*         m_sourceLabel;
    QComboBox*      m_outputFormatCombo;
    QComboBox*      m_numberFormatCombo;
    QSpinBox*       m_precisionSpinBox;
    QComboBox*      m_delimiterCombo;
    QToolButton*    m_copyButton;
    QToolButton*    m_exportButton;
    QStackedWidget* m_stackedWidget;
    QLabel*         m_placeholderLabel;
    QTabWidget*     m_tabWidget;
    QTimer*         m_refreshTimer;

    // Sort order, column widths and hidden columns of the table views, restored when the views are recreated
    std::map<QString, QByteArray> m_tableViewStates;
};
