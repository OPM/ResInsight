/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

#include <QObject>
#include <QPointer>

namespace cvf {
    class Model;
    class ModelBasicList;
};


namespace caf {
    class Viewer;
};

class QMouseEvent;

class RicPointTangentManipulatorPartMgr;

//==================================================================================================
//
//
//==================================================================================================
class RicPointTangentManipulator : public QObject
{
    Q_OBJECT

public:
    explicit RicPointTangentManipulator(caf::Viewer* viewer);
    ~RicPointTangentManipulator();

    void setOrigin(const cvf::Vec3d& origin);
    void setTangent(const cvf::Vec3d& tangent);
    void setHandleSize(double handleSize);

    void appendPartsToModel(cvf::ModelBasicList* model);

signals:
    void        notifySelected();
    void        notifyRedraw();
    void        notifyUpdate(const cvf::Vec3d& origin, const cvf::Vec3d& tangent);

protected:
    bool        eventFilter(QObject *obj, QEvent *event);

private:
    QPointer<caf::Viewer>           m_viewer;

    cvf::ref<RicPointTangentManipulatorPartMgr> m_partManager;
};


#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfVector3.h"
#include "cvfCollection.h"
#include "cvfMatrix4.h"
#include "cvfString.h"
#include "cvfColor4.h"

namespace cvf
{
class ModelBasicList;
class Part;
class DrawableGeo;
class Ray;
class HitItem;

template <typename> class Array;
typedef Array<Vec3f>   Vec3fArray;
typedef Array<uint>   UIntArray;

}

class RicPointTangentManipulatorPartMgr : public cvf::Object
{
public:
    enum HandleType
    {
        HORIZONTAL_PLANE,
        VERTICAL_AXIS,
        AZIMUTH, 
        INCLINATION
    };

public:
    RicPointTangentManipulatorPartMgr();
    ~RicPointTangentManipulatorPartMgr();

    void    setOrigin(const cvf::Vec3d& origin);
    void    setTangent(const cvf::Vec3d& tangent);
    void    setHandleSize(double handleSize);
    void    originAndTangent(cvf::Vec3d* origin, cvf::Vec3d* tangent);

    bool    isManipulatorActive() const;
    void    tryToActivateManipulator(const cvf::HitItem* hitItem);
    void    updateManipulatorFromRay(const cvf::Ray* ray);
    void    endManipulator();

    void    appendPartsToModel(cvf::ModelBasicList* model);

private:
    void        createAllHandleParts();
    void        clearAllGeometryAndParts();
    void        recreateAllGeometryAndParts();

    void        createHorizontalPlaneHandle();
    void        createVerticalAxisHandle();

    void addHandlePart(cvf::DrawableGeo* geo,
                       const cvf::Color4f& color, 
                       HandleType handleId, 
                       const cvf::String& partName);

    void addActiveModePart(cvf::DrawableGeo* geo, 
                           const cvf::Color4f& color, 
                           HandleType handleId, 
                           const cvf::String& partName);

    static cvf::ref<cvf::DrawableGeo> createTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray);
    static cvf::ref<cvf::DrawableGeo> createIndexedTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray, 
                                                                       cvf::UIntArray* triangleIndices);
    static cvf::ref<cvf::Part> createPart(cvf::DrawableGeo* geo,
                                          const cvf::Color4f& color,
                                          const cvf::String& partName);
private:
    size_t                      m_currentHandleIndex;
    std::vector< HandleType >   m_handleIds;             // These arrays have the same length
    cvf::Collection<cvf::Part>  m_handleParts;           // These arrays have the same length
    cvf::Collection<cvf::Part>  m_activeDragModeParts;

    cvf::Vec3d          m_origin;
    cvf::Vec3d          m_tangent;
    double              m_handleSize;

    cvf::Vec3d          m_initialPickPoint;
    cvf::Vec3d          m_tangentOnStartManipulation;
    cvf::Vec3d          m_originOnStartManipulation;

};


//==================================================================================================
/// 
///
///
//==================================================================================================


#include "cafSelectionChangedReceiver.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafFactory.h"
// PdmUiObjectEditorHandle -<| PdmUiWidgetObjectEditorHandle --<| PdmUiFormLayoutObjectEditor 
//                         -<| PdmUi3dObjectEditorHandle
namespace caf
{

//==================================================================================================
/// Macros helping in development of PDM UI 3d editors
//==================================================================================================

/// CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT assists the factory used when creating editors
/// Place this in the header file inside the class definition of your PdmUiEditor

#define CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT \
public: \
    static QString uiEditorTypeName()

/// CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT  implements editorTypeName() and registers the field editor in the field editor factory
/// Place this in the cpp file, preferably above the constructor

#define CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT(EditorClassName) \
    QString EditorClassName::uiEditorTypeName() { return #EditorClassName; } \
    CAF_FACTORY_REGISTER(caf::PdmUi3dObjectEditorHandle, EditorClassName, QString, EditorClassName::uiEditorTypeName())


class PdmUi3dObjectEditorHandle : public caf::PdmUiObjectEditorHandle
{
public:
    PdmUi3dObjectEditorHandle();
    ~PdmUi3dObjectEditorHandle() override;

    void setViewer(caf::Viewer* ownerViewer);

protected:
    // To be removed when splitting the PdmUiObjectEditorHandle
    virtual QWidget* createWidget(QWidget* parent) override { return nullptr;} 

    QPointer<caf::Viewer>                   m_ownerViewer;
};

//==================================================================================================
/// 
///
///
//==================================================================================================


// Selected object 3D editor visualizer
class PdmUiSelectionVisualizer3d : public QObject, caf::SelectionChangedReceiver
{
    Q_OBJECT
public:
    PdmUiSelectionVisualizer3d(caf::Viewer* ownerViewer);
    ~PdmUiSelectionVisualizer3d(); 
protected:
    virtual void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    std::vector< QPointer<PdmUi3dObjectEditorHandle> > m_active3DEditors;

    QPointer<caf::Viewer>                   m_ownerViewer;
};


}

//==================================================================================================
/// 
///
///
//==================================================================================================

class RicWellTarget3dEditor;

class RicWellPathGeometry3dEditor : public caf::PdmUi3dObjectEditorHandle
{
    CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT;
    Q_OBJECT
public:
    RicWellPathGeometry3dEditor();
    ~RicWellPathGeometry3dEditor() override;

protected:
    virtual void configureAndUpdateUi(const QString& uiConfigName) override;

private:
    
    std::vector<RicWellTarget3dEditor*> m_targetEditors;
};

//==================================================================================================
/// 
///
///
//==================================================================================================


class RicWellTarget3dEditor : public caf::PdmUi3dObjectEditorHandle
{
    CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT;
    Q_OBJECT
public:
    RicWellTarget3dEditor();
    ~RicWellTarget3dEditor() override;

protected:
    virtual void configureAndUpdateUi(const QString& uiConfigName) override;
    virtual void cleanupBeforeSettingPdmObject() override;

private slots:
    void slotUpdated(const cvf::Vec3d& origin, const cvf::Vec3d& tangent);
    void slotSelectedIn3D();
private:
    QPointer<RicPointTangentManipulator> m_manipulator;
    cvf::ref<cvf::ModelBasicList> m_cvfModel;
};

class RiuViewer;

// 3D editor manager
