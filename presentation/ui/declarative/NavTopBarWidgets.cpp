/*
 * 文件名：NavTopBarWidgets.cpp
 * 职责：声明式导航栏和顶栏组件的具体实现，包装运行时组件
 * 依赖：NavTopBarWidgets.h、运行时NavRail和TopBar组件
 * 线程：仅在UI线程使用
 * 备注：实现包装类，转发配置到运行时组件，确保行为一致性
 */

#include "NavTopBarWidgets.h"
#include "UiComponent.hpp"
#include "RenderUtils.hpp"
#include "IconCache.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>

namespace UI {

	/// NavRail包装组件：拥有并管理Ui::NavRail实例
	class NavRailComponent final : public IUiComponent {
	public:
		explicit NavRailComponent(
			fj::presentation::binding::INavDataProvider* dataProvider,
			int collapsedWidth, int expandedWidth,
			int iconSize, int itemHeight, int labelFontPx,
			const QString& expandSvg, const QString& collapseSvg,
			const Ui::NavPalette& palette, bool hasCustomPalette)
			: m_navRail(std::make_unique<Ui::NavRail>())
		{
			// 配置数据提供者
			if (dataProvider) {
				m_navRail->setDataProvider(dataProvider);
			}

			// 配置宽度
			m_navRail->setWidths(collapsedWidth, expandedWidth);

			// 配置图标大小
			m_navRail->setIconLogicalSize(iconSize);

			// 配置项目高度
			m_navRail->setItemHeight(itemHeight);

			// 配置标签字体
			m_navRail->setLabelFontPx(labelFontPx);

			// 配置切换图标
			if (!expandSvg.isEmpty() && !collapseSvg.isEmpty()) {
				m_navRail->setToggleSvgPaths(expandSvg, collapseSvg);
			}

			// 配置自定义色彩方案
			if (hasCustomPalette) {
				m_navRail->setPalette(palette);
			}
		}

		// IUiComponent interface - 转发到包装的NavRail
		void updateLayout(const QSize& windowSize) override {
			m_navRail->updateLayout(windowSize);
		}

		void updateResourceContext(IconCache& iconCache, QOpenGLFunctions* gl, const float devicePixelRatio) override {
			m_navRail->updateResourceContext(iconCache, gl, devicePixelRatio);
		}

		void append(Render::FrameData& fd) const override {
			m_navRail->append(fd);
		}

		bool onMousePress(const QPoint& pos) override {
			return m_navRail->onMousePress(pos);
		}

		bool onMouseMove(const QPoint& pos) override {
			return m_navRail->onMouseMove(pos);
		}

		bool onMouseRelease(const QPoint& pos) override {
			return m_navRail->onMouseRelease(pos);
		}

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
			return m_navRail->onWheel(pos, angleDelta);
		}

		bool tick() override {
			return m_navRail->tick();
		}

		QRect bounds() const override {
			return m_navRail->bounds();
		}

		void onThemeChanged(const bool isDark) override {
			m_navRail->onThemeChanged(isDark);
		}

	private:
		std::unique_ptr<Ui::NavRail> m_navRail;
	};

	/// TopBar包装组件：直接管理Ui::Button实例并实现动画状态机
	class TopBarComponent final : public IUiComponent {
	public:
		explicit TopBarComponent(
			bool followSystem, bool animateFollow, float cornerRadius,
			const QString& svgThemeDark, const QString& svgThemeLight,
			const QString& svgFollowOn, const QString& svgFollowOff,
			const QString& svgMin, const QString& svgMax, const QString& svgClose,
			const UiTopBar::Palette& palette, bool hasCustomPalette,
			std::function<void()> themeToggleCallback,
			std::function<void()> onMinimize,
			std::function<void()> onMaxRestore,
			std::function<void()> onClose,
			std::function<void()> onFollowToggle)
			: m_followSystem(followSystem)
			, m_cornerRadius(cornerRadius)
			, m_svgThemeDark(svgThemeDark)
			, m_svgThemeLight(svgThemeLight)
			, m_svgFollowOn(svgFollowOn)
			, m_svgFollowOff(svgFollowOff)
			, m_svgMin(svgMin)
			, m_svgMax(svgMax)
			, m_svgClose(svgClose)
			, m_palette(palette)
			, m_hasCustomPalette(hasCustomPalette)
			, m_themeToggleCallback(std::move(themeToggleCallback))
			, m_onMinimize(std::move(onMinimize))
			, m_onMaxRestore(std::move(onMaxRestore))
			, m_onClose(std::move(onClose))
			, m_onFollowToggle(std::move(onFollowToggle))
		{
			// 初始化按钮圆角半径
			m_btnTheme.setCornerRadius(cornerRadius);
			m_btnFollow.setCornerRadius(cornerRadius);
			m_btnMin.setCornerRadius(cornerRadius);
			m_btnMax.setCornerRadius(cornerRadius);
			m_btnClose.setCornerRadius(cornerRadius);

			// 初始化动画状态
			if (animateFollow) {
				// 启动动画：从相反状态开始，动画到目标状态
				// 如果目标是followSystem=true，则从false开始动画
				// 如果目标是followSystem=false，则从true开始动画
				bool startState = !followSystem;
				m_followSystem = startState; // 临时设置为起始状态
				m_themeAlpha = startState ? 0.0f : 1.0f;
				m_followSlide = startState ? 1.0f : 0.0f;
				
				// 启动动画序列到目标状态
				m_followSystem = followSystem; // 设置真正的目标状态
				startAnimSequence(followSystem);
			} else {
				// 无动画，直接设置最终状态
				m_themeAlpha = followSystem ? 0.0f : 1.0f;
				m_followSlide = followSystem ? 1.0f : 0.0f;
			}
			
			m_btnTheme.setOpacity(m_themeAlpha);
			m_btnTheme.setEnabled(themeInteractive());

			// 设置默认主题
			onThemeChanged(true); // 默认暗色主题
		}

		// IUiComponent interface - 直接管理Ui::Button实例
		void updateLayout(const QSize& windowSize) override {
			constexpr int margin = 12;
			constexpr int btnSize = 28;
			constexpr int gap = 8;

			int x = windowSize.width() - margin - btnSize;
			constexpr int y = margin;

			m_btnClose.setBaseRect(QRect(x, y, btnSize, btnSize));
			x -= (btnSize + gap);
			m_btnMax.setBaseRect(QRect(x, y, btnSize, btnSize));
			x -= (btnSize + gap);
			m_btnMin.setBaseRect(QRect(x, y, btnSize, btnSize));
			x -= (btnSize + gap);
			m_btnTheme.setBaseRect(QRect(x, y, btnSize, btnSize));
			x -= (btnSize + gap);
			m_btnFollow.setBaseRect(QRect(x, y, btnSize, btnSize));

			// 设置动画状态
			if (m_animPhase == AnimPhase::Idle) {
				m_themeAlpha = m_followSystem ? 0.0f : 1.0f;
				m_followSlide = m_followSystem ? 1.0f : 0.0f;
			}
			m_btnTheme.setOpacity(m_themeAlpha);

			const float deltaX = static_cast<float>(m_btnTheme.baseRect().x() - m_btnFollow.baseRect().x());
			m_btnFollow.setOffset(QPointF(deltaX * std::clamp(m_followSlide, 0.0f, 1.0f), 0.0f));

			m_btnTheme.setEnabled(themeInteractive());

			// 计算边界
			QRect u = m_btnTheme.visualRectF().toRect();
			u = u.united(m_btnFollow.visualRectF().toRect());
			u = u.united(m_btnMin.visualRectF().toRect());
			u = u.united(m_btnMax.visualRectF().toRect());
			u = u.united(m_btnClose.visualRectF().toRect());
			m_bounds = u;
		}

		void updateResourceContext(IconCache& iconCache, QOpenGLFunctions* gl, const float devicePixelRatio) override {
			m_cache = &iconCache; 
			m_gl = gl; 
			m_dpr = std::max(0.5f, devicePixelRatio);

			if (!m_cache || !m_gl) return;

			// 设置按钮调色板
			const auto palette = m_hasCustomPalette ? m_palette : (m_dark ? getDarkPalette() : getLightPalette());
			m_btnTheme.setPalette(palette.bg, palette.bgHover, palette.bgPressed, palette.icon);
			m_btnFollow.setPalette(palette.bg, palette.bgHover, palette.bgPressed, palette.icon);
			m_btnMin.setPalette(palette.bg, palette.bgHover, palette.bgPressed, palette.icon);
			m_btnMax.setPalette(palette.bg, palette.bgHover, palette.bgPressed, palette.icon);
			m_btnClose.setPalette(palette.bg, palette.bgHover, palette.bgPressed, palette.icon);

			// 设置图标
			const QString themePath = m_dark ? m_svgThemeDark : m_svgThemeLight;
			const QString themeBaseKey = m_dark ? "theme_sun" : "theme_moon";
			setupSvgIcon(m_btnTheme, themeBaseKey, themePath, 18);

			const QString followPath = m_followSystem ? m_svgFollowOn : m_svgFollowOff;
			const QString followBaseKey = m_followSystem ? "follow_on" : "follow_off";
			setupSvgIcon(m_btnFollow, followBaseKey, followPath, 18);

			setupSvgIcon(m_btnMin, "sys_min", m_svgMin, 16);
			setupSvgIcon(m_btnMax, "sys_max", m_svgMax, 16);
			setupSvgIcon(m_btnClose, "sys_close", m_svgClose, 16);
		}

		void append(Render::FrameData& fd) const override {
			m_btnTheme.append(fd);
			m_btnFollow.append(fd);
			m_btnMin.append(fd);
			m_btnMax.append(fd);
			m_btnClose.append(fd);
		}

		bool onMousePress(const QPoint& pos) override {
			m_btnTheme.setEnabled(themeInteractive());
			bool handled = false;
			handled = m_btnTheme.onMousePress(pos) || handled;
			handled = m_btnFollow.onMousePress(pos) || handled;
			handled = m_btnMin.onMousePress(pos) || handled;
			handled = m_btnMax.onMousePress(pos) || handled;
			handled = m_btnClose.onMousePress(pos) || handled;
			return handled;
		}

		bool onMouseMove(const QPoint& pos) override {
			m_btnTheme.setEnabled(themeInteractive());
			bool changed = false;
			changed = m_btnTheme.onMouseMove(pos) || changed;
			changed = m_btnFollow.onMouseMove(pos) || changed;
			changed = m_btnMin.onMouseMove(pos) || changed;
			changed = m_btnMax.onMouseMove(pos) || changed;
			changed = m_btnClose.onMouseMove(pos) || changed;
			return changed;
		}

		bool onMouseRelease(const QPoint& pos) override {
			m_btnTheme.setEnabled(themeInteractive());
			bool clickedTheme = false, clickedFollow = false;
			bool handled = false;

			handled = m_btnTheme.onMouseRelease(pos, clickedTheme) || handled;
			handled = m_btnFollow.onMouseRelease(pos, clickedFollow) || handled;

			bool cMin = false, cMax = false, cClose = false;
			handled = m_btnMin.onMouseRelease(pos, cMin) || handled;
			handled = m_btnMax.onMouseRelease(pos, cMax) || handled;
			handled = m_btnClose.onMouseRelease(pos, cClose) || handled;

			m_clickThemePending = m_clickThemePending || clickedTheme;
			m_clickFollowPending = m_clickFollowPending || clickedFollow;
			m_clickMinPending = m_clickMinPending || cMin;
			m_clickMaxPending = m_clickMaxPending || cMax;
			m_clickClosePending = m_clickClosePending || cClose;

			return handled || clickedTheme || clickedFollow || cMin || cMax || cClose;
		}

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
			// 顶栏不处理滚轮事件
			Q_UNUSED(pos)
			Q_UNUSED(angleDelta)
			return false;
		}

		bool tick() override {
			// 检查主题切换事件
			bool clickedTheme = false, clickedFollow = false;
			if (takeActions(clickedTheme, clickedFollow)) {
				if (clickedTheme && m_themeToggleCallback) {
					m_themeToggleCallback();
				}
				if (clickedFollow && m_onFollowToggle) {
					m_onFollowToggle();
				}
			}

			// 检查系统按钮事件
			bool clickedMin = false, clickedMaxRestore = false, clickedClose = false;
			if (takeSystemActions(clickedMin, clickedMaxRestore, clickedClose)) {
				if (clickedMin && m_onMinimize) {
					m_onMinimize();
				}
				if (clickedMaxRestore && m_onMaxRestore) {
					m_onMaxRestore();
				}
				if (clickedClose && m_onClose) {
					m_onClose();
				}
			}

			// 处理动画
			const bool active = m_animPhase != AnimPhase::Idle;
			if (!active) return false;
			if (!m_animClock.isValid()) m_animClock.start();

			const qint64 now = m_animClock.elapsed();
			const float  tRaw = m_animDurationMs > 0 ? static_cast<float>(now - m_phaseStartMs) / static_cast<float>(m_animDurationMs) : 1.0f;
			const float  t = std::clamp(tRaw, 0.0f, 1.0f);
			const float  e = easeInOut(t);

			switch (m_animPhase) {
			case AnimPhase::HideTheme_FadeOut:
				m_themeAlpha = lerp(m_phaseStartAlpha, 0.0f, e);
				if (t >= 1.0f) { m_phaseStartSlide = m_followSlide; beginPhase(AnimPhase::MoveFollow_Right, 200); }
				break;
			case AnimPhase::MoveFollow_Right:
				m_followSlide = lerp(m_phaseStartSlide, 1.0f, e);
				if (t >= 1.0f) { m_animPhase = AnimPhase::Idle; }
				break;
			case AnimPhase::MoveFollow_Left:
				m_followSlide = lerp(m_phaseStartSlide, 0.0f, e);
				if (t >= 1.0f) { m_phaseStartAlpha = m_themeAlpha; beginPhase(AnimPhase::ShowTheme_FadeIn, 160); }
				break;
			case AnimPhase::ShowTheme_FadeIn:
				m_themeAlpha = lerp(m_phaseStartAlpha, 1.0f, e);
				if (t >= 1.0f) { m_animPhase = AnimPhase::Idle; }
				break;
			case AnimPhase::Idle:
			default:
				break;
			}

			m_btnTheme.setOpacity(std::clamp(m_themeAlpha, 0.0f, 1.0f));
			const float deltaX = static_cast<float>(m_btnTheme.baseRect().x() - m_btnFollow.baseRect().x());
			m_btnFollow.setOffset(QPointF(deltaX * std::clamp(m_followSlide, 0.0f, 1.0f), 0.0f));
			m_btnTheme.setEnabled(themeInteractive());

			return m_animPhase != AnimPhase::Idle;
		}

		QRect bounds() const override {
			return m_bounds;
		}

		void onThemeChanged(const bool isDark) override {
			m_dark = isDark;
			// 更新资源上下文会重新设置调色板和图标
		}

		bool takeActions(bool& clickedTheme, bool& clickedFollow) {
			clickedTheme = m_clickThemePending;
			clickedFollow = m_clickFollowPending;
			const bool any = clickedTheme || clickedFollow;
			m_clickThemePending = m_clickFollowPending = false;
			return any;
		}

		bool takeSystemActions(bool& clickedMin, bool& clickedMaxRestore, bool& clickedClose) {
			clickedMin = m_clickMinPending;
			clickedMaxRestore = m_clickMaxPending;
			clickedClose = m_clickClosePending;
			const bool any = clickedMin || clickedMaxRestore || clickedClose;
			m_clickMinPending = m_clickMaxPending = m_clickClosePending = false;
			return any;
		}

	private:
		// 动画状态枚举
		enum class AnimPhase : uint8_t { 
			Idle, 
			HideTheme_FadeOut, 
			MoveFollow_Right, 
			MoveFollow_Left, 
			ShowTheme_FadeIn 
		};

		// 动画工具函数
		static float easeInOut(float t) {
			t = std::clamp(t, 0.0f, 1.0f);
			return t * t * (3.0f - 2.0f * t);
		}

		static float lerp(const float a, const float b, const float t) { 
			return a + (b - a) * t; 
		}

		// 动画控制方法
		void setFollowSystem(const bool on, const bool animate) {
			if (m_followSystem == on && !animate) return; // No change and no forced animation
			
			if (!animate) {
				// 立即设置状态，无动画
				m_followSystem = on;
				m_animPhase = AnimPhase::Idle;
				m_themeAlpha = on ? 0.0f : 1.0f;
				m_followSlide = on ? 1.0f : 0.0f;
				m_btnTheme.setOpacity(m_themeAlpha);
				m_btnTheme.setEnabled(themeInteractive());
				return;
			}
			
			// 启用动画：只有状态真正改变时才启动动画
			if (m_followSystem != on) {
				m_followSystem = on;
				startAnimSequence(on);
			}
		}

		void startAnimSequence(const bool followOn) {
			if (!m_animClock.isValid()) m_animClock.start();
			m_phaseStartAlpha = m_themeAlpha;
			m_phaseStartSlide = m_followSlide;

			if (followOn) beginPhase(AnimPhase::HideTheme_FadeOut, 160);
			else          beginPhase(AnimPhase::MoveFollow_Left, 180);
		}

		void beginPhase(const AnimPhase ph, const int durationMs) {
			m_animPhase = ph;
			m_animDurationMs = durationMs;
			m_phaseStartMs = m_animClock.elapsed();
		}

		bool themeInteractive() const {
			if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
				return m_themeAlpha > 0.6f;
			}
			return m_themeAlpha > 0.4f;
		}

		static UiTopBar::Palette getDarkPalette() {
			return UiTopBar::Palette{
				.bg = QColor(52,63,76,120),
				.bgHover = QColor(66,78,92,200),
				.bgPressed = QColor(58,70,84,220),
				.icon = QColor(255,255,255,255)
			};
		}

		static UiTopBar::Palette getLightPalette() {
			return UiTopBar::Palette{
				.bg = QColor(240,243,247,200),
				.bgHover = QColor(232,237,242,220),
				.bgPressed = QColor(225,230,236,230),
				.icon = QColor(60,64,72,255)
			};
		}

		void setupSvgIcon(Ui::Button& btn, const QString& baseKey, const QString& svgPath, int iconLogical) {
			if (!m_cache || !m_gl || svgPath.isEmpty()) return;
			
			btn.setIconPainter([this, baseKey, svgPath, iconLogical](const QRectF& r, Render::FrameData& fd, const QColor& iconColor, float) {
				if (!m_cache || !m_gl) return;
				const int px = std::lround(iconLogical * m_dpr);
				const QString key = RenderUtils::makeIconCacheKey(baseKey, px);

				const QByteArray svg = RenderUtils::loadSvgCached(svgPath);
				const int tex = m_cache->ensureSvgPx(key, svg, QSize(px, px), m_gl);
				const QSize texSz = m_cache->textureSizePx(tex);

				const QRectF dst(r.center().x() - iconLogical * 0.5, r.center().y() - iconLogical * 0.5, iconLogical, iconLogical);
				fd.images.push_back(Render::ImageCmd{
					.dstRect = dst, .textureId = tex, .srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
					.tint = iconColor, .clipRect = r
					});
				});
		}

	private:
		// 状态变量
		bool m_dark{ true };
		bool m_followSystem{ false };
		float m_cornerRadius{ 6.0f };

		// 按钮实例
		Ui::Button m_btnTheme;
		Ui::Button m_btnFollow;
		Ui::Button m_btnMin;
		Ui::Button m_btnMax;
		Ui::Button m_btnClose;

		QRect m_bounds;

		// 动画状态
		AnimPhase     m_animPhase{ AnimPhase::Idle };
		int           m_animDurationMs{ 0 };
		qint64        m_phaseStartMs{ 0 };
		QElapsedTimer m_animClock;

		float m_themeAlpha{ 1.0f };
		float m_followSlide{ 0.0f };
		float m_phaseStartAlpha{ 1.0f };
		float m_phaseStartSlide{ 0.0f };

		// 配置参数
		QString m_svgThemeDark;
		QString m_svgThemeLight;
		QString m_svgFollowOn;
		QString m_svgFollowOff;
		QString m_svgMin;
		QString m_svgMax;
		QString m_svgClose;
		UiTopBar::Palette m_palette;
		bool m_hasCustomPalette{ false };

		// 回调函数
		std::function<void()> m_themeToggleCallback;
		std::function<void()> m_onMinimize;
		std::function<void()> m_onMaxRestore;
		std::function<void()> m_onClose;
		std::function<void()> m_onFollowToggle;

		// 资源上下文
		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };

		// 事件状态
		bool m_clickThemePending{ false };
		bool m_clickFollowPending{ false };
		bool m_clickMinPending{ false };
		bool m_clickMaxPending{ false };
		bool m_clickClosePending{ false };
	};

	// NavRail widget implementation
	std::unique_ptr<IUiComponent> NavRail::build() const {
		auto component = std::make_unique<NavRailComponent>(
			m_dataProvider,
			m_collapsedWidth, m_expandedWidth,
			m_iconSize, m_itemHeight, m_labelFontPx,
			m_expandSvg, m_collapseSvg,
			m_palette, m_hasCustomPalette
		);

		// 应用装饰器（支持Widget基类的padding、margin等）
		return decorate(std::move(component));
	}

	// TopBar widget implementation
	std::unique_ptr<IUiComponent> TopBar::build() const {
		auto component = std::make_unique<TopBarComponent>(
			m_followSystem, m_animateFollow, m_cornerRadius,
			m_svgThemeDark, m_svgThemeLight,
			m_svgFollowOn, m_svgFollowOff,
			m_svgMin, m_svgMax, m_svgClose,
			m_palette, m_hasCustomPalette,
			m_themeToggleCallback,
			m_onMinimize, m_onMaxRestore, m_onClose, m_onFollowToggle
		);

		// 应用装饰器（支持Widget基类的padding、margin等）
		return decorate(std::move(component));
	}

} // namespace UI