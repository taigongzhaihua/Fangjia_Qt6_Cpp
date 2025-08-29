# 渲染与 UI 体系

## 坐标系与 DPR
- 输入：逻辑像素（左上原点）
- 转换：乘以 DPR 得设备像素
- 输出：标准 OpenGL NDC（-1..1）
- 剪裁：左上原点的逻辑矩形→OpenGL 底左原点的像素矩形

## RenderData
- RoundedRectCmd：圆角矩形（含 clipRect）
- ImageCmd：纹理（dstRect 逻辑像素，srcRect 设备像素，tint 调制）
- FrameData：一帧内的命令集合，按类型批量绘制

## Renderer
- 两套着色器：圆角矩形 / 纹理
- VAO/VBO 按需更新顶点，减少状态切换
- glScissor 管理剪裁开关与矩形

## 文本与图标
- IconCache：SVG/字体/文本栅格化为 QImage → OpenGL 纹理
- 白膜着色策略：纹理转白色蒙版，渲染阶段使用 tint 上色