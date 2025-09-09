#include "DataBus.hpp"
#include <algorithm>

namespace Render {

	bool DataBus::submit(const FrameData& data) {
		std::lock_guard<std::mutex> lock(m_mutex);
		
		// 清空之前的数据
		m_buffer.clear();
		
		// 拷贝新数据
		m_buffer.roundedRects = data.roundedRects;
		m_buffer.images = data.images;
		
		// 更新数据可用标志
		m_hasData.store(true);
		
		return true;
	}

	bool DataBus::consume(FrameData& outData) {
		std::lock_guard<std::mutex> lock(m_mutex);
		
		if (!m_hasData.load()) {
			return false;
		}
		
		// 移动数据到输出容器
		outData.clear();
		outData.roundedRects = std::move(m_buffer.roundedRects);
		outData.images = std::move(m_buffer.images);
		
		// 清理缓冲区
		m_buffer.clear();
		m_hasData.store(false);
		
		return true;
	}

	bool DataBus::hasData() const {
		return m_hasData.load();
	}

	void DataBus::clear() {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_buffer.clear();
		m_hasData.store(false);
	}

}