#include "AppConfig.h"
#include <memory>
#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qobject.h>
#include <qsettings.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include <qvariant.h>

AppConfig::AppConfig(QObject* parent)
	: QObject(parent)
{
	m_settings = std::make_unique<QSettings>(
		QCoreApplication::organizationName(),
		QCoreApplication::applicationName()
	);

	initDefaults();
}

AppConfig::~AppConfig()
{
	save();  // 析构时自动保存
}

void AppConfig::initDefaults()
{
	// 设置默认值
	if (!m_settings->contains(Keys::THEME_MODE)) {
		m_settings->setValue(Keys::THEME_MODE, "system");
	}

	if (!m_settings->contains(Keys::NAV_EXPANDED)) {
		m_settings->setValue(Keys::NAV_EXPANDED, false);
	}

	if (!m_settings->contains(Keys::NAV_SELECTED)) {
		m_settings->setValue(Keys::NAV_SELECTED, 0);
	}
}

QString AppConfig::themeMode() const
{
	return m_settings->value(Keys::THEME_MODE, "system").toString();
}

void AppConfig::setThemeMode(const QString& mode)
{
	if (themeMode() != mode) {
		m_settings->setValue(Keys::THEME_MODE, mode);
		emit themeModeChanged(mode);
		emit configChanged(Keys::THEME_MODE);
	}
}

bool AppConfig::navExpanded() const
{
	return m_settings->value(Keys::NAV_EXPANDED, false).toBool();
}

void AppConfig::setNavExpanded(const bool expanded)
{
	if (navExpanded() != expanded) {
		m_settings->setValue(Keys::NAV_EXPANDED, expanded);
		emit navExpandedChanged(expanded);
		emit configChanged(Keys::NAV_EXPANDED);
	}
}

int AppConfig::navSelectedIndex() const
{
	return m_settings->value(Keys::NAV_SELECTED, 0).toInt();
}

void AppConfig::setNavSelectedIndex(const int index)
{
	if (navSelectedIndex() != index) {
		m_settings->setValue(Keys::NAV_SELECTED, index);
		emit navSelectedIndexChanged(index);
		emit configChanged(Keys::NAV_SELECTED);
	}
}

QByteArray AppConfig::windowGeometry() const
{
	return m_settings->value(Keys::WINDOW_GEOMETRY).toByteArray();
}

void AppConfig::setWindowGeometry(const QByteArray& geometry)
{
	m_settings->setValue(Keys::WINDOW_GEOMETRY, geometry);
	emit configChanged(Keys::WINDOW_GEOMETRY);
}

QByteArray AppConfig::windowState() const
{
	return m_settings->value(Keys::WINDOW_STATE).toByteArray();
}

void AppConfig::setWindowState(const QByteArray& state)
{
	m_settings->setValue(Keys::WINDOW_STATE, state);
	emit configChanged(Keys::WINDOW_STATE);
}

QString AppConfig::recentTab() const
{
	return m_settings->value(Keys::RECENT_TAB).toString();
}

void AppConfig::setRecentTab(const QString& tabId)
{
	m_settings->setValue(Keys::RECENT_TAB, tabId);
	emit configChanged(Keys::RECENT_TAB);
}

QString AppConfig::recentFormula() const
{
	return m_settings->value(Keys::RECENT_FORMULA).toString();
}

void AppConfig::setRecentFormula(const QString& formulaId)
{
	m_settings->setValue(Keys::RECENT_FORMULA, formulaId);
	emit configChanged(Keys::RECENT_FORMULA);
}

QVariant AppConfig::value(const QString& key, const QVariant& defaultValue) const
{
	return m_settings->value(key, defaultValue);
}

void AppConfig::setValue(const QString& key, const QVariant& value)
{
	m_settings->setValue(key, value);
	emit configChanged(key);
}

void AppConfig::load()
{
	m_settings->sync();
}

void AppConfig::save()
{
	m_settings->sync();
}

void AppConfig::reset()
{
	m_settings->clear();
	initDefaults();
}