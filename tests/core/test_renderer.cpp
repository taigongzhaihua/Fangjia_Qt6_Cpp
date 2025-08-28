#include <QtTest>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>
#include "../../src/core/rendering/Renderer.h"
#include "../../src/core/rendering/IconCache.h"
#include "../../src/core/rendering/RenderData.hpp"

class TestRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
    
private:
    QOpenGLContext* m_context{nullptr};
    QOffscreenSurface* m_surface{nullptr};
    Renderer m_renderer;
    IconCache m_iconCache;
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing Renderer tests...";
        
        // 创建OpenGL上下文
        m_surface = new QOffscreenSurface();
        m_surface->create();
        
        m_context = new QOpenGLContext();
        m_context->create();
        QVERIFY(m_context->isValid());
        
        m_context->makeCurrent(m_surface);
        initializeOpenGLFunctions();
        
        // 初始化渲染器
        m_renderer.initializeGL(this);
    }
    
    void cleanupTestCase() {
        m_context->makeCurrent(m_surface);
        m_renderer.releaseGL();
        m_iconCache.releaseAll(this);
        
        delete m_context;
        delete m_surface;
        
        qDebug() << "Renderer tests completed.";
    }
    
    void testRendererInitialization() {
        QVERIFY(m_context->isValid());
        
        // 测试resize
        m_renderer.resize(1920, 1080);
        // 这里可以验证内部状态，但需要公开一些接口
    }
    
    void testFrameDataCreation() {
        Render::FrameData fd;
        
        // 添加圆角矩形
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(10, 10, 100, 50),
            .radiusPx = 5.0f,
            .color = QColor(255, 0, 0, 128)
        });
        
        QCOMPARE(fd.roundedRects.size(), 1);
        QVERIFY(!fd.empty());
        
        // 清空
        fd.clear();
        QVERIFY(fd.empty());
    }
    
    void testIconCache() {
        m_context->makeCurrent(m_surface);
        
        // 测试文本纹理
        QFont font;
        font.setPixelSize(16);
        QString text = "Test";
        QColor color(255, 255, 255);
        
        int texId = m_iconCache.ensureTextPx(
            "test_key", font, text, color, this
        );
        
        QVERIFY(texId > 0);
        
        // 测试纹理尺寸查询
        QSize size = m_iconCache.textureSizePx(texId);
        QVERIFY(size.width() > 0);
        QVERIFY(size.height() > 0);
        
        // 测试缓存（再次请求应返回相同ID）
        int texId2 = m_iconCache.ensureTextPx(
            "test_key", font, text, color, this
        );
        QCOMPARE(texId2, texId);
    }
    
    void testRenderCommandBatch() {
        Render::FrameData fd;
        
        // 添加多个命令
        for (int i = 0; i < 10; ++i) {
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(i * 10, i * 10, 50, 50),
                .radiusPx = 3.0f,
                .color = QColor(i * 25, 0, 0, 255)
            });
        }
        
        QCOMPARE(fd.roundedRects.size(), 10);
        
        // 测试绘制（不会崩溃即可）
        m_context->makeCurrent(m_surface);
        m_renderer.drawFrame(fd, m_iconCache, 1.0f);
    }
    
    void testDataBus() {
        Render::DataBus bus;
        Render::FrameData fd1, fd2;
        
        // 初始状态：没有数据
        QVERIFY(!bus.consume(fd2));
        
        // 提交数据
        fd1.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(0, 0, 100, 100),
            .radiusPx = 10.0f,
            .color = QColor(255, 255, 255)
        });
        bus.submit(fd1);
        
        // 消费数据
        QVERIFY(bus.consume(fd2));
        QCOMPARE(fd2.roundedRects.size(), 1);
        
        // 再次消费应该失败
        QVERIFY(!bus.consume(fd2));
    }
};

#include "test_renderer.moc"

QTEST_MAIN(TestRenderer)