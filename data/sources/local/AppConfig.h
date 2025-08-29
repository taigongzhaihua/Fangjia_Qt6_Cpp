#pragma once
#include <memory>
#include <qbytearray.h>
#include <qobject.h>
#include <qsettings.h>
#include <qstring.h>
#include <qvariant.h>

// 应用配置管理器
class AppConfig final : public QObject
{
	Q_OBJECT

public:
	// 配置键定义
	struct Keys {
		// 主题
		static constexpr auto THEME_MODE = "Theme/Mode";

		// 导航
		static constexpr auto NAV_EXPANDED = "Navigation/Expanded";
		static constexpr auto NAV_SELECTED = "Navigation/SelectedIndex";

		// 窗口
		static constexpr auto WINDOW_GEOMETRY = "Window/Geometry";
		static constexpr auto WINDOW_STATE = "Window/State";

		// 最近使用
		static constexpr auto RECENT_TAB = "Recent/LastTab";
		static constexpr auto RECENT_FORMULA = "Recent/LastFormula";
	};

	explicit AppConfig(QObject* parent = nullptr);
	~AppConfig() override;

	// 主题配置
	[[nodiscard]] QString themeMode() const;
	void setThemeMode(const QString& mode);

	// 导航栏配置
	[[nodiscard]] bool navExpanded() const;
	void setNavExpanded(bool expanded);

	[[nodiscard]] int navSelectedIndex() const;
	void setNavSelectedIndex(int index);

	// 窗口配置
	[[nodiscard]] QByteArray windowGeometry() const;
	void setWindowGeometry(const QByteArray& geometry);

	[[nodiscard]] QByteArray windowState() const;
	void setWindowState(const QByteArray& state);

	// 最近使用
	[[nodiscard]] QString recentTab() const;
	void setRecentTab(const QString& tabId);

	[[nodiscard]] QString recentFormula() const;
	void setRecentFormula(const QString& formulaId);

	// 通用配置访问
	[[nodiscard]] QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
	void setValue(const QString& key, const QVariant& value);

	// 配置管理
	void load();
	void save();
	void reset();

signals:
	void themeModeChanged(const QString& mode);
	void navExpandedChanged(bool expanded);
	void navSelectedIndexChanged(int index);
	void configChanged(const QString& key);

private:
	std::unique_ptr<QSettings> m_settings;

	void initDefaults();
};