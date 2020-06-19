
#pragma once

#include "cafTitledOverlayFrame.h"
#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfCamera.h"
#include "cvfOverlayItem.h"
#include "cvfRect.h"
#include "cvfString.h"

namespace cvf
{
class Font;
class ShaderProgram;
class MatrixState;
class TextDrawer;
} // namespace cvf

namespace caf
{
class CategoryMapper;

//==================================================================================================
//
//
//==================================================================================================
class CategoryLegend : public caf::TitledOverlayFrame
{
public:
    CategoryLegend( cvf::Font* font, const CategoryMapper* categoryMapper );
    ~CategoryLegend() override;

    size_t categoryCount() const;

    cvf::Vec2ui preferredSize() override;

protected:
    void render( cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size ) override;
    void renderSoftware( cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size ) override;
    bool pick( int oglXCoord, int oglYCoord, const cvf::Vec2i& position, const cvf::Vec2ui& size ) override;

    struct OverlayColorLegendLayoutInfo
    {
        OverlayColorLegendLayoutInfo( const cvf::Vec2ui& setSize )
        {
            charHeight        = 0.0f;
            lineSpacing       = 0.0f;
            margins           = cvf::Vec2f::ZERO;
            tickStartX        = 0.0f;
            tickMidX          = 0.0f;
            tickEndX          = 0.0f;
            tickTextLeadSpace = 0.0f;

            overallLegendSize = setSize;
        }

        float      charHeight;
        float      lineSpacing;
        cvf::Vec2f margins;
        float      tickStartX, tickMidX, tickEndX;
        float      tickTextLeadSpace;

        cvf::Rectf colorBarRect;

        cvf::Vec2ui overallLegendSize;
    };

    void layoutInfo( OverlayColorLegendLayoutInfo* layout );

    void renderGeneric( cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size, bool software );
    void setupTextDrawer( cvf::TextDrawer* textDrawer, const OverlayColorLegendLayoutInfo* layout );
    void renderLegendUsingShaders( cvf::OpenGLContext*           oglContext,
                                   OverlayColorLegendLayoutInfo* layout,
                                   const cvf::MatrixState&       matrixState );
    void renderLegendImmediateMode( cvf::OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout );

protected:
    std::vector<bool>            m_visibleCategoryLabels; // Skip labels ending up on top of previous visible label
    OverlayColorLegendLayoutInfo m_Layout;
    cvf::ref<cvf::TextDrawer>    m_textDrawer;
    cvf::cref<CategoryMapper>    m_categoryMapper;
};

} // namespace caf
