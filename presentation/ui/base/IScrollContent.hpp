#pragma once

// 可选接口：子组件如果实现它，ScrollView 就能驱动滚动偏移、读取内容高度
class IScrollContent {
public:
    virtual ~IScrollContent() = default;
    virtual void setScrollOffset(int y) = 0;
    virtual int  contentHeight() const = 0;
};