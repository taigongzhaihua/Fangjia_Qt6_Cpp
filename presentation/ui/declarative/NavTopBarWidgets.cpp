/*
 * 文件名：NavTopBarWidgets.cpp
 * 职责：声明式导航栏和顶栏组件的具体实现，包装运行时组件
 * 依赖：NavTopBarWidgets.h、运行时NavRail和TopBar组件
 * 线程：仅在UI线程使用
 * 备注：实现包装类，转发配置到运行时组件，确保行为一致性
 */

#include "NavTopBarWidgets.h"
#include "UiComponent.hpp"
#include <memory>

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

	/// TopBar包装组件：拥有并管理UiTopBar实例
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
			: m_topBar(std::make_unique<UiTopBar>())
			, m_themeToggleCallback(std::move(themeToggleCallback))
			, m_onMinimize(std::move(onMinimize))
			, m_onMaxRestore(std::move(onMaxRestore))
			, m_onClose(std::move(onClose))
			, m_onFollowToggle(std::move(onFollowToggle))
		{
			// 配置跟随系统
			m_topBar->setFollowSystem(followSystem, animateFollow);

			// 配置圆角半径
			m_topBar->setCornerRadius(cornerRadius);

			// 配置主题切换图标
			if (!svgThemeDark.isEmpty() && !svgThemeLight.isEmpty()) {
				m_topBar->setSvgPaths(svgThemeDark, svgThemeLight, svgFollowOn, svgFollowOff);
			}

			// 配置系统按钮图标
			if (!svgMin.isEmpty() && !svgMax.isEmpty() && !svgClose.isEmpty()) {
				m_topBar->setSystemButtonSvgPaths(svgMin, svgMax, svgClose);
			}

			// 配置自定义色彩方案
			if (hasCustomPalette) {
				m_topBar->setPalette(palette);
			}
		}

		// IUiComponent interface - 转发到包装的TopBar
		void updateLayout(const QSize& windowSize) override {
			m_topBar->updateLayout(windowSize);
		}

		void updateResourceContext(IconCache& iconCache, QOpenGLFunctions* gl, const float devicePixelRatio) override {
			m_topBar->updateResourceContext(iconCache, gl, devicePixelRatio);
		}

		void append(Render::FrameData& fd) const override {
			m_topBar->append(fd);
		}

		bool onMousePress(const QPoint& pos) override {
			return m_topBar->onMousePress(pos);
		}

		bool onMouseMove(const QPoint& pos) override {
			return m_topBar->onMouseMove(pos);
		}

		bool onMouseRelease(const QPoint& pos) override {
			return m_topBar->onMouseRelease(pos);
		}

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
			return m_topBar->onWheel(pos, angleDelta);
		}

		bool tick() override {
			// 检查主题切换事件
			bool clickedTheme = false, clickedFollow = false;
			if (m_topBar->takeActions(clickedTheme, clickedFollow)) {
				if (clickedTheme && m_themeToggleCallback) {
					m_themeToggleCallback();
				}
				if (clickedFollow && m_onFollowToggle) {
					m_onFollowToggle();
				}
			}

			// 检查系统按钮事件
			bool clickedMin = false, clickedMaxRestore = false, clickedClose = false;
			if (m_topBar->takeSystemActions(clickedMin, clickedMaxRestore, clickedClose)) {
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

			return m_topBar->tick();
		}

		QRect bounds() const override {
			return m_topBar->bounds();
		}

		void onThemeChanged(const bool isDark) override {
			m_topBar->onThemeChanged(isDark);
		}

	private:
		std::unique_ptr<UiTopBar> m_topBar;
		std::function<void()> m_themeToggleCallback;
		std::function<void()> m_onMinimize;
		std::function<void()> m_onMaxRestore;
		std::function<void()> m_onClose;
		std::function<void()> m_onFollowToggle;
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