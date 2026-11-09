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

#include "cafPdmPointer.h"

#include <QWidget>

#include <map>
#include <vector>

class RiaReportDataProducer;

class QComboBox;
class QLabel;
class QLineEdit;
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
/// Shows report data from a RiaReportDataProducer as tables, CSV or plain text, with one tab per
/// report section
//==================================================================================================
class RiuReportWindow : public QWidget
{
    Q_OBJECT

public:
    enum class DisplayMode
    {
        TABLE,
        CSV,
        TEXT
    };

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
    QWidget* createToolBar();
    void     rebuildContent();
    void     updateSourceLabel();
    void     selectProducerObject();

    QWidget* createSectionWidget( const RiaReportSection& section );
    QWidget* createTableWidget( const RiaReportTable& table, const QString& viewStateKey );
    void     saveTableViewStates();
    QWidget* createTextWidget( const RiaReportSection& section );

    void copyCurrentTab();
    void copyAllTabs();
    void copyToClipboard( const QString& title, const std::vector<RiaReportSection>& sections );
    void exportToFile();

    void onDisplayModeChanged();
    void onFormatOptionsChanged();
    void onCurrentTabChanged( int index );
    void loadAllSections();
    void onFilterTextChanged();

    static void showColumnMenu( QTableView* tableView, const QPoint& position );

    static void copySelectedCells( QTableView* tableView );

    DisplayMode                     displayMode() const;
    RifReportFormatOptions          formatOptions() const;
    RifReportTableFormatter::Format textFormat() const;
    RiaReportDataProducer*          producer() const;

    QString themeStyleSheet() const;

private:
    caf::PdmPointer<caf::PdmObjectHandle> m_producerObject;
    QString                               m_title;
    std::vector<RiaReportSection>         m_sections;
    bool                                  m_isRefreshPending;

    QLabel*         m_titleLabel;
    QLabel*         m_sourceLabel;
    QComboBox*      m_displayModeCombo;
    QComboBox*      m_numberFormatCombo;
    QSpinBox*       m_precisionSpinBox;
    QComboBox*      m_delimiterCombo;
    QLineEdit*      m_filterEdit;
    QToolButton*    m_copyButton;
    QToolButton*    m_exportButton;
    QToolButton*    m_refreshButton;
    QStackedWidget* m_stackedWidget;
    QLabel*         m_placeholderLabel;
    QTabWidget*     m_tabWidget;
    QTimer*         m_refreshTimer;
    QTimer*         m_filterTimer;

    // Sort order, column widths and hidden columns of the table views, restored when the views are recreated
    std::map<QString, QByteArray> m_tableViewStates;
};
