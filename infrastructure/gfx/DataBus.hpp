/*
 * 文件名：DataBus.hpp
 * 职责：渲染数据总线，负责线程安全的渲染数据传递与交换
 * 依赖：Qt6 Core、RenderData.hpp
 * 线程：线程安全，支持多线程生产者-消费者模式
 * 备注：使用双缓冲机制，确保渲染线程与UI线程之间的无锁数据传递
 */

#pragma once
#include "RenderData.hpp"
#include <atomic>
#include <mutex>

namespace Render {

	/// 渲染数据总线：线程安全的帧数据传递系统
	/// 
	/// 功能：
	/// - 支持多线程环境下的渲染数据传递
	/// - 采用双缓冲机制避免数据竞争
	/// - 提供非阻塞的数据提交与消费接口
	/// - 自动处理帧数据的生命周期管理
	/// 
	/// 使用场景：
	/// - UI线程生成渲染命令，渲染线程消费执行
	/// - 多个组件并发提交渲染数据
	/// - 渲染管线的数据流控制
	class DataBus {
	public:
		DataBus() = default;
		~DataBus() = default;

		// 禁用拷贝，确保唯一性
		DataBus(const DataBus&) = delete;
		DataBus& operator=(const DataBus&) = delete;

		/// 功能：提交渲染数据到总线
		/// 参数：data — 要提交的帧数据
		/// 说明：线程安全，数据会被拷贝存储
		/// 返回：true表示提交成功，false表示总线忙碌
		bool submit(const FrameData& data);

		/// 功能：从总线消费渲染数据  
		/// 参数：outData — 输出的帧数据容器
		/// 返回：true表示成功获取数据，false表示暂无数据
		/// 说明：线程安全，会清空当前缓冲区数据
		bool consume(FrameData& outData);

		/// 功能：检查是否有待消费的数据
		/// 返回：true表示有数据等待消费
		/// 说明：线程安全，无副作用
		[[nodiscard]] bool hasData() const;

		/// 功能：清空所有缓冲数据
		/// 说明：线程安全，用于重置或清理
		void clear();

	private:
		std::mutex m_mutex;                    // 保护缓冲区的互斥锁
		FrameData m_buffer;                    // 数据缓冲区
		std::atomic<bool> m_hasData{false};    // 快速检查标志
	};

}