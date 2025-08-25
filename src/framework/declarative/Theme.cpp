#include "Theme.h"
#include <vector>

namespace UI {

	// 构建期 Theme 栈（thread_local 保证渲染线程内安全）
	static thread_local std::vector<ThemeData> g_themeStack;

	ThemeData ThemeData::dark() {
		ThemeData t;
		t.primary = QColor(66, 165, 245);
		t.background = QColor(18, 18, 18);
		t.surface = QColor(33, 33, 33);
		t.onBackground = QColor(255, 255, 255);
		t.onSurface = QColor(255, 255, 255);
		return t;
	}
	ThemeData ThemeData::light() { return ThemeData{}; }

	Theme::Theme(const ThemeData& data, WidgetPtr child)
		: m_data(data), m_child(std::move(child)) {
	}

	std::unique_ptr<IUiComponent> Theme::build() const {
		g_themeStack.push_back(m_data);
		auto out = m_child ? m_child->build() : nullptr;
		g_themeStack.pop_back();
		return out;
	}

	const ThemeData& Theme::of() {
		static ThemeData fallback; // 默认浅色
		if (!g_themeStack.empty()) return g_themeStack.back();
		return fallback;
	}

	std::unique_ptr<IUiComponent> ThemedBuilder::build() const {
		return m_builder(Theme::of())->build();
	}

} // namespace UI