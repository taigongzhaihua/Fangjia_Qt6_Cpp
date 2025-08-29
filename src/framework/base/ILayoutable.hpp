#pragma once
#include <limits>
#include <qrect.h>
#include <qsize.h>

// 可选：布局测量/安排接口（不破坏 IUiComponent）
// - 逻辑像素为单位
// - measure: 给定约束返回期望尺寸（不必等于最终安排尺寸）
// - arrange: 给出最终矩形（容器决定），子项可缓存并用于绘制/命中
struct SizeConstraints {
	int minW{ 0 }, minH{ 0 };
	int maxW{ std::numeric_limits<int>::max() / 2 };
	int maxH{ std::numeric_limits<int>::max() / 2 };

	static SizeConstraints tight(const QSize& s) {
		return SizeConstraints{ s.width(), s.height(), s.width(), s.height() };
	}
	static SizeConstraints widthBounded(const int maxW, const int maxH = std::numeric_limits<int>::max() / 2) {
		SizeConstraints c;
		c.maxW = std::max(0, maxW);
		c.maxH = std::max(0, maxH);
		return c;
	}
};

class ILayoutable {
public:
	virtual ~ILayoutable() = default;
	// 返回希望占用的尺寸（不超过 max 限制）
	virtual QSize measure(const SizeConstraints& cs) = 0;
	// 容器决定的最终放置矩形
	virtual void arrange(const QRect& finalRect) = 0;
};