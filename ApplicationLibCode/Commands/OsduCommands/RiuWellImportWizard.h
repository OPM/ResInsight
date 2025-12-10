/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "Cloud/RiaOsduConnector.h"

#include <set>

class QFile;
class QLabel;
class QTextEdit;
class QTableView;
class OsduFieldTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit OsduFieldTableModel( QObject* parent = nullptr )
        : QAbstractTableModel( parent )
    {
    }

    enum Column
    {
        Name,
        Kind,
        Id,
        ColumnCount
    };

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override
    {
        Q_UNUSED( parent );
        return static_cast<int>( m_osduFields.size() );
    }

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override
    {
        Q_UNUSED( parent );
        return ColumnCount;
    }

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override
    {
        if ( !index.isValid() ) return QVariant();

        if ( index.row() >= static_cast<int>( m_osduFields.size() ) || index.row() < 0 ) return QVariant();

        if ( role == Qt::DisplayRole )
        {
            const OsduField& field = m_osduFields.at( index.row() );
            switch ( static_cast<Column>( index.column() ) )
            {
                case Id:
                    return field.id;
                case Kind:
                    return field.kind;
                case Name:
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
            switch ( static_cast<Column>( section ) )
            {
                case Id:
                    return tr( "ID" );
                case Kind:
                    return tr( "Kind" );
                case Name:
                    return tr( "Name" );
                default:
                    return QVariant();
            }
        }
        return QVariant();
    }

    void setOsduFields( const std::vector<OsduField>& osduFields )
    {
        beginResetModel();
        m_osduFields = osduFields;
        endResetModel();
    }

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override
    {
        std::sort( m_osduFields.begin(),
                   m_osduFields.end(),
                   [column, order]( const OsduField& a, const OsduField& b )
                   {
                       switch ( static_cast<Column>( column ) )
                       {
                           case Id:
                               return ( order == Qt::AscendingOrder ) ? a.id < b.id : a.id > b.id;
                           case Kind:
                               return ( order == Qt::AscendingOrder ) ? a.kind < b.kind : a.kind > b.kind;
                           case Name:
                               return ( order == Qt::AscendingOrder ) ? a.name < b.name : a.name > b.name;
                           default:
                               return false;
                       }
                   } );
        emit dataChanged( index( 0, 0 ), index( rowCount() - 1, columnCount() - 1 ) );
        emit layoutChanged();
    }

private:
    std::vector<OsduField> m_osduFields;
};

class OsduWellboreTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit OsduWellboreTableModel( QObject* parent = nullptr )
        : QAbstractTableModel( parent )
    {
    }

    enum Column
    {
        Name,
        Kind,
        Id,
        ColumnCount
    };

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override
    {
        Q_UNUSED( parent );
        return static_cast<int>( m_osduWellbores.size() );
    }

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override
    {
        Q_UNUSED( parent );
        return ColumnCount;
    }

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override
    {
        if ( !index.isValid() ) return QVariant();

        if ( index.row() >= static_cast<int>( m_osduWellbores.size() ) || index.row() < 0 ) return QVariant();

        if ( role == Qt::DisplayRole )
        {
            const OsduWellbore& field = m_osduWellbores.at( index.row() );
            switch ( static_cast<Column>( index.column() ) )
            {
                case Id:
                    return field.id;
                case Kind:
                    return field.kind;
                case Name:
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
            switch ( static_cast<Column>( section ) )
            {
                case Id:
                    return tr( "ID" );
                case Kind:
                    return tr( "Kind" );
                case Name:
                    return tr( "Name" );
                default:
                    return QVariant();
            }
        }
        return QVariant();
    }

    void setOsduWellbores( const QString& wellId, const std::vector<OsduWellbore>& osduWellbores )
    {
        beginResetModel();
        m_map[wellId] = osduWellbores;
        m_osduWellbores.clear();
        for ( auto [name, values] : m_map )
        {
            for ( auto v : values )
                m_osduWellbores.push_back( v );
        }
        endResetModel();
    }

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override
    {
        std::sort( m_osduWellbores.begin(),
                   m_osduWellbores.end(),
                   [column, order]( const OsduWellbore& a, const OsduWellbore& b )
                   {
                       switch ( static_cast<Column>( column ) )
                       {
                           case Id:
                               return ( order == Qt::AscendingOrder ) ? a.id < b.id : a.id > b.id;
                           case Kind:
                               return ( order == Qt::AscendingOrder ) ? a.kind < b.kind : a.kind > b.kind;
                           case Name:
                               return ( order == Qt::AscendingOrder ) ? a.name < b.name : a.name > b.name;
                           default:
                               return false;
                       }
                   } );
        emit dataChanged( index( 0, 0 ), index( rowCount() - 1, columnCount() - 1 ) );
        emit layoutChanged();
    }

private:
    std::vector<OsduWellbore>                    m_osduWellbores;
    std::map<QString, std::vector<OsduWellbore>> m_map;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class AuthenticationPage : public QWizardPage
{
    Q_OBJECT

public:
    AuthenticationPage( RiaOsduConnector* osduConnector, QWidget* parent = nullptr );

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void accessOk();

private:
    QLabel* m_connectionLabel;
    bool    m_accessOk;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class FieldSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    FieldSelectionPage( RiaOsduConnector* m_osduConnector, QWidget* parent = nullptr );
    ~FieldSelectionPage() override;

    bool isComplete() const override;
private slots:
    void fieldsFinished();
    void selectField( const QItemSelection& newSelection, const QItemSelection& oldSelection );
    void onSearchTextChanged( const QString& );
    void searchForFields();

private:
    static const int MINIMUM_CHARACTERS_FOR_SEARCH = 3;

    QLineEdit*           m_searchTextEdit;
    QPushButton*         m_searchButton;
    RiaOsduConnector*    m_osduConnector;
    QTableView*          m_tableView;
    OsduFieldTableModel* m_osduFieldsModel;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class WellSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    WellSelectionPage( RiaOsduConnector* m_osduConnector, QWidget* parent = nullptr );
    ~WellSelectionPage() override;

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void wellboresByFieldIdFinished( const QString& fieldId );
    void selectWellbore( const QItemSelection& newSelection, const QItemSelection& oldSelection );

private:
    RiaOsduConnector*       m_osduConnector;
    QTableView*             m_tableView;
    OsduWellboreTableModel* m_osduWellboresModel;
    QSortFilterProxyModel*  m_proxyModel;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class WellSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    WellSummaryPage( RiaOsduConnector* osduConnector, QWidget* parent = nullptr );

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void wellboreTrajectoryFinished( const QString& wellboreId, int numTrajectories, const QString& errorMessage );

private:
    RiaOsduConnector* m_osduConnector;
    QTextEdit*        m_textEdit;
    std::set<QString> m_pendingWellboreIds;
    mutable QMutex    m_mutex;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuWellImportWizard : public QWizard
{
    Q_OBJECT

public:
    struct WellInfo
    {
        QString name;
        QString wellId;
        QString wellboreId;
        QString wellboreTrajectoryId;
        QString existenceKind;
        double  datumElevation;
    };

    RiuWellImportWizard( RiaOsduConnector* osduConnector, QWidget* parent = nullptr );
    ~RiuWellImportWizard() override;

    // Methods used from the wizard pages
    void resetAuthenticationCount();

    void                 setSelectedFieldId( const QString& fieldId );
    QString              selectedFieldId() const;
    void                 setSelectedWellboreIds( const std::vector<QString>& wellboreIds );
    std::vector<QString> selectedWellboreIds() const;

    void                                       addWellInfo( RiuWellImportWizard::WellInfo wellInfo );
    std::vector<RiuWellImportWizard::WellInfo> importedWells() const;

public slots:
    void downloadWellPaths( const QString& wellboreId );
    void downloadWells( const QString& fieldId );
    void downloadFields( const QString& fieldName );

    void slotAuthenticationRequired( QNetworkReply* networkReply, QAuthenticator* authenticator );

private:
    RiaOsduConnector*    m_osduConnector;
    QString              m_selectedFieldId;
    std::vector<QString> m_selectedWellboreIds;

    bool m_firstTimeRequestingAuthentication;

    std::vector<RiuWellImportWizard::WellInfo> m_wellInfos;
};
