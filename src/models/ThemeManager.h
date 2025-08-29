/*
 * 文件名：ThemeManager.h
 * 职责：主题模式管理，负责系统主题监听、用户设置存储和生效配色方案推导。
 * 依赖：Qt6 Core（QObject信号机制、QSettings持久化）。
 * 线程：仅在主线程使用，信号发射线程安全。
 * 备注：支持跟随系统、强制亮色、强制暗色三种模式，兼容Qt 6.4.x版本。
 */

#pragma once
#include <cstdint>
#include <qnamespace.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtmetamacros.h>

// Qt 6.4.x兼容性处理：ColorScheme在Qt 6.5中引入
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
namespace Qt {
    enum class ColorScheme {
        Unknown,
        Light,
        Dark
    };
}
#endif

/// 主题管理器：统一管理应用主题模式与系统主题联动
/// 
/// 功能：
/// - 主题模式设置（跟随系统/强制亮色/强制暗色）
/// - 系统主题变化监听与响应
/// - 当前生效配色方案推导
/// - 持久化存储（QSettings）
/// 
/// 模式说明：
/// - FollowSystem：跟随系统配色方案变化
/// - Light：强制使用亮色主题
/// - Dark：强制使用暗色主题
/// 
/// 信号发射：
/// - modeChanged：用户主题模式改变
/// - effectiveColorSchemeChanged：当前生效配色方案改变

class ThemeManager final : public QObject
{
	Q_OBJECT
public:
	enum class ThemeMode :uint8_t {
		FollowSystem,    // 跟随系统配色方案
		Light,          // 强制亮色主题
		Dark            // 强制暗色主题
	};
	Q_ENUM(ThemeMode)

	explicit ThemeManager(QObject* parent = nullptr);
	~ThemeManager() override = default;

	/// 功能：获取当前主题模式
	/// 返回：用户设置的主题模式
	[[nodiscard]] ThemeMode mode() const noexcept { return m_mode; }
	
	/// 功能：设置主题模式
	/// 参数：mode — 新的主题模式
	/// 说明：会自动更新生效配色方案并发射相应信号
	void setMode(ThemeMode mode);

	/// 功能：获取当前生效的配色方案
	/// 返回：Light或Dark，根据模式和系统状态推导而来
	[[nodiscard]] Qt::ColorScheme effectiveColorScheme() const noexcept { return m_effective; }

	/// 功能：从QSettings加载主题设置
	/// 说明：程序启动时调用，恢复用户的主题偏好
	void load();
	
	/// 功能：保存主题设置到QSettings
	/// 说明：设置变化时调用，持久化用户偏好
	void save() const;

	/// 功能：循环切换主题模式
	/// 说明：FollowSystem -> Light -> Dark -> FollowSystem
	void cycleMode();

signals:
	/// 当前生效配色方案发生变化（受模式设置和系统主题影响）
	void effectiveColorSchemeChanged(Qt::ColorScheme scheme);
	
	/// 用户主题模式设置发生变化
	void modeChanged(ThemeMode mode);

private:
	/// 根据当前模式和系统状态重新计算生效配色方案
	void updateEffectiveColorScheme();
	
	/// 连接系统主题变化监听器
	void connectSystemWatcher();
	
	/// 断开系统主题变化监听器  
	void disconnectSystemWatcher();

private:
	ThemeMode m_mode{ ThemeMode::FollowSystem };           // 用户设置的主题模式
	Qt::ColorScheme m_effective{ Qt::ColorScheme::Light }; // 当前生效的配色方案
	QMetaObject::Connection m_sysConn;                     // 系统主题监听连接
};