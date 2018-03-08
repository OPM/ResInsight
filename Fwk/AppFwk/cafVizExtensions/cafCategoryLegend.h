
#pragma once

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
class CategoryLegend : public cvf::OverlayItem
{
public:
    CategoryLegend(cvf::Font* font, const CategoryMapper* categoryMapper);
    virtual ~CategoryLegend();

    void         setSizeHint(const cvf::Vec2ui& size);

    void         setTextColor(const cvf::Color3f& color);
    void         setLineColor(const cvf::Color3f& lineColor);
    void         setLineWidth(int lineWidth);
    void         setTitle(const cvf::String& title);

    size_t       categoryCount() const;

protected:
    cvf::Vec2ui sizeHint() override;
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

    void         renderGeneric(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size, bool software);
    void         setupTextDrawer(cvf::TextDrawer* textDrawer, OverlayColorLegendLayoutInfo* layout);
    void         renderLegendUsingShaders(cvf::OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout, const cvf::MatrixState& matrixState);
    void         renderLegendImmediateMode(cvf::OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout);

protected:
    std::vector<bool>         m_visibleCategoryLabels;    // Skip labels ending up on top of previous visible label
                              
    cvf::Vec2ui               m_sizeHint;     // Pixel size of the color legend area
                              
    cvf::Color3f              m_textColor;
    cvf::Color3f              m_lineColor;
    int                       m_lineWidth;
    std::vector<cvf::String>  m_titleStrings;
    cvf::ref<cvf::Font>       m_font;

    cvf::cref<CategoryMapper> m_categoryMapper;
};

}
