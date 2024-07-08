/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024    Equinor ASA
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

#include <QItemSelection>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrl>
#include <QWizard>

#include "RiaOsduConnector.h"

#include <set>

class QFile;
class QLabel;
class QTextEdit;
class QTableView;

class RimWellPathImport;

class OsduWellLogTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit OsduWellLogTableModel( QObject* parent = nullptr )
        : QAbstractTableModel( parent )
    {
    }

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override
    {
        Q_UNUSED( parent );
        return static_cast<int>( m_osduWellLogs.size() );
    }

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override
    {
        Q_UNUSED( parent );
        // Assuming you have three fields: id, kind, and name
        return 3;
    }

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override
    {
        if ( !index.isValid() ) return QVariant();

        if ( index.row() >= static_cast<int>( m_osduWellLogs.size() ) || index.row() < 0 ) return QVariant();

        if ( role == Qt::DisplayRole )
        {
            const OsduWellLog& field = m_osduWellLogs.at( index.row() );
            switch ( index.column() )
            {
                case 0:
                    return field.id;
                case 1:
                    return field.kind;
                case 2:
                    return field.name;
                default:
                    return QVariant();
            }
        }

        return QVariant();
    }

    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override
    {
        if ( role != Qt::DisplayRole ) return QVariant();

        if ( orientation == Qt::Horizontal )
        {
            switch ( section )
            {
                case 0:
                    return tr( "ID" );
                case 1:
                    return tr( "Kind" );
                case 2:
                    return tr( "Name" );
                default:
                    return QVariant();
            }
        }
        return QVariant();
    }

    void setOsduWellLogs( const std::vector<OsduWellLog>& osduWellLogs )
    {
        beginResetModel();
        m_osduWellLogs.clear();
        for ( auto v : osduWellLogs )
            m_osduWellLogs.push_back( v );

        endResetModel();
    }

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override
    {
        std::sort( m_osduWellLogs.begin(),
                   m_osduWellLogs.end(),
                   [column, order]( const OsduWellLog& a, const OsduWellLog& b )
                   {
                       switch ( column )
                       {
                           case 0:
                               return ( order == Qt::AscendingOrder ) ? a.id < b.id : a.id > b.id;
                           case 1:
                               return ( order == Qt::AscendingOrder ) ? a.kind < b.kind : a.kind > b.kind;
                           case 2:
                               return ( order == Qt::AscendingOrder ) ? a.name < b.name : a.name > b.name;
                           default:
                               return false;
                       }
                   } );
        emit dataChanged( index( 0, 0 ), index( rowCount() - 1, columnCount() - 1 ) );
        emit layoutChanged();
    }

private:
    std::vector<OsduWellLog> m_osduWellLogs;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class WellLogAuthenticationPage : public QWizardPage
{
    Q_OBJECT

public:
    WellLogAuthenticationPage( RiaOsduConnector* osduConnector, QWidget* parent = nullptr );

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void accessOk();

private:
    RiaOsduConnector* m_osduConnector;
    QLabel*           m_connectionLabel;
    bool              m_accessOk;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class WellLogSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    WellLogSelectionPage( RiaOsduConnector* m_osduConnector, QWidget* parent = nullptr );
    ~WellLogSelectionPage() override;

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void wellLogsFinished( const QString& wellboreId );
    void selectWellLogs( const QItemSelection& newSelection, const QItemSelection& oldSelection );

private:
    RiaOsduConnector* m_osduConnector;

    QTableView*            m_tableView;
    OsduWellLogTableModel* m_osduWellLogsModel;
    QSortFilterProxyModel* m_proxyModel;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuWellLogImportWizard : public QWizard
{
    Q_OBJECT

public:
    struct WellLogInfo
    {
        QString name;
        QString wellLog;
    };

    RiuWellLogImportWizard( RiaOsduConnector* osduConnector, const QString& wellboreId, QWidget* parent = nullptr );
    ~RiuWellLogImportWizard() override;

    void                 setSelectedWellLogs( const std::vector<QString>& wellLogIds );
    std::vector<QString> selectedWellLogs() const;

    void                                             addWellLogInfo( RiuWellLogImportWizard::WellLogInfo wellLogInfo );
    std::vector<RiuWellLogImportWizard::WellLogInfo> importedWellLogs() const;

    QString wellboreId() const;

public slots:
    void downloadWellLogs( const QString& wellboreId );

private:
    RiaOsduConnector*    m_osduConnector;
    std::vector<QString> m_selectedWellLogs;

    QString                                          m_wellboreId;
    std::vector<RiuWellLogImportWizard::WellLogInfo> m_wellLogInfos;
};
