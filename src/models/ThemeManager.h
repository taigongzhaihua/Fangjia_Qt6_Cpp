#pragma once
#include <cstdint>
#include <qnamespace.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtmetamacros.h>

// Compatibility for Qt 6.4.x - ColorScheme was added in Qt 6.5
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
namespace Qt {
    enum class ColorScheme {
        Unknown,
        Light,
        Dark
    };
}
#endif

class ThemeManager final : public QObject
{
	Q_OBJECT
public:
	enum class ThemeMode :uint8_t {
		FollowSystem,
		Light,
		Dark
	};
	Q_ENUM(ThemeMode)

		explicit ThemeManager(QObject* parent = nullptr);
	~ThemeManager() override = default;

	[[nodiscard]] ThemeMode mode() const noexcept { return m_mode; }
	void setMode(ThemeMode mode);

	// 当前生效的配色（Light 或 Dark）
	[[nodiscard]] Qt::ColorScheme effectiveColorScheme() const noexcept { return m_effective; }

	// 从 QSettings 加载/保存
	void load();
	void save() const;

	// 辅助：循环切换模式
	void cycleMode();

signals:
	void effectiveColorSchemeChanged(Qt::ColorScheme scheme);
	void modeChanged(ThemeMode mode);

private:
	void updateEffectiveColorScheme();
	void connectSystemWatcher();
	void disconnectSystemWatcher();

private:
	ThemeMode m_mode{ ThemeMode::FollowSystem };
	Qt::ColorScheme m_effective{ Qt::ColorScheme::Light };
	QMetaObject::Connection m_sysConn;
};