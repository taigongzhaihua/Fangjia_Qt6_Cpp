# 开发指南

## 项目结构

本项目采用分层架构设计：

### 核心层 (core)
- **rendering**: 渲染引擎、命令缓冲、纹理管理
- **platform**: 平台特定实现（Windows Chrome等）
- **config**: 配置管理
- **di**: 依赖注入

### 框架层 (framework)
- **base**: UI组件基础接口
- **containers**: 布局容器
- **widgets**: 可复用控件

### 模型层 (models)
- 业务数据模型（ViewModel）
- 应用状态管理

### 视图层 (views)
- 业务特定的复合视图
- 页面组件

### 应用层 (app)
- 主窗口
- 应用初始化
- 程序入口

## 开发流程

1. **添加新功能**
   - 在对应层创建新文件
   - 更新 CMakeLists.txt
   - 编写单元测试
   - 更新文档

2. **测试**
   ```bash
   ./build.sh Debug
   cd build-Debug
   ctest