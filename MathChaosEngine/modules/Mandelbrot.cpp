#include "pch.h"
#include "Mandelbrot.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

namespace {
    static constexpr int kMaxHistorySize = 256;

    static double Clamp01(double v)
    {
        if (v < 0.0) return 0.0;
        if (v > 1.0) return 1.0;
        return v;
    }
}

Mandelbrot::Mandelbrot()
    : m_viewWidth(0),
      m_viewHeight(0),
      m_view{ -0.75, 0.0, 1.0 },
      m_maxIterCurrent(180),
      m_dirty(true),
      m_pixelsChanged(false),
      m_palette(PALETTE_SCIENTIFIC),
      m_quality(QUALITY_BALANCED),
      m_stage(STAGE_PREVIEW),
      m_progress(0.0),
      m_previewStep(3),
      m_previewRow(0),
      m_tileSize(64),
      m_timeBudgetMs(8),
      m_iterBase(180),
      m_iterCap(1400),
      m_powerN(2),
      m_family(FAMILY_MULTIBROT),
      m_nextTile(0),
      m_dragging(false),
      m_boxZoomMode(false),
      m_dragMoved(false),
      m_historyIndex(-1)
{
    std::memset(&m_bmi, 0, sizeof(m_bmi));
    m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_bmi.bmiHeader.biPlanes = 1;
    m_bmi.bmiHeader.biBitCount = 32;
    m_bmi.bmiHeader.biCompression = BI_RGB;

    applyQualityPreset();
    commitCurrentView();
}

void Mandelbrot::reset()
{
    m_view = { -0.75, 0.0, 1.0 };
    m_history.clear();
    m_historyIndex = -1;
    commitCurrentView();
    beginProgressiveRender();
}

void Mandelbrot::update(double dt)
{
    UNREFERENCED_PARAMETER(dt);

    if (!m_dirty || m_stage == STAGE_FINAL)
    {
        return;
    }

    auto start = std::chrono::steady_clock::now();
    const auto budget = std::chrono::milliseconds(m_timeBudgetMs);

    bool touched = false;
    while (std::chrono::steady_clock::now() - start < budget)
    {
        if (!renderWorkUnit())
        {
            break;
        }
        touched = true;
    }

    if (touched)
    {
        m_pixelsChanged = true;
    }
}

void Mandelbrot::render(CDC* pDC)
{
    CRect rect;
    pDC->GetClipBox(&rect);

    ensureBufferSize(rect.Width(), rect.Height());

    SetDIBitsToDevice(
        pDC->GetSafeHdc(),
        rect.left,
        rect.top,
        static_cast<DWORD>(m_viewWidth),
        static_cast<DWORD>(m_viewHeight),
        0,
        0,
        0,
        static_cast<UINT>(m_viewHeight),
        m_pixels.data(),
        &m_bmi,
        DIB_RGB_COLORS);
}

void Mandelbrot::onMouseDown(int x, int y)
{
    onMouseWheel(x, y, 120);
}

void Mandelbrot::onMouseMove(int x, int y)
{
    if (m_dragging)
    {
        onMouseDragMove(x, y);
    }
}

void Mandelbrot::onMouseWheel(int x, int y, int delta)
{
    if (m_viewWidth <= 0 || m_viewHeight <= 0)
    {
        return;
    }

    ViewState before = m_view;

    double anchorX = 0.0;
    double anchorY = 0.0;
    screenToComplex(x, y, anchorX, anchorY);

    const double scale = (delta > 0) ? 1.20 : 1.0 / 1.20;
    m_view.zoom *= scale;
    m_view.zoom = (std::max)(1.0, m_view.zoom);

    double afterX = 0.0;
    double afterY = 0.0;
    screenToComplex(x, y, afterX, afterY);

    m_view.centerX += (anchorX - afterX);
    m_view.centerY += (anchorY - afterY);

    if (isDifferentView(before, m_view))
    {
        commitCurrentView();
        beginProgressiveRender();
    }
}

void Mandelbrot::onMouseDragBegin(int x, int y, bool boxZoomMode)
{
    if (m_viewWidth <= 0 || m_viewHeight <= 0)
    {
        return;
    }

    m_dragging = true;
    m_boxZoomMode = boxZoomMode;
    m_dragMoved = false;
    m_dragStart = CPoint(x, y);
    m_dragCurrent = CPoint(x, y);
    m_dragStartView = m_view;
    m_beforeInteractionView = m_view;
}

void Mandelbrot::onMouseDragMove(int x, int y)
{
    if (!m_dragging)
    {
        return;
    }

    m_dragCurrent = CPoint(x, y);
    const int dx = m_dragCurrent.x - m_dragStart.x;
    const int dy = m_dragCurrent.y - m_dragStart.y;

    if (std::abs(dx) + std::abs(dy) < 2)
    {
        return;
    }

    m_dragMoved = true;

    if (m_boxZoomMode)
    {
        m_pixelsChanged = true;
        return;
    }

    double spanX = 0.0;
    double spanY = 0.0;
    currentSpan(spanX, spanY);

    m_view.centerX = m_dragStartView.centerX - (static_cast<double>(dx) / static_cast<double>(m_viewWidth)) * spanX;
    m_view.centerY = m_dragStartView.centerY - (static_cast<double>(dy) / static_cast<double>(m_viewHeight)) * spanY;

    beginProgressiveRender();
}

void Mandelbrot::onMouseDragEnd(int x, int y)
{
    if (!m_dragging)
    {
        return;
    }

    m_dragCurrent = CPoint(x, y);
    bool committed = false;

    if (m_boxZoomMode)
    {
        CRect box = getBoxZoomRect();
        normalizeBoxRect(box);

        if (box.Width() > 8 && box.Height() > 8)
        {
            double x0 = 0.0;
            double y0 = 0.0;
            double x1 = 0.0;
            double y1 = 0.0;
            screenToComplex(box.left, box.top, x0, y0);
            screenToComplex(box.right, box.bottom, x1, y1);

            m_view.centerX = (x0 + x1) * 0.5;
            m_view.centerY = (y0 + y1) * 0.5;

            const double zoomX = static_cast<double>(m_viewWidth) / static_cast<double>(box.Width());
            const double zoomY = static_cast<double>(m_viewHeight) / static_cast<double>(box.Height());
            const double zoomFactor = (std::min)(zoomX, zoomY);
            m_view.zoom = (std::max)(1.0, m_dragStartView.zoom * zoomFactor);
            committed = true;
        }
    }
    else if (m_dragMoved)
    {
        committed = isDifferentView(m_beforeInteractionView, m_view);
    }
    m_dragging = false;
    m_boxZoomMode = false;

    if (committed)
    {
        commitCurrentView();
        beginProgressiveRender();
    }
    else
    {
        m_view = m_beforeInteractionView;
    }

    m_pixelsChanged = true;
}

void Mandelbrot::zoomOut()
{
    ViewState before = m_view;
    m_view.zoom /= 1.8;
    m_view.zoom = (std::max)(1.0, m_view.zoom);

    if (isDifferentView(before, m_view))
    {
        commitCurrentView();
        beginProgressiveRender();
    }
}

void Mandelbrot::goBack()
{
    if (m_historyIndex <= 0)
    {
        return;
    }

    --m_historyIndex;
    applyView(m_history[m_historyIndex]);
    beginProgressiveRender();
}

void Mandelbrot::goHome()
{
    if (m_history.empty())
    {
        return;
    }

    m_historyIndex = 0;
    applyView(m_history[0]);
    beginProgressiveRender();
}

void Mandelbrot::setPalette(PaletteId palette)
{
    if (m_palette == palette)
    {
        return;
    }

    m_palette = palette;
    beginProgressiveRender();
}

void Mandelbrot::setQuality(QualityPreset quality)
{
    if (m_quality == quality)
    {
        return;
    }

    m_quality = quality;
    applyQualityPreset();
    beginProgressiveRender();
}

void Mandelbrot::setPower(int n)
{
    const int clamped = clampPower(n);
    if (m_powerN == clamped)
    {
        return;
    }

    m_powerN = clamped;
    beginProgressiveRender();
}

void Mandelbrot::setFamily(FractalFamily family)
{
    if (m_family == family)
    {
        return;
    }
    m_family = family;
    beginProgressiveRender();
}

bool Mandelbrot::isRendering() const
{
    return m_stage != STAGE_FINAL || m_dirty;
}

bool Mandelbrot::consumePixelsChanged()
{
    const bool changed = m_pixelsChanged;
    m_pixelsChanged = false;
    return changed;
}

bool Mandelbrot::getComplexAtScreen(int sx, int sy, double& outX, double& outY) const
{
    if (m_viewWidth <= 0 || m_viewHeight <= 0)
    {
        return false;
    }

    screenToComplex(sx, sy, outX, outY);
    return true;
}

bool Mandelbrot::getScreenForComplex(double x, double y, int& outSx, int& outSy) const
{
    if (m_viewWidth <= 0 || m_viewHeight <= 0)
    {
        return false;
    }

    complexToScreen(x, y, outSx, outSy);
    return true;
}

bool Mandelbrot::isBoxZoomActive() const
{
    return m_dragging && m_boxZoomMode;
}

CRect Mandelbrot::getBoxZoomRect() const
{
    return CRect(m_dragStart, m_dragCurrent);
}

Mandelbrot::RenderStage Mandelbrot::getRenderStage() const
{
    return m_stage;
}

double Mandelbrot::getRenderProgress() const
{
    return m_progress;
}

Mandelbrot::PaletteId Mandelbrot::getPalette() const
{
    return m_palette;
}

Mandelbrot::QualityPreset Mandelbrot::getQuality() const
{
    return m_quality;
}

int Mandelbrot::getPower() const
{
    return m_powerN;
}

Mandelbrot::FractalFamily Mandelbrot::getFamily() const
{
    return m_family;
}

double Mandelbrot::getZoom() const
{
    return m_view.zoom;
}

void Mandelbrot::ensureBufferSize(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (width == m_viewWidth && height == m_viewHeight)
    {
        return;
    }

    m_viewWidth = width;
    m_viewHeight = height;
    m_pixels.assign(m_viewWidth * m_viewHeight, makeBgr(0, 0, 0));
    m_bmi.bmiHeader.biWidth = m_viewWidth;
    m_bmi.bmiHeader.biHeight = -m_viewHeight;

    beginProgressiveRender();
}

void Mandelbrot::beginProgressiveRender()
{
    if (m_viewWidth <= 0 || m_viewHeight <= 0)
    {
        return;
    }

    m_maxIterCurrent = computeAdaptiveMaxIter();

    m_stage = STAGE_PREVIEW;
    m_previewRow = 0;
    m_tiles.clear();
    m_nextTile = 0;
    m_progress = 0.0;
    m_dirty = true;
    m_pixelsChanged = true;
}

bool Mandelbrot::renderWorkUnit()
{
    if (m_stage == STAGE_PREVIEW)
    {
        const bool active = renderPreviewRow();
        if (active)
        {
            return true;
        }

        m_stage = STAGE_REFINING;
        buildTileQueue();
    }

    if (m_stage == STAGE_REFINING)
    {
        const bool active = renderNextTile();
        if (active)
        {
            return true;
        }

        m_stage = STAGE_FINAL;
        m_progress = 1.0;
        m_dirty = false;
        return false;
    }

    return false;
}

bool Mandelbrot::renderPreviewRow()
{
    if (m_previewRow >= m_viewHeight)
    {
        return false;
    }

    const int y0 = m_previewRow;
    const int y1 = (std::min)(m_viewHeight, y0 + m_previewStep);

    for (int x0 = 0; x0 < m_viewWidth; x0 += m_previewStep)
    {
        const int x1 = (std::min)(m_viewWidth, x0 + m_previewStep);

        const int sx = x0 + (x1 - x0) / 2;
        const int sy = y0 + (y1 - y0) / 2;

        double cx = 0.0;
        double cy = 0.0;
        screenToComplex(sx, sy, cx, cy);

        const std::uint32_t color = computeColor(cx, cy, (std::max)(32, m_maxIterCurrent / 3));

        for (int py = y0; py < y1; ++py)
        {
            const int row = py * m_viewWidth;
            for (int px = x0; px < x1; ++px)
            {
                m_pixels[row + px] = color;
            }
        }
    }

    m_previewRow += m_previewStep;
    m_progress = 0.25 * Clamp01(static_cast<double>(m_previewRow) / static_cast<double>((std::max)(1, m_viewHeight)));

    return true;
}

bool Mandelbrot::renderNextTile()
{
    if (m_nextTile >= m_tiles.size())
    {
        return false;
    }

    const CRect tile = m_tiles[m_nextTile];
    for (int py = tile.top; py < tile.bottom; ++py)
    {
        const int row = py * m_viewWidth;
        for (int px = tile.left; px < tile.right; ++px)
        {
            double cx = 0.0;
            double cy = 0.0;
            screenToComplex(px, py, cx, cy);
            m_pixels[row + px] = computeColor(cx, cy, m_maxIterCurrent);
        }
    }

    ++m_nextTile;
    const double t = Clamp01(static_cast<double>(m_nextTile) / static_cast<double>((std::max)(size_t(1), m_tiles.size())));
    m_progress = 0.25 + t * 0.75;

    return true;
}

void Mandelbrot::buildTileQueue()
{
    m_tiles.clear();

    const double cx = static_cast<double>(m_viewWidth) * 0.5;
    const double cy = static_cast<double>(m_viewHeight) * 0.5;

    for (int y = 0; y < m_viewHeight; y += m_tileSize)
    {
        for (int x = 0; x < m_viewWidth; x += m_tileSize)
        {
            CRect tile;
            tile.left = x;
            tile.top = y;
            tile.right = (std::min)(m_viewWidth, x + m_tileSize);
            tile.bottom = (std::min)(m_viewHeight, y + m_tileSize);
            m_tiles.push_back(tile);
        }
    }

    std::sort(m_tiles.begin(), m_tiles.end(), [cx, cy](const CRect& a, const CRect& b) {
        const double acx = (a.left + a.right) * 0.5;
        const double acy = (a.top + a.bottom) * 0.5;
        const double bcx = (b.left + b.right) * 0.5;
        const double bcy = (b.top + b.bottom) * 0.5;

        const double adx = acx - cx;
        const double ady = acy - cy;
        const double bdx = bcx - cx;
        const double bdy = bcy - cy;

        return (adx * adx + ady * ady) < (bdx * bdx + bdy * bdy);
    });

    m_nextTile = 0;
}

void Mandelbrot::applyQualityPreset()
{
    switch (m_quality)
    {
    case QUALITY_FAST:
        m_previewStep = 4;
        m_tileSize = 96;
        m_timeBudgetMs = 6;
        m_iterBase = 120;
        m_iterCap = 900;
        break;
    case QUALITY_DETAIL:
        m_previewStep = 2;
        m_tileSize = 48;
        m_timeBudgetMs = 12;
        m_iterBase = 260;
        m_iterCap = 2200;
        break;
    case QUALITY_BALANCED:
    default:
        m_previewStep = 3;
        m_tileSize = 64;
        m_timeBudgetMs = 8;
        m_iterBase = 180;
        m_iterCap = 1400;
        break;
    }
}

int Mandelbrot::computeAdaptiveMaxIter() const
{
    const double z = (std::max)(1.0, m_view.zoom);
    const double level = std::log2(z);
    const int powerBoost = (m_powerN - 2) * 14;
    const int adaptive = m_iterBase + static_cast<int>(level * 18.0) + powerBoost;
    return (std::min)(m_iterCap, (std::max)(48, adaptive));
}

int Mandelbrot::clampPower(int n) const
{
    return (std::max)(2, (std::min)(5, n));
}

std::uint32_t Mandelbrot::computeColor(double cx, double cy, int maxIter) const
{
    if (m_family == FAMILY_NEWTON)
    {
        double zx = cx;
        double zy = cy;
        int iter = 0;

        auto powerOf = [](double x, double y, int p, double& ox, double& oy) {
            if (p <= 0) { ox = 1.0; oy = 0.0; return; }
            double rx = x;
            double ry = y;
            for (int i = 1; i < p; ++i)
            {
                const double nx = rx * x - ry * y;
                const double ny = rx * y + ry * x;
                rx = nx;
                ry = ny;
            }
            ox = rx;
            oy = ry;
        };

        for (; iter < maxIter; ++iter)
        {
            double zNRe = 0.0, zNIm = 0.0;
            double zNm1Re = 0.0, zNm1Im = 0.0;
            powerOf(zx, zy, m_powerN, zNRe, zNIm);
            powerOf(zx, zy, m_powerN - 1, zNm1Re, zNm1Im);

            const double fRe = zNRe - 1.0;
            const double fIm = zNIm;
            const double fNorm = std::sqrt(fRe * fRe + fIm * fIm);
            if (fNorm < 1e-6)
            {
                break;
            }

            const double dRe = static_cast<double>(m_powerN) * zNm1Re;
            const double dIm = static_cast<double>(m_powerN) * zNm1Im;
            const double den = dRe * dRe + dIm * dIm;
            if (den < 1e-18)
            {
                break;
            }

            const double qRe = (fRe * dRe + fIm * dIm) / den;
            const double qIm = (fIm * dRe - fRe * dIm) / den;
            zx -= qRe;
            zy -= qIm;

            if (!std::isfinite(zx) || !std::isfinite(zy))
            {
                return makeBgr(6, 6, 10);
            }
        }

        double finalNRe = 0.0, finalNIm = 0.0;
        powerOf(zx, zy, m_powerN, finalNRe, finalNIm);
        const double fr = finalNRe - 1.0;
        const double fi = finalNIm;
        const bool converged = (fr * fr + fi * fi) < 1e-4;
        if (!converged)
        {
            return makeBgr(6, 6, 10);
        }

        const double twoPi = 6.283185307179586;
        double angle = std::atan2(zy, zx);
        if (angle < 0.0) angle += twoPi;
        const double sector = twoPi / static_cast<double>(m_powerN);
        int rootIndex = static_cast<int>(std::floor((angle + sector * 0.5) / sector)) % m_powerN;
        if (rootIndex < 0) rootIndex += m_powerN;

        const int baseColors[6][3] = {
            {250, 86, 72}, {66, 180, 255}, {255, 216, 84},
            {128, 214, 130}, {195, 120, 255}, {255, 150, 210}
        };

        const double depth = Clamp01(static_cast<double>(iter) / static_cast<double>((std::max)(1, maxIter)));
        const double gain = 0.35 + 0.65 * (1.0 - depth);
        const int* bc = baseColors[rootIndex % 6];
        return makeBgr(
            static_cast<int>(bc[0] * gain),
            static_cast<int>(bc[1] * gain),
            static_cast<int>(bc[2] * gain));
    }

    double zx = 0.0;
    double zy = 0.0;
    int iter = 0;

    while ((zx * zx + zy * zy) <= 4.0 && iter < maxIter)
    {
        double powX = 0.0;
        double powY = 0.0;
        if (m_family == FAMILY_BURNING_SHIP)
        {
            applyPower(std::abs(zx), std::abs(zy), powX, powY);
        }
        else
        {
            applyPower(zx, zy, powX, powY);
        }

        const double nextX = powX + cx;
        const double nextY = powY + cy;
        if (!std::isfinite(nextX) || !std::isfinite(nextY))
        {
            break;
        }
        zx = nextX;
        zy = nextY;
        ++iter;
    }

    if (iter >= maxIter)
    {
        return colorFromNormalized(0.0, true);
    }

    const double magSq = zx * zx + zy * zy;
    const double log_zn = std::log((std::max)(magSq, 1e-12)) * 0.5;
    const double nu = std::log((std::max)(log_zn / std::log(2.0), 1e-12)) / std::log(2.0);
    const double smoothIter = static_cast<double>(iter) + 1.0 - nu;
    const double t = Clamp01(smoothIter / static_cast<double>(maxIter));

    return colorFromNormalized(t, false);
}

void Mandelbrot::applyPower(double zx, double zy, double& outX, double& outY) const
{
    if (m_powerN <= 2)
    {
        outX = zx * zx - zy * zy;
        outY = 2.0 * zx * zy;
        return;
    }

    double rx = zx;
    double ry = zy;
    double ax = zx;
    double ay = zy;
    for (int i = 1; i < m_powerN; ++i)
    {
        const double nx = rx * ax - ry * ay;
        const double ny = rx * ay + ry * ax;
        rx = nx;
        ry = ny;
    }
    outX = rx;
    outY = ry;
}

std::uint32_t Mandelbrot::colorFromNormalized(double t, bool inside) const
{
    if (inside)
    {
        if (m_palette == PALETTE_MONO)
        {
            return makeBgr(10, 10, 10);
        }
        return makeBgr(5, 8, 20);
    }

    const double x = Clamp01(t);

    if (m_palette == PALETTE_NEON)
    {
        if (x < 0.33)
        {
            return lerpColor(makeBgr(10, 5, 40), makeBgr(0, 190, 255), x / 0.33);
        }
        if (x < 0.66)
        {
            return lerpColor(makeBgr(0, 190, 255), makeBgr(255, 60, 210), (x - 0.33) / 0.33);
        }
        return lerpColor(makeBgr(255, 60, 210), makeBgr(255, 230, 40), (x - 0.66) / 0.34);
    }

    if (m_palette == PALETTE_MONO)
    {
        const int g = static_cast<int>(25.0 + x * 220.0);
        return makeBgr(g, g, g);
    }

    if (x < 0.4)
    {
        return lerpColor(makeBgr(0, 25, 80), makeBgr(0, 160, 220), x / 0.4);
    }
    if (x < 0.75)
    {
        return lerpColor(makeBgr(0, 160, 220), makeBgr(240, 230, 90), (x - 0.4) / 0.35);
    }
    return lerpColor(makeBgr(240, 230, 90), makeBgr(255, 90, 50), (x - 0.75) / 0.25);
}

std::uint32_t Mandelbrot::lerpColor(std::uint32_t c0, std::uint32_t c1, double t) const
{
    const double k = Clamp01(t);

    const int b0 = static_cast<int>(c0 & 0xFF);
    const int g0 = static_cast<int>((c0 >> 8) & 0xFF);
    const int r0 = static_cast<int>((c0 >> 16) & 0xFF);

    const int b1 = static_cast<int>(c1 & 0xFF);
    const int g1 = static_cast<int>((c1 >> 8) & 0xFF);
    const int r1 = static_cast<int>((c1 >> 16) & 0xFF);

    const int r = static_cast<int>(r0 + (r1 - r0) * k);
    const int g = static_cast<int>(g0 + (g1 - g0) * k);
    const int b = static_cast<int>(b0 + (b1 - b0) * k);

    return makeBgr(r, g, b);
}

std::uint32_t Mandelbrot::makeBgr(int r, int g, int b) const
{
    const std::uint32_t ur = static_cast<std::uint32_t>((std::min)(255, (std::max)(0, r)));
    const std::uint32_t ug = static_cast<std::uint32_t>((std::min)(255, (std::max)(0, g)));
    const std::uint32_t ub = static_cast<std::uint32_t>((std::min)(255, (std::max)(0, b)));

    return ub | (ug << 8) | (ur << 16);
}

void Mandelbrot::screenToComplex(int sx, int sy, double& x, double& y) const
{
    if (m_viewWidth <= 0 || m_viewHeight <= 0)
    {
        x = m_view.centerX;
        y = m_view.centerY;
        return;
    }

    double spanX = 0.0;
    double spanY = 0.0;
    currentSpan(spanX, spanY);

    x = m_view.centerX + ((static_cast<double>(sx) / static_cast<double>(m_viewWidth)) - 0.5) * spanX;
    y = m_view.centerY + ((static_cast<double>(sy) / static_cast<double>(m_viewHeight)) - 0.5) * spanY;
}

void Mandelbrot::complexToScreen(double x, double y, int& sx, int& sy) const
{
    double spanX = 0.0;
    double spanY = 0.0;
    currentSpan(spanX, spanY);

    const double nx = ((x - m_view.centerX) / spanX) + 0.5;
    const double ny = ((y - m_view.centerY) / spanY) + 0.5;

    sx = static_cast<int>(nx * static_cast<double>(m_viewWidth));
    sy = static_cast<int>(ny * static_cast<double>(m_viewHeight));
}

void Mandelbrot::currentSpan(double& spanX, double& spanY) const
{
    spanX = 3.5 / m_view.zoom;
    const double aspect = (m_viewWidth > 0) ? (static_cast<double>(m_viewHeight) / static_cast<double>(m_viewWidth)) : (2.0 / 3.5);
    spanY = spanX * aspect;
}

void Mandelbrot::normalizeBoxRect(CRect& rc) const
{
    if (rc.left > rc.right)
    {
        std::swap(rc.left, rc.right);
    }
    if (rc.top > rc.bottom)
    {
        std::swap(rc.top, rc.bottom);
    }

    const LONG minX = 0;
    const LONG maxX = static_cast<LONG>((std::max)(0, m_viewWidth));
    const LONG minY = 0;
    const LONG maxY = static_cast<LONG>((std::max)(0, m_viewHeight));

    rc.left = (std::max)(minX, (std::min)(maxX, rc.left));
    rc.right = (std::max)(minX, (std::min)(maxX, rc.right));
    rc.top = (std::max)(minY, (std::min)(maxY, rc.top));
    rc.bottom = (std::max)(minY, (std::min)(maxY, rc.bottom));
}

bool Mandelbrot::isDifferentView(const ViewState& a, const ViewState& b) const
{
    const double eps = 1e-9;
    return std::abs(a.centerX - b.centerX) > eps ||
        std::abs(a.centerY - b.centerY) > eps ||
        std::abs(a.zoom - b.zoom) > eps;
}

void Mandelbrot::commitCurrentView()
{
    if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_history.size()))
    {
        if (!isDifferentView(m_history[m_historyIndex], m_view))
        {
            return;
        }
    }

    if (m_historyIndex + 1 < static_cast<int>(m_history.size()))
    {
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }

    m_history.push_back(m_view);
    if (static_cast<int>(m_history.size()) > kMaxHistorySize)
    {
        m_history.erase(m_history.begin());
    }

    m_historyIndex = static_cast<int>(m_history.size()) - 1;
}

void Mandelbrot::applyView(const ViewState& view)
{
    m_view = view;
}
