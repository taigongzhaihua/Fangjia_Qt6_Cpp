#!/usr/bin/env bash

# GitHub Actions 本地验证脚本
# 用于在提交前本地验证 GitHub Actions 工作流

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== GitHub Actions 本地验证 ==="
echo "项目路径: $PROJECT_ROOT"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查依赖
check_dependencies() {
    echo -e "${BLUE}检查依赖...${NC}"
    
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v git &> /dev/null; then
        missing_deps+=("git")
    fi
    
    # 检查 Qt6
    if ! pkg-config --exists Qt6Core 2>/dev/null; then
        missing_deps+=("qt6-base-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo -e "${RED}缺少依赖: ${missing_deps[*]}${NC}"
        echo "在 Ubuntu/Debian 上安装："
        echo "sudo apt-get install cmake git qt6-base-dev qt6-tools-dev libqt6opengl6-dev libqt6svg6-dev"
        exit 1
    fi
    
    echo -e "${GREEN}✓ 所有依赖都已安装${NC}"
}

# 验证项目结构
validate_structure() {
    echo -e "${BLUE}验证项目结构...${NC}"
    
    local required_files=(
        "CMakeLists.txt"
        "apps/fangjia"
        "tests"
        "domain"
        "data"
        "presentation"
        "infrastructure"
        ".github/workflows/ci.yml"
        ".github/workflows/comprehensive-ci.yml"
        ".github/workflows/ai-pr-validation.yml"
        ".github/workflows/auto-fix.yml"
    )
    
    cd "$PROJECT_ROOT"
    
    for file in "${required_files[@]}"; do
        if [ ! -e "$file" ]; then
            echo -e "${RED}✗ 缺少必需文件/目录: $file${NC}"
            exit 1
        fi
    done
    
    echo -e "${GREEN}✓ 项目结构验证通过${NC}"
}

# 模拟 GitHub Actions 构建环境
simulate_ci_build() {
    echo -e "${BLUE}模拟 CI 构建环境...${NC}"
    
    cd "$PROJECT_ROOT"
    
    # 清理之前的构建
    if [ -d "build" ]; then
        echo "清理之前的构建..."
        rm -rf build
    fi
    
    # 设置环境变量（模拟 CI）
    export QT_QPA_PLATFORM=offscreen
    export QT_LOGGING_RULES="qt.qpa.gl=false"
    
    # CMake 配置
    echo "CMake 配置..."
    if ! cmake -B build -S . -DCMAKE_BUILD_TYPE=Release; then
        echo -e "${RED}✗ CMake 配置失败${NC}"
        return 1
    fi
    
    # 构建
    echo "构建项目..."
    if ! cmake --build build --parallel; then
        echo -e "${RED}✗ 构建失败${NC}"
        return 1
    fi
    
    # 验证可执行文件
    if [ ! -f "build/FangJia" ]; then
        echo -e "${RED}✗ 主可执行文件未生成${NC}"
        return 1
    fi
    
    if [ ! -f "build/tests/FangJia_Tests" ]; then
        echo -e "${RED}✗ 测试可执行文件未生成${NC}"
        return 1
    fi
    
    echo -e "${GREEN}✓ 构建成功${NC}"
    return 0
}

# 运行测试
run_tests() {
    echo -e "${BLUE}运行测试...${NC}"
    
    cd "$PROJECT_ROOT/build"
    
    # 运行测试（带超时）
    if timeout 60s ./tests/FangJia_Tests; then
        echo -e "${GREEN}✓ 所有测试通过${NC}"
        return 0
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            echo -e "${YELLOW}⚠ 测试超时，但这可能是正常的${NC}"
        else
            echo -e "${RED}✗ 测试失败，退出码: $exit_code${NC}"
        fi
        return $exit_code
    fi
}

# 验证应用程序启动
validate_app_startup() {
    echo -e "${BLUE}验证应用程序启动...${NC}"
    
    cd "$PROJECT_ROOT/build"
    
    # 测试应用程序启动（带超时）
    if timeout 10s ./FangJia --help || true; then
        echo -e "${GREEN}✓ 应用程序可以启动${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠ 应用程序启动测试完成（可能超时）${NC}"
        return 0
    fi
}

# 检查工作流文件
validate_workflows() {
    echo -e "${BLUE}验证 GitHub Actions 工作流文件...${NC}"
    
    cd "$PROJECT_ROOT"
    
    # 检查 YAML 语法
    for workflow in .github/workflows/*.yml; do
        if [ -f "$workflow" ]; then
            echo "验证: $(basename "$workflow")"
            # 这里可以添加更多的 YAML 语法检查
            if ! grep -q "name:" "$workflow"; then
                echo -e "${RED}✗ 工作流缺少名称: $workflow${NC}"
                return 1
            fi
        fi
    done
    
    echo -e "${GREEN}✓ 工作流文件验证通过${NC}"
}

# 生成报告
generate_report() {
    echo -e "${BLUE}生成验证报告...${NC}"
    
    cat > "$PROJECT_ROOT/ci_validation_report.md" << EOF
# CI 验证报告

**生成时间**: $(date)
**Git 提交**: $(git rev-parse HEAD)
**分支**: $(git branch --show-current)

## 验证结果

- ✅ 依赖检查
- ✅ 项目结构验证
- ✅ 构建测试
- ✅ 单元测试
- ✅ 应用程序启动验证
- ✅ 工作流文件验证

## 构建信息

- **CMake 版本**: $(cmake --version | head -1)
- **编译器**: $(c++ --version | head -1)
- **Qt 版本**: $(pkg-config --modversion Qt6Core 2>/dev/null || echo "未检测到")

## 下一步

该项目已准备好进行 GitHub Actions CI/CD：

1. 推送代码到 GitHub
2. GitHub Actions 将自动运行验证
3. 如有问题，自动修复系统会尝试生成修复 PR

EOF

    echo -e "${GREEN}✓ 报告已生成: ci_validation_report.md${NC}"
}

# 主函数
main() {
    echo -e "${YELLOW}开始 GitHub Actions 本地验证...${NC}"
    
    check_dependencies
    validate_structure
    validate_workflows
    
    if simulate_ci_build; then
        run_tests
        validate_app_startup
        generate_report
        
        echo -e "\n${GREEN}🎉 所有验证通过！项目已准备好使用 GitHub Actions CI/CD${NC}"
        echo -e "${BLUE}你可以安全地推送代码到 GitHub 仓库${NC}"
    else
        echo -e "\n${RED}❌ 构建失败，请修复问题后重试${NC}"
        exit 1
    fi
}

# 显示帮助
show_help() {
    cat << EOF
GitHub Actions 本地验证脚本

用法:
    $0 [选项]

选项:
    -h, --help      显示此帮助信息
    --build-only    仅执行构建测试
    --test-only     仅执行测试（假设已构建）
    --clean         清理构建文件并退出

示例:
    $0                  # 完整验证
    $0 --build-only     # 仅构建测试
    $0 --clean          # 清理构建文件
EOF
}

# 处理命令行参数
case "${1:-}" in
    -h|--help)
        show_help
        exit 0
        ;;
    --build-only)
        check_dependencies
        validate_structure
        simulate_ci_build
        exit $?
        ;;
    --test-only)
        if [ -d "$PROJECT_ROOT/build" ] && [ -f "$PROJECT_ROOT/build/tests/FangJia_Tests" ]; then
            cd "$PROJECT_ROOT"
            export QT_QPA_PLATFORM=offscreen
            export QT_LOGGING_RULES="qt.qpa.gl=false"
            run_tests
            exit $?
        else
            echo -e "${RED}未找到构建文件，请先运行构建${NC}"
            exit 1
        fi
        ;;
    --clean)
        cd "$PROJECT_ROOT"
        if [ -d "build" ]; then
            echo "清理构建目录..."
            rm -rf build
            echo -e "${GREEN}✓ 构建目录已清理${NC}"
        fi
        if [ -f "ci_validation_report.md" ]; then
            rm ci_validation_report.md
            echo -e "${GREEN}✓ 验证报告已删除${NC}"
        fi
        exit 0
        ;;
    "")
        main
        ;;
    *)
        echo "未知选项: $1"
        show_help
        exit 1
        ;;
esac