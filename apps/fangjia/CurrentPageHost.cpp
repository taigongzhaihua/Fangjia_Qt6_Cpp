#include "CurrentPageHost.h"
#include "UiPage.h"

void CurrentPageHost::setViewportRect(const QRect& r) {
    m_viewport = r;
    
    // 将视口信息转发给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        currentPage->setViewportRect(r);
    }
}

void CurrentPageHost::updateLayout(const QSize& windowSize) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        currentPage->updateLayout(windowSize);
    }
}

void CurrentPageHost::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        currentPage->updateResourceContext(cache, gl, devicePixelRatio);
    }
}

void CurrentPageHost::append(Render::FrameData& fd) const {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        currentPage->append(fd);
    }
}

bool CurrentPageHost::onMousePress(const QPoint& pos) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        return currentPage->onMousePress(pos);
    }
    return false;
}

bool CurrentPageHost::onMouseMove(const QPoint& pos) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        return currentPage->onMouseMove(pos);
    }
    return false;
}

bool CurrentPageHost::onMouseRelease(const QPoint& pos) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        return currentPage->onMouseRelease(pos);
    }
    return false;
}

bool CurrentPageHost::onWheel(const QPoint& pos, const QPoint& angleDelta) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        return currentPage->onWheel(pos, angleDelta);
    }
    return false;
}

bool CurrentPageHost::tick() {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        return currentPage->tick();
    }
    return false;
}

QRect CurrentPageHost::bounds() const {
    // 返回分配的视口区域作为边界
    // 如果当前页面有更精确的边界，也可以委托给它
    if (auto* currentPage = m_router.currentPage()) {
        return currentPage->bounds();
    }
    return m_viewport;
}

void CurrentPageHost::onThemeChanged(bool isDark) {
    // 委托给当前页面
    if (auto* currentPage = m_router.currentPage()) {
        currentPage->onThemeChanged(isDark);
    }
}