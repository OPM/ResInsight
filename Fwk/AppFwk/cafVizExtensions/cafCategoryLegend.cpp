
#include "cafCategoryLegend.h"
#include "cafCategoryMapper.h"

#include "cvfBase.h"
#include "cvfBufferObjectManaged.h"
#include "cvfCamera.h"
#include "cvfFont.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfGlyph.h"
#include "cvfMatrixState.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateLine.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfTextDrawer.h"
#include "cvfUniform.h"
#include "cvfViewport.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

#include "cvfScalarMapper.h"
#include "cafInternalLegendRenderTools.h"


using namespace cvf;

namespace caf {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CategoryLegend::CategoryLegend(Font* font, const CategoryMapper* categoryMapper)
    : m_sizeHint(200, 200),
    m_textColor(Color3::BLACK),
    m_lineColor(Color3::BLACK),
    m_backgroundColor(1.0f, 1.0f, 1.0f, 0.8f),
    m_backgroundFrameColor(0.0f, 0.0f, 0.0f, 0.5f),
    m_isBackgroundEnabled(true),
    m_lineWidth(1),
    m_font(font),
    m_categoryMapper(categoryMapper)
{
    CVF_ASSERT(font);
    CVF_ASSERT(!font->isEmpty());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CategoryLegend::~CategoryLegend()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui CategoryLegend::sizeHint()
{
    return m_sizeHint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setSizeHint(const Vec2ui& size)
{
    m_sizeHint = size;
}

//--------------------------------------------------------------------------------------------------
/// Set color of the text 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setTextColor(const Color3f& color)
{
    m_textColor = color;
}

//--------------------------------------------------------------------------------------------------
/// Set the title (text that will be rendered above the legend)
/// 
/// The legend supports multi-line titles. Separate each line with a '\n' character
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setTitle(const String& title)
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
size_t CategoryLegend::categoryCount() const
{
    if (m_categoryMapper.notNull())
    {
        return m_categoryMapper->categoryCount();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void CategoryLegend::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    renderGeneric(oglContext, position, size, false);
}

//--------------------------------------------------------------------------------------------------
/// Software rendering using software
//--------------------------------------------------------------------------------------------------
void CategoryLegend::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    renderGeneric(oglContext, position, size, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CategoryLegend::pick(int oglXCoord, int oglYCoord, const Vec2i& position, const Vec2ui& size)
{
    Recti oglRect(position, size.x(), size.y());

    OverlayColorLegendLayoutInfo layoutInViewPortCoords(oglRect.min(), Vec2ui(oglRect.width(), oglRect.height()));
    layoutInfo(&layoutInViewPortCoords);

    Vec2i legendBarOrigin = oglRect.min();
    legendBarOrigin.x() += static_cast<uint>(layoutInViewPortCoords.legendRect.min().x());
    legendBarOrigin.y() += static_cast<uint>(layoutInViewPortCoords.legendRect.min().y());

    Recti legendBarRect = Recti(legendBarOrigin, static_cast<uint>(layoutInViewPortCoords.legendRect.width()), static_cast<uint>(layoutInViewPortCoords.legendRect.height()));

    if ((oglXCoord > legendBarRect.min().x()) && (oglXCoord < legendBarRect.max().x()) &&
        (oglYCoord > legendBarRect.min().y()) && (oglYCoord < legendBarRect.max().y()))
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void CategoryLegend::renderGeneric(OpenGLContext* oglContext, 
                                   const Vec2i& position, 
                                   const Vec2ui& size, 
                                   bool software)
{
    if (size.x() <= 0 || size.y() <= 0)
    {
        return;
    }

    Camera camera;
    camera.setViewport(position.x(), position.y(), size.x(), size.y());
    camera.setProjectionAsPixelExact2D();
    camera.setViewMatrix(Mat4d::IDENTITY);
    camera.applyOpenGL();
    camera.viewport()->applyOpenGL(oglContext, Viewport::CLEAR_DEPTH);

    // Get layout information
    // Todo: Cache this between renderings. Update only when needed.
    OverlayColorLegendLayoutInfo layout(position, size);
    layoutInfo(&layout);

    // Set up text drawer
    float maxLegendRightPos = 0; 
    TextDrawer textDrawer(m_font.p());
    setupTextDrawer(&textDrawer, &layout, &maxLegendRightPos);

    Vec2f backgroundSize(CVF_MIN(maxLegendRightPos + 3.0f, (float)size.x()), (float)size.y());

    // Do the actual rendering
    if (software)
    {
        if (m_isBackgroundEnabled) InternalLegendRenderTools::renderBackgroundImmediateMode(oglContext, 
                                                                                            backgroundSize, 
                                                                                            m_backgroundColor, 
                                                                                            m_backgroundFrameColor);
        renderLegendImmediateMode(oglContext, &layout);
        textDrawer.renderSoftware(oglContext, camera);
    }
    else
    {
        const MatrixState matrixState(camera);
        if (m_isBackgroundEnabled) InternalLegendRenderTools::renderBackgroundUsingShaders(oglContext, 
                                                                                           matrixState, 
                                                                                           backgroundSize, 
                                                                                           m_backgroundColor, 
                                                                                           m_backgroundFrameColor);
        renderLegendUsingShaders(oglContext, &layout, matrixState);
        textDrawer.render(oglContext, camera);
    }

    CVF_CHECK_OGL(oglContext);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setupTextDrawer(TextDrawer* textDrawer, 
                                     OverlayColorLegendLayoutInfo* layout, 
                                     float* maxLegendRightPos)
{
    if (m_categoryMapper.isNull())
    {
        return;
    }

    CVF_ASSERT(layout);

    float legendRight = 0.0f;

    textDrawer->setVerticalAlignment(TextDrawer::CENTER);
    textDrawer->setTextColor(m_textColor);

    m_visibleCategoryLabels.clear();

    const float textX = layout->tickX + 5;

    const float overlapTolerance = 1.2f * layout->charHeight;
    float lastVisibleTextY = 0.0;

    CVF_ASSERT(m_categoryMapper.notNull());
    size_t numLabels = m_categoryMapper->categoryCount();

    float categoryHeight = static_cast<float>(layout->legendRect.height() / numLabels);

    for (size_t it = 0; it < numLabels; it++)
    {
        float textY = static_cast<float>(layout->legendRect.min().y() + it * categoryHeight + categoryHeight / 2);

        // Always draw first and last tick label. For all others, skip drawing if text ends up
        // on top of the previous label. 
        if (it != 0 && it != (numLabels - 1))
        {
            if (cvf::Math::abs(textY - lastVisibleTextY) < overlapTolerance)
            {
                m_visibleCategoryLabels.push_back(false);
                continue;
            }
            // Make sure it does not overlap the last tick as well

            float lastTickY = static_cast<float>(layout->legendRect.max().y());

            if (cvf::Math::abs(textY - lastTickY) < overlapTolerance)
            {
                m_visibleCategoryLabels.push_back(false);
                continue;
            }
        }

        String displayText = m_categoryMapper->textForCategoryIndex(it);

        Vec2f pos(textX, textY);
        textDrawer->addText(displayText, pos);

        float neededRightPos = pos.x() + m_font->textExtent(displayText).x();
        legendRight = legendRight >= neededRightPos ? legendRight :neededRightPos;

        lastVisibleTextY = textY;
        m_visibleCategoryLabels.push_back(true);
    }

    float titleY = static_cast<float>(layout->size.y()) - layout->margins.y() - layout->charHeight / 2.0f;
    for (size_t it = 0; it < m_titleStrings.size(); it++)
    {
        Vec2f pos(layout->margins.x(), titleY);
        textDrawer->addText(m_titleStrings[it], pos);

        float neededRightPos = pos.x() + m_font->textExtent(m_titleStrings[it]).x();
        legendRight = legendRight >= neededRightPos ? legendRight :neededRightPos;

        titleY -= layout->lineSpacing;
    }

    *maxLegendRightPos = legendRight;
}

//--------------------------------------------------------------------------------------------------
/// Draw the legend using shader programs
//--------------------------------------------------------------------------------------------------
void CategoryLegend::renderLegendUsingShaders(OpenGLContext* oglContext, 
                                              OverlayColorLegendLayoutInfo* layout, 
                                              const MatrixState& matrixState)
{
    CVF_CALLSITE_OPENGL(oglContext);

    CVF_TIGHT_ASSERT(layout);
    CVF_TIGHT_ASSERT(layout->size.x() > 0);
    CVF_TIGHT_ASSERT(layout->size.y() > 0);

    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);
    RenderStateLine line(static_cast<float>(m_lineWidth));
    line.applyOpenGL(oglContext);

    // All vertices. Initialized here to set Z to zero once and for all.
    static float vertexArray[] =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };

    // Per vector convenience pointers
    float* v0 = &vertexArray[0];
    float* v1 = &vertexArray[3];
    float* v2 = &vertexArray[6];
    float* v3 = &vertexArray[9];
    float* v4 = &vertexArray[12];

    // Constant coordinates
    v0[0] = v3[0] = layout->x0;
    v1[0] = v4[0] = layout->x1;

    // Connects
    static const ushort trianglesConnects[] = { 0, 1, 4, 0, 4, 3 };

    ref<ShaderProgram> shaderProgram = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram(oglContext);
    CVF_TIGHT_ASSERT(shaderProgram.notNull());

    if (shaderProgram->useProgram(oglContext))
    {
        shaderProgram->clearUniformApplyTracking();
        shaderProgram->applyFixedUniforms(oglContext, matrixState);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(ShaderProgram::VERTEX);
    glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray);

    // Render color bar as one colored quad per pixel

    int legendHeightPixelCount = static_cast<int>(layout->legendRect.height());
    if (m_categoryMapper.notNull())
    {
        int iPx;
        for (iPx = 0; iPx < legendHeightPixelCount; iPx++)
        {
            const Color3ub& clr = m_categoryMapper->mapToColor(m_categoryMapper->domainValue((iPx + 0.5) / legendHeightPixelCount));
            float y0 = static_cast<float>(layout->legendRect.min().y() + iPx);
            float y1 = static_cast<float>(layout->legendRect.min().y() + iPx + 1);

            // Dynamic coordinates for rectangle
            v0[1] = v1[1] = y0;
            v3[1] = v4[1] = y1;

            // Draw filled rectangle elements
            {
                UniformFloat uniformColor("u_color", Color4f(Color3f(clr)));
                shaderProgram->applyUniform(oglContext, uniformColor);

#ifdef CVF_OPENGL_ES
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, trianglesConnects);
#else
                glDrawRangeElements(GL_TRIANGLES, 0, 4, 6, GL_UNSIGNED_SHORT, trianglesConnects);
#endif
            }
        }
    }

    // Render frame

    // Dynamic coordinates for  tickmarks-lines
    bool isRenderingFrame = true;
    if (isRenderingFrame)
    {
        v0[0] = v2[0] = layout->legendRect.min().x() - 0.5f;
        v1[0] = v3[0] = layout->legendRect.max().x() - 0.5f;
        v0[1] = v1[1] = layout->legendRect.min().y() - 0.5f;
        v2[1] = v3[1] = layout->legendRect.max().y() - 0.5f;
        static const ushort frameConnects[] = { 0, 1, 1, 3, 3, 2, 2, 0 };

        UniformFloat uniformColor("u_color", Color4f(m_lineColor));
        shaderProgram->applyUniform(oglContext, uniformColor);

#ifdef CVF_OPENGL_ES
        glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, frameConnects);
#else
        glDrawRangeElements(GL_LINES, 0, 3, 8, GL_UNSIGNED_SHORT, frameConnects);
#endif
    }

    glDisableVertexAttribArray(ShaderProgram::VERTEX);

    CVF_TIGHT_ASSERT(shaderProgram.notNull());
    shaderProgram->useNoProgram(oglContext);

    // Reset render states
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    RenderStateLine resetLine;
    resetLine.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// Draw the legend using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void CategoryLegend::renderLegendImmediateMode(OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout)
{
#ifdef CVF_OPENGL_ES
    CVF_UNUSED(layout);
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    CVF_TIGHT_ASSERT(layout);
    CVF_TIGHT_ASSERT(layout->size.x() > 0);
    CVF_TIGHT_ASSERT(layout->size.y() > 0);

    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    RenderStateLighting_FF lighting(false);
    lighting.applyOpenGL(oglContext);

    // All vertices. Initialized here to set Z to zero once and for all.
    static float vertexArray[] =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };

    // Per vector convenience pointers
    float* v0 = &vertexArray[0];
    float* v1 = &vertexArray[3];
    float* v2 = &vertexArray[6];
    float* v3 = &vertexArray[9];
    float* v4 = &vertexArray[12];

    // Constant coordinates
    v0[0] = v3[0] = layout->x0;
    v1[0] = v4[0] = layout->x1;

    // Render color bar as one colored quad per pixel

    int legendHeightPixelCount = static_cast<int>(layout->legendRect.height());
    if (m_categoryMapper.notNull())
    {
        int iPx;
        for (iPx = 0; iPx < legendHeightPixelCount; iPx++)
        {
            const Color3ub& clr = m_categoryMapper->mapToColor(m_categoryMapper->domainValue((iPx + 0.5) / legendHeightPixelCount));
            float y0 = static_cast<float>(layout->legendRect.min().y() + iPx);
            float y1 = static_cast<float>(layout->legendRect.min().y() + iPx + 1);

            // Dynamic coordinates for rectangle
            v0[1] = v1[1] = y0;
            v3[1] = v4[1] = y1;

            // Draw filled rectangle elements
            glColor3ubv(clr.ptr());
            glBegin(GL_TRIANGLE_FAN);
            glVertex3fv(v0);
            glVertex3fv(v1);
            glVertex3fv(v4);
            glVertex3fv(v3);
            glEnd();
        }
    }

    // Render frame

    // Dynamic coordinates for  tickmarks-lines
    bool isRenderingFrame = true;
    if (isRenderingFrame)
    {
        v0[0] = v2[0] = layout->legendRect.min().x() - 0.5f;
        v1[0] = v3[0] = layout->legendRect.max().x() - 0.5f;
        v0[1] = v1[1] = layout->legendRect.min().y() - 0.5f;
        v2[1] = v3[1] = layout->legendRect.max().y() - 0.5f;

        glColor3fv(m_textColor.ptr());
        glBegin(GL_LINES);
        glVertex3fv(v0);
        glVertex3fv(v1);
        glVertex3fv(v1);
        glVertex3fv(v3);
        glVertex3fv(v3);
        glVertex3fv(v2);
        glVertex3fv(v2);
        glVertex3fv(v0);
        glEnd();

    }

    // Reset render states
    RenderStateLighting_FF resetLighting;
    resetLighting.applyOpenGL(oglContext);
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
#endif // CVF_OPENGL_ES
}

//--------------------------------------------------------------------------------------------------
/// Get layout information
//--------------------------------------------------------------------------------------------------
void CategoryLegend::layoutInfo(OverlayColorLegendLayoutInfo* layout)
{
    CVF_TIGHT_ASSERT(layout);

    ref<Glyph> glyph = m_font->getGlyph(L'A');
    layout->charHeight = static_cast<float>(glyph->height());
    layout->lineSpacing = layout->charHeight*1.5f;
    layout->margins = Vec2f(4.0f, 4.0f);

    float legendWidth = 25.0f;
    float legendHeight = static_cast<float>(layout->size.y()) - 2 * layout->margins.y() - static_cast<float>(m_titleStrings.size())*layout->lineSpacing - layout->lineSpacing;
    layout->legendRect = Rectf(layout->margins.x(), layout->margins.y() + layout->charHeight / 2.0f, legendWidth, legendHeight);

    if (layout->legendRect.width() < 1 || layout->legendRect.height() < 1)
    {
        return;
    }

    layout->x0 = layout->margins.x();
    layout->x1 = layout->margins.x() + layout->legendRect.width();
    layout->tickX = layout->x1 + 5;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setLineColor(const Color3f& lineColor)
{
    m_lineColor = lineColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setLineWidth(int lineWidth)
{
    m_lineWidth = lineWidth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setBackgroundColor(const Color4f& backgroundColor)
{
    m_backgroundColor = backgroundColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::setBackgroundFrameColor(const Color4f& backgroundFrameColor)
{
    m_backgroundFrameColor = backgroundFrameColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryLegend::enableBackground(bool enable)
{
    m_isBackgroundEnabled = enable;
}




} // namespace cvf

