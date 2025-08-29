#include "ThemeManager.h"

#include <qglobal.h>
#include <qguiapplication.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsettings.h>
#include <qstring.h>
#include <qstylehints.h>
#include <qtmetamacros.h>

namespace {
	constexpr auto K_SETTINGS_GROUP = "Theme";
	constexpr auto K_MODE_KEY = "Mode";

	QString modeToString(const ThemeManager::ThemeMode m) {
		switch (m) {
		case ThemeManager::ThemeMode::FollowSystem: return "system";
		case ThemeManager::ThemeMode::Light: return "light";
		case ThemeManager::ThemeMode::Dark: return "dark";
		}
		return "system";
	}

	ThemeManager::ThemeMode stringToMode(const QString& s) {
		const auto v = s.toLower();
		if (v == "light") return ThemeManager::ThemeMode::Light;
		if (v == "dark")  return ThemeManager::ThemeMode::Dark;
		return ThemeManager::ThemeMode::FollowSystem;
	}

	Qt::ColorScheme systemColorScheme() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
		return qApp->styleHints()->colorScheme();
#else
		// 简单回退：老版本按浅色处理
		return Qt::ColorScheme::Light;
#endif
	}
}

ThemeManager::ThemeManager(QObject* parent)
	: QObject(parent)
{
	// 初始根据系统设置一个默认值
	m_effective = systemColorScheme();
	connectSystemWatcher();
}

void ThemeManager::setMode(const ThemeMode mode)
{
	if (m_mode == mode) return;
	m_mode = mode;
	emit modeChanged(m_mode);

	// 监听系统/不监听
	disconnectSystemWatcher();
	if (m_mode == ThemeMode::FollowSystem) {
		connectSystemWatcher();
	}

	updateEffectiveColorScheme();
}

void ThemeManager::updateEffectiveColorScheme()
{
	auto newScheme = Qt::ColorScheme::Light;
	switch (m_mode) {
	case ThemeMode::FollowSystem:
		newScheme = systemColorScheme();
		break;
	case ThemeMode::Light:
		newScheme = Qt::ColorScheme::Light;
		break;
	case ThemeMode::Dark:
		newScheme = Qt::ColorScheme::Dark;
		break;
	}
	if (newScheme != m_effective) {
		m_effective = newScheme;
		emit effectiveColorSchemeChanged(m_effective);
	}
}

void ThemeManager::connectSystemWatcher()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	if (!m_sysConn) {
		m_sysConn = connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged,
			this, [this](Qt::ColorScheme) { updateEffectiveColorScheme(); });
	}
#endif
}

void ThemeManager::disconnectSystemWatcher()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	if (m_sysConn) {
		disconnect(m_sysConn);
		m_sysConn = {};
	}
#endif
}

void ThemeManager::load()
{
	QSettings s;
	s.beginGroup(K_SETTINGS_GROUP);
	const auto modeStr = s.value(K_MODE_KEY, "system").toString();
	s.endGroup();

	setMode(stringToMode(modeStr));
	// setMode 已经会调用 update 和 save（这里再保存也无妨）
}

void ThemeManager::save() const
{
	QSettings s;
	s.beginGroup(K_SETTINGS_GROUP);
	s.setValue(K_MODE_KEY, modeToString(m_mode));
	s.endGroup();
}

void ThemeManager::cycleMode()
{
	switch (m_mode) {
	case ThemeMode::FollowSystem: setMode(ThemeMode::Light); break;
	case ThemeMode::Light:        setMode(ThemeMode::Dark);  break;
	case ThemeMode::Dark:         setMode(ThemeMode::FollowSystem); break;
	}
}