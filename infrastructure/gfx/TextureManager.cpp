#include "TextureManager.hpp"
#include "IconLoader.h"
#include <QtGui/qopengl.h>
#include <qloggingcategory.h>

Q_DECLARE_LOGGING_CATEGORY(lcTextureManager)
Q_LOGGING_CATEGORY(lcTextureManager, "gfx.texture", QtWarningMsg)

namespace Render {

	TextureManager::TextureManager(size_t maxMemoryMB) 
		: m_iconCache(std::make_unique<IconCache>())
		, m_maxMemoryBytes(maxMemoryMB * 1024 * 1024) {
		m_timer.start();
	}

	TextureManager::~TextureManager() = default;

	int TextureManager::getOrCreateTexture(const QString& path, const QSize& size, 
		                                   const QColor& tint, QOpenGLFunctions* gl) {
		QMutexLocker locker(&m_mutex);

		const QString key = makeTextureKey(path, size, tint);
		
		// 检查缓存
		auto it = m_keyToTexture.find(key);
		if (it != m_keyToTexture.end()) {
			updateTextureUsage(it->second);
			m_stats.cacheHits++;
			return it->second;
		}
		
		m_stats.cacheMisses++;
		
		// 创建新纹理 - 使用IconCache的现有功能
		// 注意：这里简化了实现，实际应该从文件加载SVG数据
		QByteArray svgData; // TODO: 从path加载SVG数据
		int textureId = m_iconCache->ensureSvgPx(key, svgData, size, gl);
		
		if (textureId > 0) {
			// 创建资源记录
			TextureResource resource;
			resource.textureId = textureId;
			resource.sizePx = size;
			resource.lastUsedTime = m_timer.elapsed();
			resource.updateMemorySize();
			
			// 更新内存使用
			m_currentMemoryBytes += resource.memorySize;
			
			// 添加到缓存
			m_keyToTexture[key] = textureId;
			m_resources[textureId] = resource;
			
			// 更新LRU
			m_lruList.push_front(textureId);
			m_lruMap[textureId] = m_lruList.begin();
			
			// 检查内存限制
			enforceLRULimit(gl);
			
			m_stats.totalTextures++;
			m_stats.totalMemoryMB = m_currentMemoryBytes / (1024 * 1024);
		}
		
		return textureId;
	}

	int TextureManager::getOrCreateTextTexture(const QString& text, const QFont& font, 
		                                       const QColor& color, QOpenGLFunctions* gl) {
		QMutexLocker locker(&m_mutex);
		
		const QString key = makeTextKey(text, font, color);
		
		// 检查缓存
		auto it = m_keyToTexture.find(key);
		if (it != m_keyToTexture.end()) {
			updateTextureUsage(it->second);
			m_stats.cacheHits++;
			return it->second;
		}
		
		m_stats.cacheMisses++;
		
		// 创建文本纹理
		int textureId = m_iconCache->ensureTextPx(key, font, text, color, gl);
		
		if (textureId > 0) {
			const QSize textureSize = m_iconCache->textureSizePx(textureId);
			
			// 创建资源记录
			TextureResource resource;
			resource.textureId = textureId;
			resource.sizePx = textureSize;
			resource.lastUsedTime = m_timer.elapsed();
			resource.updateMemorySize();
			
			// 更新内存使用
			m_currentMemoryBytes += resource.memorySize;
			
			// 添加到缓存
			m_keyToTexture[key] = textureId;
			m_resources[textureId] = resource;
			
			// 更新LRU
			m_lruList.push_front(textureId);
			m_lruMap[textureId] = m_lruList.begin();
			
			// 检查内存限制
			enforceLRULimit(gl);
			
			m_stats.totalTextures++;
			m_stats.totalMemoryMB = m_currentMemoryBytes / (1024 * 1024);
		}
		
		return textureId;
	}

	QSize TextureManager::getTextureSize(int textureId) const {
		QMutexLocker locker(&m_mutex);
		
		auto it = m_resources.find(textureId);
		if (it != m_resources.end()) {
			return it->second.sizePx;
		}
		
		// 回退到IconCache
		return m_iconCache->textureSizePx(textureId);
	}

	void TextureManager::setTexturePersistent(int textureId, bool persistent) {
		QMutexLocker locker(&m_mutex);
		
		auto it = m_resources.find(textureId);
		if (it != m_resources.end()) {
			it->second.isPersistent = persistent;
		}
	}

	int TextureManager::preloadTextures(const QStringList& paths, const QSize& size, QOpenGLFunctions* gl) {
		int loadedCount = 0;
		
		for (const QString& path : paths) {
			int textureId = getOrCreateTexture(path, size, Qt::white, gl);
			if (textureId > 0) {
				// 标记为持久化，避免被LRU清理
				setTexturePersistent(textureId, true);
				loadedCount++;
			}
		}
		
		qCDebug(lcTextureManager) << "Preloaded" << loadedCount << "textures";
		return loadedCount;
	}

	int TextureManager::cleanupUnusedTextures(QOpenGLFunctions* gl, int maxAgeSec) {
		QMutexLocker locker(&m_mutex);
		
		const qint64 currentTime = m_timer.elapsed();
		const qint64 maxAgeMs = maxAgeSec * 1000;
		
		int cleanedCount = 0;
		
		// 从LRU列表尾部开始清理（最久未使用）
		auto it = m_lruList.rbegin();
		while (it != m_lruList.rend()) {
			const int textureId = *it;
			const auto& resource = m_resources[textureId];
			
			// 检查是否应该清理
			if (!resource.isPersistent && 
				(currentTime - resource.lastUsedTime) > maxAgeMs) {
				// 移除纹理
				++it; // 移动到下一个元素，因为removeTexture会修改列表
				removeTexture(textureId, gl);
				cleanedCount++;
			} else {
				++it;
			}
		}
		
		if (cleanedCount > 0) {
			qCDebug(lcTextureManager) << "Cleaned up" << cleanedCount << "unused textures";
		}
		
		return cleanedCount;
	}

	void TextureManager::releaseAllTextures(QOpenGLFunctions* gl) {
		QMutexLocker locker(&m_mutex);
		
		// 释放IconCache中的所有纹理
		m_iconCache->releaseAll(gl);
		
		// 清理所有内部数据结构
		m_keyToTexture.clear();
		m_resources.clear();
		m_lruList.clear();
		m_lruMap.clear();
		
		m_currentMemoryBytes = 0;
		m_stats = TextureStats{}; // 重置统计
	}

	TextureStats TextureManager::getStats() const {
		QMutexLocker locker(&m_mutex);
		
		// 更新实时统计
		TextureStats stats = m_stats;
		stats.totalTextures = m_resources.size();
		stats.totalMemoryMB = m_currentMemoryBytes / (1024 * 1024);
		
		return stats;
	}

	void TextureManager::resetStats() {
		QMutexLocker locker(&m_mutex);
		
		m_stats.cacheHits = 0;
		m_stats.cacheMisses = 0;
		m_stats.lruEvictions = 0;
		// 保留totalTextures和totalMemoryMB
	}

	void TextureManager::setMemoryLimit(size_t maxMemoryMB) {
		QMutexLocker locker(&m_mutex);
		
		m_maxMemoryBytes = maxMemoryMB * 1024 * 1024;
		qCDebug(lcTextureManager) << "Memory limit set to" << maxMemoryMB << "MB";
	}

	QString TextureManager::makeTextureKey(const QString& path, const QSize& size, const QColor& tint) const {
		return QString("%1_%2x%3_%4")
			.arg(path)
			.arg(size.width())
			.arg(size.height()) 
			.arg(tint.rgba());
	}

	QString TextureManager::makeTextKey(const QString& text, const QFont& font, const QColor& color) const {
		return QString("text_%1_%2_%3px_%4")
			.arg(text)
			.arg(font.family())
			.arg(font.pixelSize())
			.arg(color.rgba());
	}

	void TextureManager::updateTextureUsage(int textureId) {
		// 更新使用时间
		auto resourceIt = m_resources.find(textureId);
		if (resourceIt != m_resources.end()) {
			resourceIt->second.lastUsedTime = m_timer.elapsed();
		}
		
		// 更新LRU位置
		auto lruIt = m_lruMap.find(textureId);
		if (lruIt != m_lruMap.end()) {
			// 移动到链表前端
			m_lruList.splice(m_lruList.begin(), m_lruList, lruIt->second);
		}
	}

	void TextureManager::enforceLRULimit(QOpenGLFunctions* gl) {
		// 当内存超限时，从LRU尾部清理纹理
		while (m_currentMemoryBytes > m_maxMemoryBytes && !m_lruList.empty()) {
			int textureId = m_lruList.back();
			const auto& resource = m_resources[textureId];
			
			// 不清理持久化纹理
			if (resource.isPersistent) {
				// 如果所有剩余纹理都是持久化的，停止清理
				bool allPersistent = true;
				for (int id : m_lruList) {
					if (!m_resources[id].isPersistent) {
						allPersistent = false;
						break;
					}
				}
				if (allPersistent) break;
				
				// 将持久化纹理移到前面，继续清理其他纹理
				m_lruList.splice(m_lruList.begin(), m_lruList, std::prev(m_lruList.end()));
				continue;
			}
			
			removeTexture(textureId, gl);
			m_stats.lruEvictions++;
		}
	}

	void TextureManager::removeTexture(int textureId, QOpenGLFunctions* gl) {
		auto resourceIt = m_resources.find(textureId);
		if (resourceIt == m_resources.end()) return;
		
		const TextureResource& resource = resourceIt->second;
		
		// 释放OpenGL纹理
		GLuint tex = static_cast<GLuint>(textureId);
		gl->glDeleteTextures(1, &tex);
		
		// 更新内存使用
		m_currentMemoryBytes -= resource.memorySize;
		
		// 从数据结构中移除
		m_resources.erase(resourceIt);
		
		// 从LRU列表中移除
		auto lruIt = m_lruMap.find(textureId);
		if (lruIt != m_lruMap.end()) {
			m_lruList.erase(lruIt->second);
			m_lruMap.erase(lruIt);
		}
		
		// 从key映射中查找并移除
		for (auto it = m_keyToTexture.begin(); it != m_keyToTexture.end(); ++it) {
			if (it->second == textureId) {
				m_keyToTexture.erase(it);
				break;
			}
		}
	}

}