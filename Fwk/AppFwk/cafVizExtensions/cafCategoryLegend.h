
#pragma once

#include "cafTitledOverlayFrame.h"
#include "cvfBase.h"
#include "cvfArray.h"
#include "cvfCamera.h"
#include "cvfOverlayItem.h"
#include "cvfRect.h"
#include "cvfString.h"

namespace cvf {

class Font;
class ShaderProgram;
class MatrixState;
class TextDrawer;
}

namespace caf {
class CategoryMapper;

//==================================================================================================
//
//
//==================================================================================================
class CategoryLegend : public caf::TitledOverlayFrame
{
public:
    CategoryLegend(cvf::Font* font, const CategoryMapper* categoryMapper);
    virtual ~CategoryLegend();

    size_t       categoryCount() const;
    void         computeLayoutAndExtents() override;

protected:
    void        render(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size) override;
    void        renderSoftware(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size) override;
    bool        pick(int oglXCoord, int oglYCoord, const cvf::Vec2i& position, const cvf::Vec2ui& size) override;

    struct OverlayColorLegendLayoutInfo
    {
        OverlayColorLegendLayoutInfo(const cvf::Vec2i& pos, const cvf::Vec2ui& setSize)
        {
            charHeight = 0.0f;
            lineSpacing = 0.0f;
            margins = cvf::Vec2f::ZERO;
            tickX = 0.0f;
            x0 = 0.0f;
            x1 = 0.0f;

            position = pos;
            size = setSize;
        }

        float charHeight;
        float lineSpacing;
        cvf::Vec2f margins;
        float tickX;
        float x0, x1;

        cvf::Rectf legendRect;

        cvf::Vec2i position;
        cvf::Vec2ui size;
    };

    void         layoutInfo(OverlayColorLegendLayoutInfo* layout);

    void         renderGeneric(cvf::OpenGLContext* oglContext, 
                               const cvf::Vec2i& position, 
                               const cvf::Vec2ui& size, 
                               bool software);
    void         setupTextDrawer(cvf::TextDrawer* textDrawer, 
                                 OverlayColorLegendLayoutInfo* layout, 
                                 float* maxLegendRightPos);
    void         renderLegendUsingShaders(cvf::OpenGLContext* oglContext, 
                                          OverlayColorLegendLayoutInfo* layout,
                                          const cvf::MatrixState& matrixState);
    void         renderLegendImmediateMode(cvf::OpenGLContext* oglContext, 
                                           OverlayColorLegendLayoutInfo* layout);

protected:
    std::vector<bool>            m_visibleCategoryLabels;    // Skip labels ending up on top of previous visible label
    OverlayColorLegendLayoutInfo m_Layout;
    cvf::ref<cvf::TextDrawer>    m_textDrawer;
    cvf::cref<CategoryMapper>    m_categoryMapper;
};

}
