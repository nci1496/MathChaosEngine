#pragma once

#include <afxwin.h>

#include <cstdint>
#include <vector>

class JuliaSet
{
public:
    enum PaletteId
    {
        PALETTE_SCIENTIFIC = 0,
        PALETTE_NEON = 1,
        PALETTE_MONO = 2
    };

    enum QualityPreset
    {
        QUALITY_FAST = 0,
        QUALITY_BALANCED = 1,
        QUALITY_DETAIL = 2
    };

    enum RenderStage
    {
        STAGE_PREVIEW = 0,
        STAGE_REFINING = 1,
        STAGE_FINAL = 2
    };

    enum FractalFamily
    {
        FAMILY_MULTIBROT = 0,
        FAMILY_BURNING_SHIP = 1,
        FAMILY_NEWTON = 2
    };

    JuliaSet();

    void reset();
    void update(double dt);
    void render(CDC* pDC, const CRect& targetRect);

    void setConstant(double realPart, double imagPart);
    void setPalette(PaletteId palette);
    void setQuality(QualityPreset quality);
    void setPower(int n);
    void setFamily(FractalFamily family);
    void onMouseWheel(int x, int y, int delta);
    void onMouseDragBegin(int x, int y, bool boxZoomMode);
    void onMouseDragMove(int x, int y);
    void onMouseDragEnd(int x, int y);

    bool consumePixelsChanged();
    bool isRendering() const;
    RenderStage getRenderStage() const;
    double getRenderProgress() const;
    double getConstantReal() const;
    double getConstantImag() const;
    int getPower() const;
    FractalFamily getFamily() const;
    bool isBoxZoomActive() const;
    CRect getBoxZoomRect() const;

private:
    int m_viewWidth;
    int m_viewHeight;
    double m_centerX;
    double m_centerY;
    double m_zoom;
    double m_cRe;
    double m_cIm;
    int m_maxIterCurrent;
    bool m_dirty;
    bool m_pixelsChanged;
    PaletteId m_palette;
    QualityPreset m_quality;
    RenderStage m_stage;
    double m_progress;
    int m_previewStep;
    int m_previewRow;
    int m_tileSize;
    int m_timeBudgetMs;
    int m_iterBase;
    int m_iterCap;
    int m_powerN;
    FractalFamily m_family;
    std::vector<CRect> m_tiles;
    size_t m_nextTile;
    std::vector<std::uint32_t> m_pixels;
    BITMAPINFO m_bmi;
    bool m_dragging;
    bool m_boxZoomMode;
    bool m_dragMoved;
    CPoint m_dragStart;
    CPoint m_dragCurrent;
    double m_dragStartCenterX;
    double m_dragStartCenterY;
    double m_dragStartZoom;

    void ensureBufferSize(int width, int height);
    void beginProgressiveRender();
    bool renderWorkUnit();
    bool renderPreviewRow();
    bool renderNextTile();
    void buildTileQueue();
    void applyQualityPreset();
    int computeAdaptiveMaxIter() const;
    int clampPower(int n) const;
    void screenToComplex(int sx, int sy, double& x, double& y) const;
    void currentSpan(double& spanX, double& spanY) const;
    void normalizeBoxRect(CRect& rc) const;
    std::uint32_t computeColor(double zx0, double zy0, int maxIter) const;
    void applyPower(double zx, double zy, double& outX, double& outY) const;
    std::uint32_t colorFromNormalized(double t, bool inside) const;
    std::uint32_t lerpColor(std::uint32_t c0, std::uint32_t c1, double t) const;
    std::uint32_t makeBgr(int r, int g, int b) const;
};
