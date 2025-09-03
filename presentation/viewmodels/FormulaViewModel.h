/*
 * 文件名：FormulaViewModel.h
 * 职责：方剂数据的业务逻辑管理，提供分层树状结构和选中状态管理。
 * 依赖：Qt6 Core（QObject信号机制）。
 * 线程：仅在主线程使用。
 * 备注：实现方剂分类体系的树状数据模型，支持展开/折叠和详情查看。
 */

#pragma once
#include <memory>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include <qvector.h>

 // Forward declarations
namespace domain::services {
	class IFormulaService;
}

namespace domain::entities {
	struct FormulaDetail;
}

/// 方剂数据视图模型：管理中医方剂的分层数据结构和交互状态
/// 
/// 数据层次：
/// - 第0层：大类（如"解表剂"）
/// - 第1层：小类（如"辛温解表"）  
/// - 第2层：具体方剂（如"麻黄汤"）
/// 
/// 功能：
/// - 树状节点的展开/折叠状态管理
/// - 方剂选中状态与详情数据绑定
/// - 分类体系的动态加载和清理
/// - 信号发射通知UI更新
class FormulaViewModel final : public QObject
{
	Q_OBJECT
public:
	/// 方剂详细信息结构
	struct FormulaDetail {
		QString name;           // 方剂名称
		QString source;         // 方剂出处（典籍来源）
		QString composition;    // 药物组成
		QString usage;          // 用法用量
		QString function;       // 功效作用
		QString indication;     // 主治病证
		QString note;          // 备注说明
	};

	/// 树状节点数据结构
	struct TreeNode {
		QString id;            // 节点唯一标识符
		QString label;         // 显示文本内容
		int level;            // 层级深度：0=大类，1=小类，2=方剂
		bool expanded;        // 节点展开状态
		int parentIndex;      // 父节点索引（-1表示根节点）
		FormulaDetail* detail; // 方剂详情（仅叶子节点非空）
	};

	explicit FormulaViewModel(QObject* parent = nullptr);

	/// Constructor with service injection
	/// Parameters: service - Formula service for data access, parent - QObject parent
	/// Note: If service is null or unavailable, falls back to sample data
	explicit FormulaViewModel(std::shared_ptr<domain::services::IFormulaService> service, QObject* parent = nullptr);

	~FormulaViewModel() override;

	/// 功能：加载示例方剂数据
	/// 说明：构建完整的方剂分类体系，包含经典方剂的详细信息
	void loadSampleData();

	/// 功能：加载数据（优先从服务，回退到示例数据）
	/// 说明：如果注入了服务且可用，从服务加载；否则加载示例数据
	void loadData();

	/// 功能：清空所有数据
	/// 说明：释放方剂详情内存并清空节点列表
	void clearData();

	/// 功能：获取所有树节点的只读访问
	/// 返回：包含所有节点的向量引用
	[[nodiscard]] const QVector<TreeNode>& nodes() const noexcept { return m_nodes; }

	/// 功能：获取节点总数
	/// 返回：当前节点数量
	[[nodiscard]] int nodeCount() const noexcept { return m_nodes.size(); }

	/// 功能：获取指定节点的所有子节点索引
	/// 参数：parentIdx — 父节点索引
	/// 返回：子节点索引列表
	[[nodiscard]] QVector<int> childIndices(int parentIdx) const;

	/// 功能：获取当前选中节点索引
	/// 返回：选中节点的索引，-1表示无选中
	[[nodiscard]] int selectedIndex() const noexcept { return m_selectedIdx; }

	/// 功能：设置选中节点
	/// 参数：idx — 要选中的节点索引
	/// 说明：会发射selectionChanged信号通知UI更新
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

	/// Load data from injected service
	void loadDataFromService();

	/// Convert domain entity to ViewModel structure
	FormulaDetail* convertDomainDetail(const domain::entities::FormulaDetail& domainDetail) const;

private:
	QVector<TreeNode> m_nodes;
	int m_selectedIdx{ -1 };
	std::shared_ptr<domain::services::IFormulaService> m_formulaService;

	/// Flag to track object destruction state
	/// This prevents memory access violations when callbacks try to access
	/// the object during or after destruction (e.g., from BindingHost connections)
	bool m_isDestroying{ false };
};