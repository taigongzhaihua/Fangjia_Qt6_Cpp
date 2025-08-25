#include "BasicWidgets.h"
#include "IconLoader.h"
#include "Layouts.h"
#include "RenderData.hpp"
#include "UiBoxLayout.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <memory>
#include <qchar.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <utility>
#include <vector>

namespace UI {

	// 简单的文本组件实现（支持换行 / 裁切 / 省略）
	class TextComponent : public IUiComponent, public IUiContent {
	public:
		TextComponent(const QString& text,
			QColor color, bool autoColor,
			int fontSize, QFont::Weight weight,
			Qt::Alignment align,
			bool wrap, int maxLines, Text::Overflow overflow,
			bool wordWrap, int lineSpacing)
			: m_text(text)
			, m_color(color)
			, m_autoColor(autoColor)
			, m_fontSize(fontSize)
			, m_fontWeight(weight)
			, m_alignment(align)
			, m_wrap(wrap)
			, m_maxLines(maxLines)
			, m_overflow(overflow)
			, m_wordWrap(wordWrap)
			, m_lineSpacing(lineSpacing) {
		}

		// IUiContent
		void setViewportRect(const QRect& r) override { m_bounds = r; }

		void updateLayout(const QSize&) override {}

		void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override {
			m_loader = &loader;
			m_gl = gl;
			m_dpr = std::max(0.5f, dpr);
		}

		void append(Render::FrameData& fd) const override {
			if (!m_loader || !m_gl || m_text.isEmpty() || !m_bounds.isValid()) return;

			// 构建字体（像素大小）
			QFont font;
			font.setPixelSize(std::lround(m_fontSize * m_dpr));
			font.setWeight(m_fontWeight);
			QFontMetrics fm(font);

			const int lineHpx = fm.height();
			const int lineGapPx = (m_lineSpacing >= 0) ? std::lround(m_lineSpacing * m_dpr)
				: std::lround(lineHpx * 0.2); // 默认 0.2 倍行距
			const int availWpx = std::max(0, static_cast<int>(std::lround(m_bounds.width() * m_dpr)));
			const int availHpx = std::max(0, static_cast<int>(std::lround(m_bounds.height() * m_dpr)));

			// 行布局
			struct Line { QString text; int tex{ 0 }; QSize texPx; int drawWpx{ 0 }; };
			std::vector<Line> lines;

			auto makeTex = [&](const QString& s) -> Line {
				const QString key = QString("text_%1_%2_%3").arg(s).arg(m_fontSize).arg(m_color.name(QColor::HexArgb));
				const int tex = m_loader->ensureTextPx(key, font, s, m_color, m_gl);
				const QSize ts = m_loader->textureSizePx(tex);
				return Line{ s, tex, ts, ts.width() };
				};

			auto elideRight = [&](const QString& s, int maxWpx) -> QString {
				return fm.elidedText(s, Qt::ElideRight, std::max(0, maxWpx));
				};

			if (!m_wrap) {
				// 单行
				QString s = m_text;
				if (m_overflow == Text::Overflow::Ellipsis) {
					s = elideRight(s, availWpx);
					lines.push_back(makeTex(s));
				}
				else {
					Line ln = makeTex(s);
					// Clip: 按像素裁剪 srcRect（Visible: 不裁剪）
					if (m_overflow == Text::Overflow::Clip && ln.drawWpx > availWpx) {
						ln.drawWpx = availWpx; // 仅绘制可见宽度
					}
					lines.push_back(std::move(ln));
				}
			}
			else {
				// 多行换行
				const QString& sAll = m_text;
				int pos = 0;
				const int n = sAll.size();
				const int maxLines = (m_maxLines > 0 ? m_maxLines : INT_MAX);

				while (pos < n && lines.size() < static_cast<size_t>(maxLines)) {
					int lineEnd = pos;
					int lastBreak = -1;
					int widthPx = 0;

					while (lineEnd < n) {
						const QChar ch = sAll[lineEnd];
						const QString piece(ch);
						const int w = fm.horizontalAdvance(piece);
						if (widthPx + w > availWpx && availWpx > 0) {
							// 超宽，回退到最后断点；如果没有断点，按字符断
							if (m_wordWrap && lastBreak > pos) {
								lineEnd = lastBreak;
							}
							else if (lineEnd == pos) {
								// 单个字符都放不下：强制至少放一个
								++lineEnd;
							}
							break;
						}
						widthPx += w;
						if (m_wordWrap && (ch.isSpace() || ch == QChar::fromLatin1('-') || ch == QChar::fromLatin1('/'))) {
							lastBreak = lineEnd + 1; // 断点放在后一个位置（跳过空格）
						}
						++lineEnd;
					}

					QString seg = sAll.mid(pos, lineEnd - pos);

					// 去掉末尾多余空格
					while (!seg.isEmpty() && seg.back().isSpace()) seg.chop(1);

					// 如果已经到达最后一行但还有剩余内容，根据溢出策略处理
					const bool lastLine = (lines.size() + 1 == static_cast<size_t>(maxLines));
					const bool hasMore = (lineEnd < n);

					if (lastLine && hasMore) {
						if (m_overflow == Text::Overflow::Ellipsis) {
							seg = elideRight(sAll.mid(pos), availWpx);
							lines.push_back(makeTex(seg));
							break;
						}
						else if (m_overflow == Text::Overflow::Clip) {
							// 正常放这一行，余下不再绘制
							lines.push_back(makeTex(seg));
							break;
						}
						else { // Visible
							lines.push_back(makeTex(seg));
							break;
						}
					}
					else {
						if (seg.isEmpty()) {
							// 放至少一个字符，防止死循环
							seg = sAll.mid(pos, 1);
							lineEnd = pos + 1;
						}
						lines.push_back(makeTex(seg));
						pos = lineEnd;
						// 跳过前导空格
						while (pos < n && sAll[pos].isSpace()) ++pos;
					}

					// 高度满了就停止（Clip/ Ellipsis 皆停止）
					const int totalHpx = static_cast<int>(lines.size()) * lineHpx + static_cast<int>(lines.size() > 0 ? (lines.size() - 1) : 0) * lineGapPx;
					if (totalHpx > availHpx && availHpx > 0) {
						break;
					}
				}
			}

			if (lines.empty()) return;

			// 计算总高度用于垂直对齐
			const int totalHpx = static_cast<int>(lines.size()) * lineHpx + static_cast<int>(lines.size() > 0 ? (lines.size() - 1) : 0) * lineGapPx;

			// 起始 Y（逻辑像素）
			float y0 = static_cast<float>(m_bounds.top());
			if (m_alignment.testFlag(Qt::AlignVCenter)) {
				y0 = m_bounds.center().y() - (static_cast<float>(totalHpx) / m_dpr) * 0.5f;
			}
			else if (m_alignment.testFlag(Qt::AlignBottom)) {
				y0 = m_bounds.bottom() - static_cast<float>(totalHpx) / m_dpr;
			}

			// 逐行绘制
			for (size_t i = 0; i < lines.size(); ++i) {
				const auto& ln = lines[i];

				// 逻辑尺寸
				const float wLogical = static_cast<float>(ln.texPx.width()) / m_dpr;
				const float hLogical = static_cast<float>(ln.texPx.height()) / m_dpr;

				// 行基线 Y（顶部对齐）
				const float lineTop = y0 + static_cast<float>(i) * (static_cast<float>(lineHpx + lineGapPx) / m_dpr);

				// 水平对齐
				float x = static_cast<float>(m_bounds.left());
				if (m_alignment.testFlag(Qt::AlignHCenter)) {
					x = m_bounds.center().x() - wLogical * 0.5f;
				}
				else if (m_alignment.testFlag(Qt::AlignRight)) {
					x = m_bounds.right() - wLogical;
				}

				// 水平裁切：已在 Renderer 用 glScissor 统一处理；这里仍保留像素级 srcRect 裁切（多行/clip 模式）
				QRectF srcPx(0, 0, ln.texPx.width(), ln.texPx.height());
				float drawW = wLogical;
				if ((m_overflow == Text::Overflow::Clip || m_wrap) && m_bounds.width() > 0) {
					const float leftVisible = static_cast<float>(m_bounds.left());
					const float rightVisible = static_cast<float>(m_bounds.right());
					if (x < leftVisible) {
						const float cutL = std::min(leftVisible - x, drawW);
						const float cutLpx = cutL * m_dpr;
						srcPx.setX(srcPx.x() + cutLpx);
						srcPx.setWidth(std::max(0.0, srcPx.width() - cutLpx));
						x += cutL;
						drawW -= cutL;
					}
					if (x + drawW > rightVisible) {
						const float cutR = std::max(0.0f, x + drawW - rightVisible);
						const float newW = std::max(0.0f, drawW - cutR);
						const float newWpx = newW * m_dpr;
						srcPx.setWidth(std::max(0.0f, std::min(static_cast<float>(srcPx.width()), newWpx)));
						drawW = newW;
					}
				}

				// 垂直裁切：若超出下边界，停止绘制
				if (lineTop >= static_cast<float>(m_bounds.bottom())) break;
				if (lineTop + hLogical <= static_cast<float>(m_bounds.top())) continue;

				// 目标矩形
				const QRectF dst(x, lineTop, drawW, hLogical);
				if (dst.width() <= 0.0 || dst.height() <= 0.0) continue;

				fd.images.push_back(Render::ImageCmd{
					.dstRect = dst,
					.textureId = ln.tex,
					.srcRectPx = srcPx,
					.tint = QColor(255,255,255,255),
					.clipRect = QRectF(m_bounds) // 新增：对整段文本使用 m_bounds 作为裁剪区域
					});
			}
		}

		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }
		QRect bounds() const override { return m_bounds; }

		void onThemeChanged(bool isDark) override {
			if (m_autoColor) {
				m_color = isDark ? QColor(240, 245, 250) : QColor(30, 35, 40);
			}
		}

	private:
		QString m_text;
		QColor m_color;
		bool m_autoColor{ true };
		int m_fontSize;
		QFont::Weight m_fontWeight;
		Qt::Alignment m_alignment;
		QRect m_bounds;

		bool m_wrap{ false };
		int m_maxLines{ 1 };
		Text::Overflow m_overflow{ Text::Overflow::Clip };
		bool m_wordWrap{ true };
		int  m_lineSpacing{ -1 };

		IconLoader* m_loader{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

	std::unique_ptr<IUiComponent> Text::build() const {
		auto comp = std::make_unique<TextComponent>(
			m_text, m_color, m_autoColor,
			m_fontSize, m_fontWeight, m_alignment,
			m_wrap, m_maxLines, m_overflow, m_wordWrap, m_lineSpacing
		);
		return decorate(std::move(comp));
	}

	// 图标组件实现（与之前相同，补上裁剪）
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
							  static_cast<qreal>(m_size), static_cast<qreal>(m_size)),
				.radiusPx = m_size / 4.0f,
				.color = m_color,
				.clipRect = QRectF(m_bounds) // 新增：裁剪到自身 bounds
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
		QColor m_color{ 0,0,0 };
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

	// 容器组件实现（保持与你当前版本一致）
	std::unique_ptr<IUiComponent> Container::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);

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
			layout->addChild(childComp.release(), 1.0f, toBoxCross(
				m_alignment == Alignment::Stretch ? Alignment::Stretch : Alignment::Center == m_alignment ? Alignment::Center :
				m_alignment == Alignment::End ? Alignment::End : Alignment::Start));
		}

		if (m_decorations.padding != QMargins()) layout->setMargins(m_decorations.padding);
		if (m_decorations.backgroundColor.alpha() > 0) {
			layout->setBackgroundColor(m_decorations.backgroundColor);
			layout->setCornerRadius(m_decorations.backgroundRadius);
		}
		return decorate(std::move(layout));
	}

} // namespace UI