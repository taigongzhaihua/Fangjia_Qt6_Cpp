#include "AppConfig.h"
#include "SettingsRepository.h"
#include <QByteArray>
#include <QString>

namespace data::repositories
{

	SettingsRepository::SettingsRepository(std::shared_ptr<AppConfig> appConfig)
		: m_appConfig(std::move(appConfig))
	{
	}

	::domain::entities::Settings SettingsRepository::getSettings() const
	{
		return mapToDomain();
	}

	void SettingsRepository::updateSettings(const ::domain::entities::Settings& settings)
	{
		mapFromDomain(settings);
	}

	void SettingsRepository::save()
	{
		m_appConfig->save();
	}

	void SettingsRepository::reset()
	{
		m_appConfig->reset();
	}

	::domain::entities::Settings SettingsRepository::mapToDomain() const
	{
		::domain::entities::Settings settings;

		// Theme configuration
		settings.themeMode = m_appConfig->themeMode().toStdString();

		// Navigation configuration
		settings.navExpanded = m_appConfig->navExpanded();
		settings.navSelectedIndex = m_appConfig->navSelectedIndex();

		// Window configuration - decode from QByteArray
		const QByteArray geo = m_appConfig->windowGeometry();
		if (!geo.isEmpty() && geo.size() == sizeof(int) * 4) {
			const auto* data = reinterpret_cast<const int*>(geo.data());
			settings.windowGeometry.x = data[0];
			settings.windowGeometry.y = data[1];
			settings.windowGeometry.width = data[2];
			settings.windowGeometry.height = data[3];
		}

		settings.windowState = m_appConfig->windowState().toStdString();

		// Recent usage
		settings.recentTab = m_appConfig->recentTab().toStdString();
		settings.recentFormula = m_appConfig->recentFormula().toStdString();

		return settings;
	}

	void SettingsRepository::mapFromDomain(const ::domain::entities::Settings& settings)
	{
		// Theme configuration
		m_appConfig->setThemeMode(QString::fromStdString(settings.themeMode));

		// Navigation configuration
		m_appConfig->setNavExpanded(settings.navExpanded);
		m_appConfig->setNavSelectedIndex(settings.navSelectedIndex);

		// Window configuration - encode to QByteArray
		QByteArray geo(sizeof(int) * 4, 0);
		auto* data = reinterpret_cast<int*>(geo.data());
		data[0] = settings.windowGeometry.x;
		data[1] = settings.windowGeometry.y;
		data[2] = settings.windowGeometry.width;
		data[3] = settings.windowGeometry.height;
		m_appConfig->setWindowGeometry(geo);

		m_appConfig->setWindowState(QByteArray::fromStdString(settings.windowState));

		// Recent usage
		m_appConfig->setRecentTab(QString::fromStdString(settings.recentTab));
		m_appConfig->setRecentFormula(QString::fromStdString(settings.recentFormula));
	}

}
