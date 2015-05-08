#include "RimView.h"
#include "cafPdmObjectFactory.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RiuViewer.h"
#include "RiuMainWindow.h"
#include "cafCeetronPlusNavigation.h"
#include "cafCadNavigation.h"
#include "cvfCamera.h"
#include "cvfViewport.h"
#include "cafFrameAnimationControl.h"

CAF_PDM_ABSTRACT_SOURCE_INIT(RimView, "GenericView"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView::RimView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);


    CAF_PDM_InitField(&name, "UserDescription", QString(""), "Name", "", "", "");

    CAF_PDM_InitField(&showWindow, "ShowWindow", true, "Show 3D viewer", "", "", "");
    showWindow.setUiHidden(true);
    CAF_PDM_InitField(&cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");

    double defaultScaleFactor = 1.0;
    if (preferences) defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ, "GridZScale", defaultScaleFactor, "Z Scale", "", "Scales the scene in the Z direction", "");

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&backgroundColor, "ViewBackgroundColor", defBackgColor, "Background", "", "", "");

    CAF_PDM_InitField(&maximumFrameRate, "MaximumFrameRate", 10, "Maximum frame rate", "", "", "");
    maximumFrameRate.setUiHidden(true);
    CAF_PDM_InitField(&animationMode, "AnimationMode", false, "Animation Mode", "", "", "");
    animationMode.setUiHidden(true);

    CAF_PDM_InitField(&m_currentTimeStep, "CurrentTimeStep", 0, "Current Time Step", "", "", "");
    m_currentTimeStep.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&overlayInfoConfig, "OverlayInfoConfig", "Info Box", "", "", "");
    overlayInfoConfig = new Rim3dOverlayInfoConfig();
    overlayInfoConfig->setReservoirView(this);


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView::~RimView(void)
{
    delete this->overlayInfoConfig();

    if (m_viewer)
    {
        RiuMainWindow::instance()->removeViewer(m_viewer);
    }

    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer* RimView::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateViewerWidget()
{
    if (showWindow())
    {
        bool isViewerCreated = false;
        if (!m_viewer)
        {
            QGLFormat glFormat;
            glFormat.setDirectRendering(RiaApplication::instance()->useShaders());

            m_viewer = new RiuViewer(glFormat, NULL);
            m_viewer->setOwnerReservoirView(this);

            RiuMainWindow::instance()->addViewer(m_viewer);
            m_viewer->setMinNearPlaneDistance(10);
           
            this->resetLegendsInViewer();

            if (RiaApplication::instance()->navigationPolicy() == RiaApplication::NAVIGATION_POLICY_CEETRON)
            {
                m_viewer->setNavigationPolicy(new caf::CeetronPlusNavigation);
            }
            else
            {
                m_viewer->setNavigationPolicy(new caf::CadNavigation);
            }

            m_viewer->enablePerfInfoHud(RiaApplication::instance()->showPerformanceInfo());

            isViewerCreated = true;
        }

        RiuMainWindow::instance()->setActiveViewer(m_viewer);

        if (isViewerCreated) m_viewer->mainCamera()->setViewMatrix(cameraPosition);
        m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));

        m_viewer->update();
    }
    else
    {
        if (m_viewer)
        {
            if (m_viewer->layoutWidget()->parentWidget())
            {
                m_viewer->layoutWidget()->parentWidget()->hide();
            }
            else
            {
                m_viewer->layoutWidget()->hide(); 
            }
        }
    }

    updateViewerWidgetWindowTitle();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::scheduleCreateDisplayModelAndRedraw()
{
    RiaApplication::instance()->scheduleDisplayModelUpdateAndRedraw(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setCurrentTimeStep(int frameIndex)
{
    m_currentTimeStep = frameIndex;
    this->animationMode = true;
    this->updateCurrentTimeStep();
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateCurrentTimeStepAndRedraw()
{
    this->updateCurrentTimeStep();
    
    if (m_viewer) m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::createDisplayModelAndRedraw()
{
    if (m_viewer)
    {
        m_viewer->animationControl()->slotStop();

        this->clampCurrentTimestep();

        createDisplayModel();
        updateDisplayModelVisibility();

        if (m_viewer->frameCount() > 0)
        {
            m_viewer->animationControl()->setCurrentFrame(m_currentTimeStep);
        }
    }

    RiuMainWindow::instance()->refreshAnimationActions(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setDefaultView()
{
    if (m_viewer)
    {
        m_viewer->setDefaultView();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::endAnimation()
{
    this->animationMode = false;
    this->updateStaticCellColors();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setupBeforeSave()
{
    if (m_viewer)
    {
        animationMode = m_viewer->isAnimationActive();
        cameraPosition = m_viewer->mainCamera()->viewMatrix();
    }
}



