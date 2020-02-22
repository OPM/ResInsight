/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class RigMainGrid;

//==================================================================================================
///
//==================================================================================================
class RimGridInfo : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridInfo();
    ~RimGridInfo() override {}

    void    setName( const QString& name );
    void    setEclipseGridIndex( int index );
    void    setActive( bool active );
    bool    isActive() const;
    QString name() const;
    int     eclipseGridIndex() const;

    caf::PdmFieldHandle* objectToggleField() override;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<bool>    m_isActive;
    caf::PdmField<QString> m_gridName;
    caf::PdmField<int>     m_eclipseGridIndex;
};

//==================================================================================================
///
//==================================================================================================
class RimGridInfoCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridInfoCollection();
    ~RimGridInfoCollection() override {}

    bool                      isActive() const;
    void                      addGridInfo( RimGridInfo* gridInfo );
    void                      clear();
    bool                      containsGrid( const QString& gridName ) const;
    void                      deleteGridInfo( const QString& gridName );
    std::vector<RimGridInfo*> gridInfos() const;
    caf::PdmFieldHandle*      objectToggleField() override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<bool>                   m_isActive;
    caf::PdmChildArrayField<RimGridInfo*> m_gridInfos;
};

//==================================================================================================
///
///
//==================================================================================================
class RimGridCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCollection();
    ~RimGridCollection() override;

    void                setActive( bool active );
    bool                isActive() const;
    std::vector<size_t> indicesToVisibleGrids() const;

    caf::PdmFieldHandle* objectToggleField() override;
    void                 syncFromMainEclipseGrid();
    void                 setMainGridActive( bool active );

    static const QString persistentGridUiName();
    static const QString temporaryGridUiName();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    const RigMainGrid* mainEclipseGrid() const;
    bool               hasPersistentLgrs() const;
    bool               hasTemporaryLgrs() const;

    caf::PdmField<bool>                        m_isActive;
    caf::PdmChildField<RimGridInfo*>           m_mainGrid;
    caf::PdmChildField<RimGridInfoCollection*> m_persistentLgrs;
    caf::PdmChildField<RimGridInfoCollection*> m_temporaryLgrs;
};
