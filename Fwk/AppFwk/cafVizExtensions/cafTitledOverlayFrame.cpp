#include "cafTitledOverlayFrame.h"
#include "cafCategoryMapper.h"
#include "cvfFont.h"

#include <algorithm>

using namespace cvf;

namespace caf {

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    TitledOverlayFrame::TitledOverlayFrame(Font* font, unsigned int width, unsigned int height)
        : m_font(font)
        , m_renderSize(width, height)
        , m_textColor(Color3::BLACK)
        , m_lineColor(Color3::BLACK)
        , m_lineWidth(1)
        , m_isBackgroundEnabled(true)
        , m_backgroundColor(1.0f, 1.0f, 1.0f, 0.8f)
        , m_backgroundFrameColor(0.0f, 0.0f, 0.0f, 0.5f)                
    {
    }

    TitledOverlayFrame::~TitledOverlayFrame()
    {
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setRenderSize(const Vec2ui& size)
    {
        m_renderSize = size;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    cvf::Vec2ui TitledOverlayFrame::renderSize() const
    {
        return m_renderSize;
    }

    //--------------------------------------------------------------------------------------------------
    /// Set color of the text 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setTextColor(const Color3f& color)
    {
        m_textColor = color;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setLineColor(const Color3f& lineColor)
    {
        m_lineColor = lineColor;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setLineWidth(int lineWidth)
    {
        m_lineWidth = lineWidth;
    }

    //--------------------------------------------------------------------------------------------------
    /// Set the title (text that will be rendered above the legend)
    /// 
    /// The legend supports multi-line titles. Separate each line with a '\n' character
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setTitle(const String& title)
    {
        // Title
        if (title.isEmpty())
        {
            m_titleStrings.clear();
        }
        else
        {
            m_titleStrings = title.split("\n");
        }
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::enableBackground(bool enable)
    {
        m_isBackgroundEnabled = enable;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setBackgroundColor(const Color4f& backgroundColor)
    {
        m_backgroundColor = backgroundColor;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void TitledOverlayFrame::setBackgroundFrameColor(const Color4f& backgroundFrameColor)
    {
        m_backgroundFrameColor = backgroundFrameColor;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    cvf::Vec2ui TitledOverlayFrame::sizeHint()
    {
        return m_renderSize;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    cvf::Color3f TitledOverlayFrame::textColor() const
    {
        return m_textColor;
    }
    
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    cvf::Color3f TitledOverlayFrame::lineColor() const
    {
        return m_lineColor;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    int TitledOverlayFrame::lineWidth() const
    {
        return m_lineWidth;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    bool TitledOverlayFrame::backgroundEnabled() const
    {
        return m_isBackgroundEnabled;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    cvf::Color4f TitledOverlayFrame::backgroundColor() const
    {
        return m_backgroundColor;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    cvf::Color4f TitledOverlayFrame::backgroundFrameColor() const
    {
        return m_backgroundFrameColor;
    }

    std::vector<cvf::String>& TitledOverlayFrame::titleStrings()
    {
        return m_titleStrings;
    }

    cvf::Font* TitledOverlayFrame::font()
    {
        return m_font.p();
    }
}