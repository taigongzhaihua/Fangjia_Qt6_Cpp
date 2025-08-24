#pragma once
#include <qobject.h>
#include <qstring.h>
#include <qvector.h>
#include <qtmetamacros.h>

// 方剂数据模型
class FormulaViewModel final : public QObject
{
    Q_OBJECT
public:
    // 方剂详情
    struct FormulaDetail {
        QString name;           // 方剂名
        QString source;         // 出处
        QString composition;    // 组成
        QString usage;          // 用法
        QString function;       // 功效
        QString indication;     // 主治
        QString note;          // 备注
    };

    // 树节点
    struct TreeNode {
        QString id;            // 唯一标识
        QString label;         // 显示文本
        int level;            // 层级：0=分类，1=小类，2=方剂
        bool expanded;        // 是否展开
        int parentIndex;      // 父节点索引（-1为根）
        FormulaDetail* detail; // 方剂详情（仅叶子节点）
    };

    explicit FormulaViewModel(QObject* parent = nullptr);
    ~FormulaViewModel() override;

    // 数据管理
    void loadSampleData();  // 加载示例数据
    void clearData();

    // 树结构访问
    [[nodiscard]] const QVector<TreeNode>& nodes() const noexcept { return m_nodes; }
    [[nodiscard]] int nodeCount() const noexcept { return m_nodes.size(); }
    
    // 获取节点的子节点
    [[nodiscard]] QVector<int> childIndices(int parentIdx) const;
    
    // 选中管理
    [[nodiscard]] int selectedIndex() const noexcept { return m_selectedIdx; }
    void setSelectedIndex(int idx);
    
    // 展开/折叠
    void toggleExpanded(int idx);
    void setExpanded(int idx, bool expanded);
    
    // 获取选中的方剂详情
    [[nodiscard]] const FormulaDetail* selectedFormula() const;

signals:
    void dataChanged();
    void selectedChanged(int index);
    void nodeExpandChanged(int index, bool expanded);

private:
    void addCategory(const QString& id, const QString& label);
    void addSubCategory(const QString& id, const QString& label, int parentIdx);
    void addFormula(const QString& id, const QString& label, int parentIdx, FormulaDetail* detail);

private:
    QVector<TreeNode> m_nodes;
    int m_selectedIdx{ -1 };
};