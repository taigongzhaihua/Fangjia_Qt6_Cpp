#pragma once
#include <memory>
#include <qbytearray.h>
#include <qobject.h>
#include <qsettings.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include <qvariant.h>

// 应用配置管理器
class AppConfig final : public QObject
{
	Q_OBJECT  // 添加 Q_OBJECT 宏

public:
	// 配置键定义
	struct Keys {
		static constexpr auto THEME_MODE = "Theme/Mode";
		static constexpr auto NAV_EXPANDED = "Navigation/Expanded";
		static constexpr auto WINDOW_GEOMETRY = "Window/Geometry";
		static constexpr auto WINDOW_STATE = "Window/State";
	};

	explicit AppConfig(QObject* parent = nullptr);
	~AppConfig() override = default;

	// 单例访问
	static AppConfig* instance();

	// 主题配置
	QString themeMode() const;
	void setThemeMode(const QString& mode);

	// 导航栏配置
	bool navExpanded() const;
	void setNavExpanded(bool expanded);

	// 窗口配置
	QByteArray windowGeometry() const;
	void setWindowGeometry(const QByteArray& geometry);

	QByteArray windowState() const;
	void setWindowState(const QByteArray& state);

	// 通用配置访问
	QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
	void setValue(const QString& key, const QVariant& value);

	// 配置管理
	void load();
	void save();
	void reset();

signals:
	void themeModeChanged(const QString& mode);
	void navExpandedChanged(bool expanded);
	void configChanged(const QString& key);

private:
	std::unique_ptr<QSettings> m_settings;
	static AppConfig* s_instance;

	void initDefaults();
};