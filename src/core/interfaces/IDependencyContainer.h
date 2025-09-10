#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <string>

namespace Fangjia::Core {

/**
 * @brief 依赖注入容器接口
 * 基于PR #21中统一依赖注入的设计原则
 * 
 * 设计原则：
 * - 编译时类型安全
 * - 清晰的生命周期管理
 * - 支持单例和多例模式
 * - 易于测试和替换实现
 */
class IDependencyContainer {
public:
    virtual ~IDependencyContainer() = default;

    // 服务注册
    template<typename Interface, typename Implementation>
    void registerSingleton() {
        registerSingletonImpl(
            std::type_index(typeid(Interface)),
            []() -> std::shared_ptr<void> {
                return std::static_pointer_cast<void>(
                    std::make_shared<Implementation>()
                );
            }
        );
    }

    template<typename Interface, typename Implementation>
    void registerTransient() {
        registerTransientImpl(
            std::type_index(typeid(Interface)),
            []() -> std::shared_ptr<void> {
                return std::static_pointer_cast<void>(
                    std::make_shared<Implementation>()
                );
            }
        );
    }

    template<typename Interface>
    void registerInstance(std::shared_ptr<Interface> instance) {
        registerInstanceImpl(
            std::type_index(typeid(Interface)),
            std::static_pointer_cast<void>(instance)
        );
    }

    // 服务解析
    template<typename T>
    std::shared_ptr<T> resolve() {
        auto ptr = resolveImpl(std::type_index(typeid(T)));
        return std::static_pointer_cast<T>(ptr);
    }

    template<typename T>
    bool isRegistered() const {
        return isRegisteredImpl(std::type_index(typeid(T)));
    }

protected:
    using FactoryFunction = std::function<std::shared_ptr<void>()>;

    virtual void registerSingletonImpl(std::type_index type, FactoryFunction factory) = 0;
    virtual void registerTransientImpl(std::type_index type, FactoryFunction factory) = 0;
    virtual void registerInstanceImpl(std::type_index type, std::shared_ptr<void> instance) = 0;
    virtual std::shared_ptr<void> resolveImpl(std::type_index type) = 0;
    virtual bool isRegisteredImpl(std::type_index type) const = 0;
};

/**
 * @brief 服务定位器接口
 * 基于PR #21中简化的依赖访问模式
 */
class IServiceLocator {
public:
    virtual ~IServiceLocator() = default;

    template<typename T>
    std::shared_ptr<T> get() {
        auto typeIndex = std::type_index(typeid(T));
        auto ptr = getImpl(typeIndex);
        return std::static_pointer_cast<T>(ptr);
    }

    template<typename T>
    bool has() const {
        return hasImpl(std::type_index(typeid(T)));
    }

protected:
    virtual std::shared_ptr<void> getImpl(std::type_index type) = 0;
    virtual bool hasImpl(std::type_index type) const = 0;
};

/**
 * @brief 配置接口
 * 基于PR #21中主题和参数管理的模式
 */
class IConfiguration {
public:
    virtual ~IConfiguration() = default;

    // 基本配置访问
    virtual bool getBool(const std::string& key, bool defaultValue = false) const = 0;
    virtual int getInt(const std::string& key, int defaultValue = 0) const = 0;
    virtual float getFloat(const std::string& key, float defaultValue = 0.0f) const = 0;
    virtual std::string getString(const std::string& key, const std::string& defaultValue = "") const = 0;

    // 配置更新
    virtual void setBool(const std::string& key, bool value) = 0;
    virtual void setInt(const std::string& key, int value) = 0;
    virtual void setFloat(const std::string& key, float value) = 0;
    virtual void setString(const std::string& key, const std::string& value) = 0;

    // 配置持久化
    virtual void save() = 0;
    virtual void load() = 0;

    // 配置变更通知
    using ConfigChangedCallback = std::function<void(const std::string& key)>;
    virtual void onConfigChanged(ConfigChangedCallback callback) = 0;
};

/**
 * @brief 主题管理接口
 * 基于PR #21中主题切换和颜色管理的设计
 */
class IThemeManager {
public:
    virtual ~IThemeManager() = default;

    // 主题状态
    virtual bool isDarkTheme() const = 0;
    virtual void setDarkTheme(bool isDark) = 0;
    virtual void toggleTheme() = 0;

    // 主题变更通知
    using ThemeChangedCallback = std::function<void(bool isDark)>;
    virtual void onThemeChanged(ThemeChangedCallback callback) = 0;

    // 系统主题跟随 (PR #21特性)
    virtual bool isFollowingSystem() const = 0;
    virtual void setFollowingSystem(bool follow) = 0;
    virtual bool detectSystemTheme() = 0;
};

} // namespace Fangjia::Core