#pragma once
#include "Widget.h"
#include <functional>
#include <qcolor.h>

namespace UI {

	struct ThemeData {
		QColor primary{ 0,122,255 };
		QColor secondary{ 108,117,125 };
		QColor background{ 255,255,255 };
		QColor surface{ 248,249,250 };
		QColor error{ 220,53,69 };
		QColor onPrimary{ 255,255,255 };
		QColor onSecondary{ 255,255,255 };
		QColor onBackground{ 33,37,41 };
		QColor onSurface{ 33,37,41 };
		QColor onError{ 255,255,255 };
		struct { int h1{ 32 }, h2{ 24 }, h3{ 20 }, body1{ 16 }, body2{ 14 }, caption{ 12 }; } fontSize;
		struct { int xs{ 4 }, sm{ 8 }, md{ 16 }, lg{ 24 }, xl{ 32 }; } spacing;
		struct { float sm{ 4.f }, md{ 8.f }, lg{ 16.f }; } radius;
		static ThemeData dark();
		static ThemeData light();
	};

	class Theme : public Widget {
	public:
		Theme(const ThemeData& data, WidgetPtr child);
		std::unique_ptr<IUiComponent> build() const override;

		static const ThemeData& of();

	private:
		ThemeData m_data;
		WidgetPtr m_child;
	};

	class ThemedBuilder : public Widget {
	public:
		using BuildFunc = std::function<WidgetPtr(const ThemeData&)>;
		explicit ThemedBuilder(BuildFunc builder) : m_builder(std::move(builder)) {}
		std::unique_ptr<IUiComponent> build() const override;

	private:
		BuildFunc m_builder;
	};

} // namespace UI