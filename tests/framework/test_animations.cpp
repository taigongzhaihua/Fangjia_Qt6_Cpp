#include <QtTest>
#include <QElapsedTimer>
#include <cmath>

// 测试动画插值函数
class TestAnimations : public QObject
{
    Q_OBJECT
    
private:
    static float easeInOut(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
    
    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing Animation tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "Animation tests completed.";
    }
    
    void testEaseInOut() {
        // 测试边界值
        QVERIFY(qFuzzyCompare(easeInOut(0.0f), 0.0f));
        QVERIFY(qFuzzyCompare(easeInOut(1.0f), 1.0f));
        
        // 测试中间值
        float mid = easeInOut(0.5f);
        QVERIFY(mid > 0.4f && mid < 0.6f);
        
        // 测试缓动特性
        float early = easeInOut(0.1f);
        float late = easeInOut(0.9f);
        QVERIFY(early < 0.1f * 2); // 开始时缓慢
        QVERIFY(late > 0.9f * 0.8); // 结束时缓慢
    }
    
    void testLerp() {
        // 基本插值
        QVERIFY(qFuzzyCompare(lerp(0.0f, 100.0f, 0.0f), 0.0f));
        QVERIFY(qFuzzyCompare(lerp(0.0f, 100.0f, 1.0f), 100.0f));
        QVERIFY(qFuzzyCompare(lerp(0.0f, 100.0f, 0.5f), 50.0f));
        
        // 负值插值
        QVERIFY(qFuzzyCompare(lerp(-50.0f, 50.0f, 0.5f), 0.0f));
        
        // 反向插值
        QVERIFY(qFuzzyCompare(lerp(100.0f, 0.0f, 0.25f), 75.0f));
    }
    
    void testAnimationTiming() {
        QElapsedTimer timer;
        timer.start();
        
        // 模拟60fps动画
        const int targetFps = 60;
        const int frameDuration = 1000 / targetFps; // ~16ms
        
        QTest::qWait(frameDuration);
        
        qint64 elapsed = timer.elapsed();
        
        // 允许一定误差
        QVERIFY(elapsed >= frameDuration - 5);
        QVERIFY(elapsed <= frameDuration + 10);
    }
    
    void testAnimationSequence() {
        struct Animation {
            float start;
            float end;
            int duration;
            float current;
            bool active;
        };
        
        Animation anim{0.0f, 100.0f, 1000, 0.0f, true};
        QElapsedTimer timer;
        timer.start();
        
        // 模拟动画更新
        while (anim.active && timer.elapsed() < 2000) {
            qint64 elapsed = timer.elapsed();
            float t = static_cast<float>(elapsed) / static_cast<float>(anim.duration);
            
            if (t >= 1.0f) {
                t = 1.0f;
                anim.active = false;
            }
            
            anim.current = lerp(anim.start, anim.end, easeInOut(t));
            
            QTest::qWait(16); // 模拟帧间隔
        }
        
        QVERIFY(!anim.active);
        QVERIFY(qFuzzyCompare(anim.current, anim.end));
    }
    
    void testMultipleAnimations() {
        // 测试多个并行动画
        struct AnimState {
            float alpha{0.0f};
            float x{0.0f};
            float y{0.0f};
        };
        
        AnimState state;
        
        // 同时动画多个属性
        const int steps = 10;
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            float eased = easeInOut(t);
            
            state.alpha = lerp(0.0f, 1.0f, eased);
            state.x = lerp(-100.0f, 100.0f, eased);
            state.y = lerp(0.0f, 50.0f, eased);
        }
        
        QVERIFY(qFuzzyCompare(state.alpha, 1.0f));
        QVERIFY(qFuzzyCompare(state.x, 100.0f));
        QVERIFY(qFuzzyCompare(state.y, 50.0f));
    }
};