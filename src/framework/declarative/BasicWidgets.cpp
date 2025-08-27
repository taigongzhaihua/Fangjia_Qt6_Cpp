#include "BasicWidgets.h"
#include "IconCache.h"
#include "Layouts.h"
#include "RenderData.hpp"
#include "UiContainer.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <memory>
#include <qcolor.h>
#include <qfile.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qnamespace.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <utility>
#include <vector>

#include "ILayoutable.hpp"
#include <qbytearray.h>
#include <qchar.h>
#include <qhash.h>
#include <qiodevice.h>

#include "RenderUtils.hpp"

namespace UI {

	// 简单的文本组件实现（支持换行 / 裁切 / 省略）
	class TextComponent final : public IUiComponent, public IUiContent, public ILayoutable {
	public:
		TextComponent(QString text,
			QColor color, bool autoColor,
			int fontSize, QFont::Weight weight,
			Qt::Alignment align,
			bool wrap, int maxLines, Text::Overflow overflow,
			bool wordWrap, int lineSpacing,
			bool useThemeColor, QColor light, QColor dark)
			: m_text(std::move(text))
			, m_color(color)
			, m_autoColor(autoColor)
			, m_fontSize(fontSize)
			, m_fontWeight(weight)
			, m_alignment(align)
			, m_wrap(wrap)
			, m_maxLines(maxLines)
			, m_overflow(overflow)
			, m_wordWrap(wordWrap)
			, m_lineSpacing(lineSpacing)
			, m_useThemeColor(useThemeColor)
			, m_colorLight(light)
			, m_colorDark(dark)
		{
		}

		// IUiContent
		void setViewportRect(const QRect& r) override { m_bounds = r; }

		// ILayoutable
		QSize measure(const SizeConstraints& cs) override {
			QFont font;
			font.setPixelSize(std::max(1, m_fontSize));
			font.setWeight(m_fontWeight);
			QFontMetrics fm(font);

			const int lineH = fm.height();
			const int lineGap = (m_lineSpacing >= 0) ? m_lineSpacing : std::lround(lineH * 0.2);

			const int maxW = std::max(0, cs.maxW);
			const int maxH = std::max(0, cs.maxH);

			if (!m_wrap) {
				int w = fm.horizontalAdvance(m_text);
				if (m_overflow == UI::Text::Overflow::Ellipsis || m_overflow == UI::Text::Overflow::Clip) {
					w = std::min(w, maxW > 0 ? maxW : w);
				}
				int h = lineH;
				w = std::clamp(w, cs.minW, cs.maxW);
				h = std::clamp(h, cs.minH, cs.maxH);
				return { w, h };
			}
			else {
				int wMax = 0;
				int hTot = 0;
				int curW = 0;
				for (auto i : m_text)
				{
					int chW = fm.horizontalAdvance(i);
					if (maxW > 0 && curW + chW > maxW) {
						wMax = std::max(wMax, curW);
						hTot += (hTot == 0 ? lineH : (lineH + lineGap));
						curW = chW;
					}
					else {
						curW += chW;
					}
				}
				if (curW > 0) {
					wMax = std::max(wMax, curW);
					hTot += (hTot == 0 ? lineH : (lineH + lineGap));
				}
				if (m_maxLines > 0) {
					const int per = (lineH + lineGap);
					const int cap = lineH + (m_maxLines - 1) * per;
					hTot = std::min(hTot, cap);
				}
				int w = (maxW > 0 ? std::min(wMax, maxW) : wMax);
				int h = (maxH > 0 ? std::min(hTot, maxH) : hTot);
				w = std::clamp(w, cs.minW, cs.maxW);
				h = std::clamp(h, cs.minH, cs.maxH);
				return { w, h };
			}
		}

		void arrange(const QRect& finalRect) override {
			m_bounds = finalRect;
		}

		void updateLayout(const QSize&) override {}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float dpr) override {
			m_cache = &cache;
			m_gl = gl;
			m_dpr = std::max(0.5f, dpr);
		}

		void append(Render::FrameData& fd) const override {
			if (!m_cache || !m_gl || m_text.isEmpty() || !m_bounds.isValid()) return;

			QFont font;
			font.setPixelSize(std::lround(static_cast<float>(m_fontSize) * m_dpr));
			font.setWeight(m_fontWeight);
			QFontMetrics fm(font);

			const int lineHpx = fm.height();
			const int lineGapPx = (m_lineSpacing >= 0) ? std::lround(static_cast<float>(m_lineSpacing) * m_dpr)
				: std::lround(lineHpx * 0.2);
			const int availWpx = std::max(0, static_cast<int>(std::lround(static_cast<float>(m_bounds.width()) * m_dpr)));
			const int availHpx = std::max(0, static_cast<int>(std::lround(static_cast<float>(m_bounds.height()) * m_dpr)));

			struct Line { QString text; int tex{ 0 }; QSize texPx; int drawWpx{ 0 }; };
			std::vector<Line> lines;

			auto makeTex = [&](const QString& s) -> Line {
				const QString key = QString("text_%1_%2_%3").arg(s).arg(m_fontSize).arg(m_color.name(QColor::HexArgb));
				const int tex = m_cache->ensureTextPx(key, font, s, m_color, m_gl);
				const QSize ts = m_cache->textureSizePx(tex);
				return Line{ .text = s, .tex = tex, .texPx = ts, .drawWpx = ts.width() };
				};
			auto elideRight = [&](const QString& s, const int maxWpx) -> QString {
				return QFontMetrics(font).elidedText(s, Qt::ElideRight, std::max(0, maxWpx));
				};

			if (!m_wrap) {
				QString s = m_text;
				if (m_overflow == Text::Overflow::Ellipsis) {
					s = elideRight(s, availWpx);
					lines.push_back(makeTex(s));
				}
				else {
					Line ln = makeTex(s);
					if (m_overflow == Text::Overflow::Clip && ln.drawWpx > availWpx) {
						ln.drawWpx = availWpx;
					}
					lines.push_back(std::move(ln));
				}
			}
			else {
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
						const int w = QFontMetrics(font).horizontalAdvance(piece);
						if (widthPx + w > availWpx && availWpx > 0) {
							if (m_wordWrap && lastBreak > pos) {
								lineEnd = lastBreak;
							}
							else if (lineEnd == pos) {
								++lineEnd;
							}
							break;
						}
						widthPx += w;
						if (m_wordWrap && (ch.isSpace() || ch == QChar::fromLatin1('-') || ch == QChar::fromLatin1('/'))) {
							lastBreak = lineEnd + 1;
						}
						++lineEnd;
					}

					QString seg = sAll.mid(pos, lineEnd - pos);
					while (!seg.isEmpty() && seg.back().isSpace()) seg.chop(1);

					const bool lastLine = (lines.size() + 1 == static_cast<size_t>(maxLines));
					const bool hasMore = (lineEnd < n);

					if (lastLine && hasMore) {
						if (m_overflow == Text::Overflow::Ellipsis) {
							seg = elideRight(sAll.mid(pos), availWpx);
							lines.push_back(makeTex(seg));
							break;
						}
						else {
							lines.push_back(makeTex(seg));
							break;
						}
					}
					else {
						if (seg.isEmpty()) {
							seg = sAll.mid(pos, 1);
							lineEnd = pos + 1;
						}
						lines.push_back(makeTex(seg));
						pos = lineEnd;
						while (pos < n && sAll[pos].isSpace()) ++pos;
					}

					const int totalHpx = static_cast<int>(lines.size()) * lineHpx + static_cast<int>(!lines.empty() ? (lines.size() - 1) : 0) * lineGapPx;
					if (totalHpx > availHpx && availHpx > 0) {
						break;
					}
				}
			}

			if (lines.empty()) return;

			const int totalHpx = static_cast<int>(lines.size()) * lineHpx + static_cast<int>(!lines.empty() ? (lines.size() - 1) : 0) * lineGapPx;

			float y0 = static_cast<float>(m_bounds.top());
			if (m_alignment.testFlag(Qt::AlignVCenter)) {
				y0 = static_cast<float>(m_bounds.center().y()) - (static_cast<float>(totalHpx) / m_dpr) * 0.5f;
			}
			else if (m_alignment.testFlag(Qt::AlignBottom)) {
				y0 = static_cast<float>(m_bounds.bottom()) - static_cast<float>(totalHpx) / m_dpr;
			}

			for (size_t i = 0; i < lines.size(); ++i) {
				const auto& ln = lines[i];

				const float wLogical = static_cast<float>(ln.texPx.width()) / m_dpr;
				const float hLogical = static_cast<float>(ln.texPx.height()) / m_dpr;

				const float lineTop = y0 + static_cast<float>(i) * (static_cast<float>(lineHpx + lineGapPx) / m_dpr);

				float x = static_cast<float>(m_bounds.left());
				if (m_alignment.testFlag(Qt::AlignHCenter)) {
					x = static_cast<float>(m_bounds.center().x()) - wLogical * 0.5f;
				}
				else if (m_alignment.testFlag(Qt::AlignRight)) {
					x = static_cast<float>(m_bounds.right()) - wLogical;
				}

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

				if (lineTop >= static_cast<float>(m_bounds.bottom())) break;
				if (lineTop + hLogical <= static_cast<float>(m_bounds.top())) continue;

				const QRectF dst(x, lineTop, drawW, hLogical);
				if (dst.width() <= 0.0 || dst.height() <= 0.0) continue;

				fd.images.push_back(Render::ImageCmd{
					.dstRect = dst,
					.textureId = ln.tex,
					.srcRectPx = srcPx,
					.tint = QColor(255,255,255,255),
					.clipRect = QRectF(m_bounds)
					});
			}
		}

		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }

		QRect bounds() const override {
			if (m_bounds.isValid() && m_bounds.height() > 0) return m_bounds;
			const int estimatedLineH = std::max(1, static_cast<int>(std::round(m_fontSize * 1.4)));
			return { 0, 0, 0, estimatedLineH };
		}

		void onThemeChanged(const bool isDark) override {
			// 优先使用用户指定的主题色
			if (m_useThemeColor) {
				m_color = isDark ? m_colorDark : m_colorLight;
				return;
			}
			// 否则按默认的自动配色
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

		// 新增：主题色支持
		bool  m_useThemeColor{ false };
		QColor m_colorLight{ 30,35,40 };
		QColor m_colorDark{ 240,245,250 };

		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

	std::unique_ptr<IUiComponent> Text::build() const {
		auto comp = std::make_unique<TextComponent>(
			m_text, m_color, m_autoColor,
			m_fontSize, m_fontWeight, m_alignment,
			m_wrap, m_maxLines, m_overflow, m_wordWrap, m_lineSpacing,
			m_useThemeColor, m_colorLight, m_colorDark
		);
		return decorate(std::move(comp));
	}

	// 图标组件实现（与之前相同，补上裁剪）
	class IconComponent : public IUiComponent, public IUiContent, public ILayoutable {
	public:
		IconComponent(QString path, const QColor& color, const int size, const bool autoColor)
			: m_path(std::move(path)), m_color(color), m_size(size), m_autoColor(autoColor) {
		}

		// IUiContent
		void setViewportRect(const QRect& r) override { m_bounds = r; }

		// ILayoutable：图标自然尺寸为 m_size（正方形），并受父约束裁剪
		QSize measure(const SizeConstraints& cs) override {
			const int s = std::max(0, m_size);
			int w = std::clamp(s, cs.minW, cs.maxW);
			int h = std::clamp(s, cs.minH, cs.maxH);
			return { w, h };
		}
		void arrange(const QRect& finalRect) override { m_bounds = finalRect; }

		void updateLayout(const QSize&) override {}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float dpr) override {
			m_cache = &cache;
			m_gl = gl;
			m_dpr = std::max(0.5f, dpr);
		}

		void append(Render::FrameData& fd) const override {
			if (!m_cache || !m_gl || m_path.isEmpty() || !m_bounds.isValid()) return;

			// 目标逻辑尺寸：不超过自身 bounds，保持正方形
			const int availW = std::max(0, m_bounds.width());
			const int availH = std::max(0, m_bounds.height());
			const int logicalS = std::max(0, std::min({ m_size, availW, availH }));
			if (logicalS <= 0) return;

			// 中心放置
			const QRectF dst(
				m_bounds.center().x() - logicalS * 0.5,
				m_bounds.center().y() - logicalS * 0.5,
				logicalS,
				logicalS
			);

			// 生成/获取纹理
			const int px = std::lround(static_cast<float>(logicalS) * m_dpr);
			QByteArray svg = RenderUtils::loadSvgCached(m_path);
			const QString key = RenderUtils::makeIconCacheKey(m_path, px);
			const int tex = m_cache->ensureSvgPx(key, svg, QSize(px, px), m_gl);
			const QSize ts = m_cache->textureSizePx(tex);

			fd.images.push_back(Render::ImageCmd{
				.dstRect = dst,
				.textureId = tex,
				.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
				.tint = m_color,                     // 以白膜纹理 + tint 输出指定颜色
				.clipRect = QRectF(m_bounds)         // 严格裁剪到自身 bounds
				});
		}

		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }

		QRect bounds() const override {
			// 若已有布局矩形，返回之；否则以自然尺寸给出一个首选矩形
			if (m_bounds.isValid()) return m_bounds;
			const int s = std::max(0, m_size);
			return { 0, 0, s, s };
		}

		void onThemeChanged(const bool isDark) override {
			if (m_autoColor) {
				// 简单的自动配色：可按需要调整
				m_color = isDark ? QColor(100, 160, 220) : QColor(60, 120, 180);
			}
		}

	private:
		QString m_path;
		QColor  m_color{ 0,0,0 };
		int     m_size{ 24 };
		bool    m_autoColor{ true };

		QRect m_bounds;
		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

	std::unique_ptr<IUiComponent> Icon::build() const {
		auto comp = std::make_unique<IconComponent>(m_path, m_color, m_size, m_autoColor);
		return decorate(std::move(comp));
	}


	// 容器组件实现（保持与你当前版本一致）
	std::unique_ptr<IUiComponent> Container::build() const {
		auto cont = std::make_unique<UiContainer>();
		// 映射对齐：单子项容器用统一两轴对齐
		auto toAlign = [](const Alignment a) {
			switch (a) {
			case Alignment::Center:  return UiContainer::Align::Center;
			case Alignment::End:     return UiContainer::Align::End;
			case Alignment::Stretch: return UiContainer::Align::Stretch;
			case Alignment::Start:
			default:				 return UiContainer::Align::Start;
			}
			};
		cont->setAlignment(toAlign(m_alignment));
		if (m_child) {
			cont->setChild(m_child->build().release());
		}
		// 注意：不再把 Widget 的 padding/background 写入容器本体，
			// 统一交给 DecoratedBox（decorate）处理，避免双重 padding。
		return decorate(std::move(cont));
	}

} // namespace UI