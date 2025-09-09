/*
 * 文件名：TextureManager.hpp
 * 职责：高级纹理资源管理，支持资源池化、LRU缓存和批次优化
 * 依赖：Qt6 OpenGL、IconCache
 * 线程：线程安全的资源访问，支持多线程纹理请求
 * 备注：扩展IconCache功能，添加内存管理、资源池化和性能优化
 */

#pragma once
#include "IconCache.h"
#include <qmutex.h>
#include <qelapsedtimer.h>
#include <list>
#include <unordered_map>
#include <memory>

namespace Render {

	/// 纹理资源信息
	struct TextureResource {
		int textureId{0};           ///< OpenGL纹理ID
		QSize sizePx;               ///< 纹理尺寸（设备像素）
		qint64 lastUsedTime{0};     ///< 最后使用时间戳
		size_t memorySize{0};       ///< 估算的内存占用（字节）
		bool isPersistent{false};   ///< 是否为持久化纹理（不会被LRU清理）
		
		/// 计算纹理内存占用
		void updateMemorySize() {
			memorySize = static_cast<size_t>(sizePx.width()) * sizePx.height() * 4; // RGBA
		}
	};

	/// 纹理使用统计
	struct TextureStats {
		int totalTextures{0};       ///< 总纹理数量
		size_t totalMemoryMB{0};    ///< 总内存占用（MB）
		int cacheHits{0};          ///< 缓存命中次数
		int cacheMisses{0};        ///< 缓存未命中次数
		int lruEvictions{0};       ///< LRU淘汰次数
	};

	/// 高级纹理管理器：基于IconCache的增强版纹理资源管理
	/// 
	/// 功能：
	/// - 内存限制和LRU淘汰策略
	/// - 纹理资源池化和预加载
	/// - 使用统计和性能监控
	/// - 线程安全的资源访问
	/// - 批次绑定优化
	class TextureManager {
	public:
		/// 构造函数
		/// 参数：maxMemoryMB — 最大内存限制（MB）
		explicit TextureManager(size_t maxMemoryMB = 64);
		~TextureManager();

		/// 功能：获取或创建SVG纹理
		/// 参数：path — 资源路径
		/// 参数：size — 目标尺寸
		/// 参数：tint — 着色颜色
		/// 参数：gl — OpenGL函数表
		/// 返回：纹理ID（0表示失败）
		int getOrCreateTexture(const QString& path, const QSize& size, 
		                      const QColor& tint, QOpenGLFunctions* gl);

		/// 功能：获取或创建文本纹理
		/// 参数：text — 文本内容
		/// 参数：font — 字体设置
		/// 参数：color — 文本颜色
		/// 参数：gl — OpenGL函数表
		/// 返回：纹理ID（0表示失败）
		int getOrCreateTextTexture(const QString& text, const QFont& font, 
		                          const QColor& color, QOpenGLFunctions* gl);

		/// 功能：查询纹理尺寸
		/// 参数：textureId — 纹理ID
		/// 返回：纹理尺寸
		[[nodiscard]] QSize getTextureSize(int textureId) const;

		/// 功能：标记纹理为持久化（不会被LRU清理）
		/// 参数：textureId — 纹理ID
		/// 参数：persistent — 是否持久化
		void setTexturePersistent(int textureId, bool persistent);

		/// 功能：预加载纹理资源
		/// 参数：paths — 资源路径列表
		/// 参数：size — 预加载尺寸
		/// 参数：gl — OpenGL函数表
		/// 返回：成功预加载的数量
		int preloadTextures(const QStringList& paths, const QSize& size, QOpenGLFunctions* gl);

		/// 功能：清理未使用的纹理（手动触发LRU清理）
		/// 参数：gl — OpenGL函数表
		/// 参数：maxAgeSec — 最大空闲时间（秒）
		/// 返回：清理的纹理数量
		int cleanupUnusedTextures(QOpenGLFunctions* gl, int maxAgeSec = 300);

		/// 功能：释放所有纹理资源
		/// 参数：gl — OpenGL函数表
		void releaseAllTextures(QOpenGLFunctions* gl);

		/// 功能：获取使用统计信息
		/// 返回：当前的统计数据
		[[nodiscard]] TextureStats getStats() const;

		/// 功能：重置统计计数器
		void resetStats();

		/// 功能：设置内存限制
		/// 参数：maxMemoryMB — 新的内存限制（MB）
		void setMemoryLimit(size_t maxMemoryMB);

	private:
		mutable QMutex m_mutex;                                      ///< 线程安全锁
		std::unique_ptr<IconCache> m_iconCache;                      ///< 底层图标缓存
		std::unordered_map<QString, int> m_keyToTexture;            ///< 缓存键到纹理ID映射
		std::unordered_map<int, TextureResource> m_resources;        ///< 纹理资源信息
		std::list<int> m_lruList;                                   ///< LRU链表（最近使用在前）
		std::unordered_map<int, std::list<int>::iterator> m_lruMap;  ///< 纹理ID到LRU位置映射

		size_t m_maxMemoryBytes;                                     ///< 内存限制（字节）
		size_t m_currentMemoryBytes{0};                             ///< 当前内存使用（字节）
		QElapsedTimer m_timer;                                       ///< 时间计时器
		mutable TextureStats m_stats;                               ///< 使用统计

		/// 功能：生成纹理缓存键
		[[nodiscard]] QString makeTextureKey(const QString& path, const QSize& size, const QColor& tint) const;
		[[nodiscard]] QString makeTextKey(const QString& text, const QFont& font, const QColor& color) const;

		/// 功能：更新纹理使用记录（LRU）
		void updateTextureUsage(int textureId);

		/// 功能：执行LRU淘汰以释放内存
		void enforceLRULimit(QOpenGLFunctions* gl);

		/// 功能：移除纹理资源
		void removeTexture(int textureId, QOpenGLFunctions* gl);
	};

}