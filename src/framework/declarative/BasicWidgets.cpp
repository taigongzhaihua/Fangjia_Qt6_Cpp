#include "BasicWidgets.h"
#include "IconLoader.h"
#include "Layouts.h"
#include "RenderData.hpp"
#include "UiBoxLayout.h"
#include <cmath>
#include <memory>
#include <qcolor.h>
#include <qfont.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <utility>

namespace UI {

	// 简单的文本组件实现
	class TextComponent : public IUiComponent, public IUiContent {
	public:
		TextComponent(const QString& text, QColor color, bool autoColor, int fontSize, QFont::Weight weight, Qt::Alignment align)
			: m_text(text), m_color(color), m_autoColor(autoColor), m_fontSize(fontSize), m_fontWeight(weight), m_alignment(align) {
		}

		// IUiContent
		void setViewportRect(const QRect& r) override { m_bounds = r; }

		void updateLayout(const QSize&) override {}

		void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override {
			m_loader = &loader;
			m_gl = gl;
			m_dpr = dpr;
		}

		void append(Render::FrameData& fd) const override {
			if (!m_loader || !m_gl || m_text.isEmpty()) return;

			QFont font;
			font.setPixelSize(std::lround(m_fontSize * m_dpr));
			font.setWeight(m_fontWeight);

			const QString key = QString("text_%1_%2_%3").arg(m_text).arg(m_fontSize).arg(m_color.name(QColor::HexArgb));
			const int tex = m_loader->ensureTextPx(key, font, m_text, m_color, m_gl);
			const QSize ts = m_loader->textureSizePx(tex);

			const float w = ts.width() / m_dpr;
			const float h = ts.height() / m_dpr;

			QRectF dst(m_bounds.x(), m_bounds.y(), w, h);

			// 应用对齐
			if (m_alignment & Qt::AlignHCenter) {
				dst.moveLeft(m_bounds.center().x() - w / 2);
			}
			else if (m_alignment & Qt::AlignRight) {
				dst.moveLeft(m_bounds.right() - w);
			}

			if (m_alignment & Qt::AlignVCenter) {
				dst.moveTop(m_bounds.center().y() - h / 2);
			}
			else if (m_alignment & Qt::AlignBottom) {
				dst.moveTop(m_bounds.bottom() - h);
			}

			fd.images.push_back(Render::ImageCmd{
				.dstRect = dst,
				.textureId = tex,
				.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
				.tint = QColor(255,255,255,255)
				});
		}

		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }
		QRect bounds() const override { return m_bounds; }

		void onThemeChanged(bool isDark) override {
			if (m_autoColor) {
				// 根据主题自动设色
				m_color = isDark ? QColor(240, 245, 250) : QColor(30, 35, 40);
			}
		}

		void setBounds(const QRect& bounds) { m_bounds = bounds; }

	private:
		QString m_text;
		QColor m_color;
		bool m_autoColor{ true };
		int m_fontSize;
		QFont::Weight m_fontWeight;
		Qt::Alignment m_alignment;
		QRect m_bounds;

		IconLoader* m_loader{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

	std::unique_ptr<IUiComponent> Text::build() const {
		auto comp = std::make_unique<TextComponent>(m_text, m_color, m_autoColor, m_fontSize, m_fontWeight, m_alignment);
		return decorate(std::move(comp));
	}

	// 图标组件实现（保留：如有需要也可按 m_autoColor 处理主题色）
	class IconComponent : public IUiComponent, public IUiContent {
	public:
		IconComponent(const QString& path, const QColor& color, int size, bool /*autoColor*/)
			: m_path(path), m_color(color), m_size(size) {
		}

		// IUiContent
		void setViewportRect(const QRect& r) override { m_bounds = r; }

		void updateLayout(const QSize&) override {}

		void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override {
			m_loader = &loader;
			m_gl = gl;
			m_dpr = dpr;
		}

		void append(Render::FrameData& fd) const override {
			if (!m_loader || !m_gl || m_path.isEmpty()) return;

			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(m_bounds.center().x() - m_size / 2.0f,
							  m_bounds.center().y() - m_size / 2.0f,
							  m_size, m_size),
				.radiusPx = m_size / 4.0f,
				.color = m_color
				});
		}

		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }
		QRect bounds() const override { return m_bounds; }

		void setBounds(const QRect& bounds) { m_bounds = bounds; }

	private:
		QString m_path;
		QColor m_color;
		int m_size;
		QRect m_bounds;

		IconLoader* m_loader{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

	std::unique_ptr<IUiComponent> Icon::build() const {
		auto comp = std::make_unique<IconComponent>(m_path, m_color, m_size, m_autoColor);
		return decorate(std::move(comp));
	}


	// 容器组件实现
	std::unique_ptr<IUiComponent> Container::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);

		// 将 Container 的 alignment 同时用作：
		// - 主轴对齐（Vertical 主轴为竖直方向）
		// - 子项交叉轴对齐（已有逻辑）
		auto toBoxMain = [](Alignment a) {
			switch (a) {
			case Alignment::Start: return UiBoxLayout::MainAlignment::Start;
			case Alignment::Center: return UiBoxLayout::MainAlignment::Center;
			case Alignment::End: return UiBoxLayout::MainAlignment::End;
			default: return UiBoxLayout::MainAlignment::Start;
			}
			};
		auto toBoxCross = [](Alignment a) {
			switch (a) {
			case Alignment::Start: return UiBoxLayout::Alignment::Start;
			case Alignment::Center: return UiBoxLayout::Alignment::Center;
			case Alignment::End: return UiBoxLayout::Alignment::End;
			case Alignment::Stretch: return UiBoxLayout::Alignment::Stretch;
			default: return UiBoxLayout::Alignment::Start;
			}
			};

		layout->setMainAlignment(toBoxMain(m_alignment));

		if (m_child) {
			auto childComp = m_child->build();
			// 说明：
			// - 继续为子项提供 weight=1，确保子项获得可用区域；
			// - 若需精确的“主轴不拉伸并居中”，建议子项自身处理对齐（例如 Text::align(Qt::AlignCenter)）。
			layout->addChild(childComp.release(), 1.0f, toBoxCross(
				m_alignment == Alignment::Stretch ? Alignment::Stretch : Alignment::Center == m_alignment ? Alignment::Center :
				m_alignment == Alignment::End ? Alignment::End : Alignment::Start));
		}

		// 落地装饰
		if (m_decorations.padding != QMargins()) layout->setMargins(m_decorations.padding);
		if (m_decorations.backgroundColor.alpha() > 0) {
			layout->setBackgroundColor(m_decorations.backgroundColor);
			layout->setCornerRadius(m_decorations.backgroundRadius);
		}
		return decorate(std::move(layout));
	}

} // namespace UI