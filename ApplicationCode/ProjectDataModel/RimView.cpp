#include "RimView.h"
#include "cafPdmObjectFactory.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RiuViewer.h"

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
 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer* RimView::viewer()
{
    return m_viewer;
}



