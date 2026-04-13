#pragma once

#include "../core/Module.h"

#include <cstdint>
#include <vector>

class Mandelbrot : public Module
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

    Mandelbrot();

    void reset() override;
    void update(double dt) override;
    void render(CDC* pDC) override;
    void onMouseDown(int x, int y) override;
    void onMouseMove(int x, int y) override;

    void onMouseWheel(int x, int y, int delta);
    void onMouseDragBegin(int x, int y, bool boxZoomMode);
    void onMouseDragMove(int x, int y);
    void onMouseDragEnd(int x, int y);

    void zoomOut();
    void goBack();
    void goHome();

    void setPalette(PaletteId palette);
    void setQuality(QualityPreset quality);

    bool isRendering() const;
    bool consumePixelsChanged();
    bool isBoxZoomActive() const;
    CRect getBoxZoomRect() const;
    RenderStage getRenderStage() const;
    double getRenderProgress() const;
    PaletteId getPalette() const;
    QualityPreset getQuality() const;
    double getZoom() const;

private:
    struct ViewState
    {
        double centerX;
        double centerY;
        double zoom;
    };

    int m_viewWidth;
    int m_viewHeight;
    ViewState m_view;
    int m_maxIterCurrent;
    bool m_dirty;
    std::vector<std::uint32_t> m_pixels;
    BITMAPINFO m_bmi;
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
    std::vector<CRect> m_tiles;
    size_t m_nextTile;

    bool m_dragging;
    bool m_boxZoomMode;
    bool m_dragMoved;
    CPoint m_dragStart;
    CPoint m_dragCurrent;
    ViewState m_dragStartView;
    ViewState m_beforeInteractionView;

    std::vector<ViewState> m_history;
    int m_historyIndex;

    void ensureBufferSize(int width, int height);
    void beginProgressiveRender();
    bool renderWorkUnit();
    bool renderPreviewRow();
    bool renderNextTile();
    void buildTileQueue();
    void applyQualityPreset();
    int computeAdaptiveMaxIter() const;

    std::uint32_t computeColor(double cx, double cy, int maxIter) const;
    std::uint32_t colorFromNormalized(double t, bool inside) const;
    std::uint32_t lerpColor(std::uint32_t c0, std::uint32_t c1, double t) const;
    std::uint32_t makeBgr(int r, int g, int b) const;

    void screenToComplex(int sx, int sy, double& x, double& y) const;
    void complexToScreen(double x, double y, int& sx, int& sy) const;
    void currentSpan(double& spanX, double& spanY) const;
    void normalizeBoxRect(CRect& rc) const;

    bool isDifferentView(const ViewState& a, const ViewState& b) const;
    void commitCurrentView();
    void applyView(const ViewState& view);
};
