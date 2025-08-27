#include "UiFormulaView.h"

#include "FormulaViewModel.h"
#include "UiTreeList.h"

#include <IconLoader.h>
#include <RenderData.hpp>

#include <RebuildHost.h>
#include <UI.h>

#include <algorithm>
#include <cmath>
#include <ILayoutable.hpp>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qfont.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <UiPanel.h>
#include <Widget.h>

using namespace UI;

// ====== VM -> UiTreeList::Model 适配器 ======
class UiFormulaView::VmTreeAdapter : public UiTreeList::Model
{
public:
	explicit VmTreeAdapter(FormulaViewModel* vm) : m_vm(vm)
	{
	}

	QVector<int> rootIndices() const override
	{
		QVector<int> roots;
		if (!m_vm) return roots;
		const auto& nodes = m_vm->nodes();
		for (int i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].parentIndex == -1) roots.push_back(i);
		}
		return roots;
	}

	QVector<int> childIndices(int nodeId) const override
	{
		return m_vm ? m_vm->childIndices(nodeId) : QVector<int>{};
	}

	UiTreeList::NodeInfo nodeInfo(int nodeId) const override
	{
		UiTreeList::NodeInfo info;
		if (!m_vm) return info;
		const auto& nodes = m_vm->nodes();
		if (nodeId < 0 || nodeId >= nodes.size()) return info;
		info.label = nodes[nodeId].label;
		info.level = nodes[nodeId].level;
		info.expanded = nodes[nodeId].expanded;
		return info;
	}

	int selectedId() const override { return m_vm ? m_vm->selectedIndex() : -1; }
	void setSelectedId(int nodeId) override { if (m_vm) m_vm->setSelectedIndex(nodeId); }
	void setExpanded(int nodeId, bool on) override { if (m_vm) m_vm->setExpanded(nodeId, on); }

private:
	FormulaViewModel* m_vm{ nullptr };
};

// ====== 中间分割条（固定宽度 1px） ======
class UiFormulaView::VSplitter : public IUiComponent, public IUiContent
{
public:
	explicit VSplitter(QColor color, int logicalW = 1) : m_color(color), m_w(std::max(1, logicalW))
	{
	}

	void setViewportRect(const QRect& r) override { m_viewport = r; }

	void updateLayout(const QSize&) override
	{
	}

	void updateResourceContext(IconLoader&, QOpenGLFunctions*, float) override
	{
	}

	void append(Render::FrameData& fd) const override
	{
		if (!m_viewport.isValid()) return;
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport), .radiusPx = 0.0f, .color = m_color, .clipRect = QRectF(m_viewport)
			});
	}

	bool onMousePress(const QPoint&) override { return false; }
	bool onMouseMove(const QPoint&) override { return false; }
	bool onMouseRelease(const QPoint&) override { return false; }
	bool tick() override { return false; }
	QRect bounds() const override { return { 0, 0, m_w, 0 }; }

	void onThemeChanged(bool) override
	{
	}

private:
	QColor m_color;
	int m_w{ 1 };
	QRect m_viewport;
};

// ====== 宽度提示包装器：为 Panel 提供主轴方向“期望宽度” ======
class UiFormulaView::WidthHint : public IUiComponent, public IUiContent, public ILayoutable
{
public:
	explicit WidthHint(IUiComponent* wrapped, int preferredW = 0) : m_child(wrapped), m_prefW(std::max(0, preferredW))
	{
	}

	void setPreferredWidth(int w) { m_prefW = std::max(0, w); }

	void setViewportRect(const QRect& r) override
	{
		m_viewport = r;
		if (auto* c = dynamic_cast<IUiContent*>(m_child)) c->setViewportRect(r);
	}

	QSize measure(const SizeConstraints& cs) override
	{
		const int w = std::clamp(m_prefW, cs.minW, cs.maxW);
		const int h = std::clamp(0, cs.minH, cs.maxH);
		return { w, h };
	}

	void arrange(const QRect& finalRect) override
	{
		setViewportRect(finalRect);
		if (auto* l = dynamic_cast<ILayoutable*>(m_child)) l->arrange(finalRect);
	}

	void updateLayout(const QSize& windowSize) override { if (m_child) m_child->updateLayout(windowSize); }

	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override
	{
		if (m_child) m_child->updateResourceContext(loader, gl, dpr);
	}

	void append(Render::FrameData& fd) const override { if (m_child) m_child->append(fd); }
	bool onMousePress(const QPoint& pos) override { return m_child ? m_child->onMousePress(pos) : false; }
	bool onMouseMove(const QPoint& pos) override { return m_child ? m_child->onMouseMove(pos) : false; }
	bool onMouseRelease(const QPoint& pos) override { return m_child ? m_child->onMouseRelease(pos) : false; }
	bool tick() override { return m_child ? m_child->tick() : false; }
	QRect bounds() const override { return { 0, 0, std::max(0, m_prefW), 0 }; }
	void onThemeChanged(bool isDark) override { if (m_child) m_child->onThemeChanged(isDark); }

private:
	IUiComponent* m_child{ nullptr }; // 非拥有
	int m_prefW{ 0 };
	QRect m_viewport;
};

// ====== UiFormulaView 实现（UiPanel-based） ======
UiFormulaView::UiFormulaView()
	: UiPanel(Orientation::Horizontal)
{
	qDebug() << "[UiFormulaView] ctor (UiPanel-based)";

	// 1) VM + 左树
	m_vm = std::make_unique<FormulaViewModel>();
	m_tree = std::make_unique<UiTreeList>();
	m_adapter = std::make_unique<VmTreeAdapter>(m_vm.get());
	m_tree->setModel(m_adapter.get());

	// 2) 右侧详情：RebuildHost
	m_detailHost = std::make_unique<RebuildHost>();
	m_detailHost->setBuilder([this]() -> std::unique_ptr<IUiComponent>
		{
			const auto* detail = m_vm->selectedFormula();

			const QColor cardBg = m_isDark ? QColor(32, 38, 46, 0) : QColor(255, 255, 255, 0);
			const QColor title = m_isDark ? QColor(250, 252, 255) : QColor(20, 25, 30);
			const QColor label = m_isDark ? QColor(100, 160, 220) : QColor(60, 120, 180);
			const QColor body = m_isDark ? QColor(255, 255, 255, 230) : QColor(50, 55, 60, 230);

			WidgetPtr bodyWidget;
			if (!detail)
			{
				bodyWidget = container(
					text("请从左侧列表选择一个方剂")->fontSize(14)->align(Qt::AlignCenter)
				)->alignment(Alignment::Center);
			}
			else
			{
				auto section = [&](const QString& head, const QString& content) -> WidgetPtr
					{
						if (content.isEmpty()) return container();
						return panel({
								text(head + "：")
								->fontSize(13)
								->fontWeight(QFont::DemiBold)
								->color(label)
								->align(Qt::AlignVCenter | Qt::AlignLeft),
								container(
									text(content)
									->fontSize(14)
									->color(body)
									->wrap(true)
								)->padding(20, 0)
							})->horizontal()
							->spacing(12)
							->crossAxisAlignment(Alignment::Stretch)
							->margin(20, 0);
					};

				bodyWidget = panel({
						text(detail->name)->fontSize(22)
										  ->fontWeight(QFont::Bold)
										  ->color(title),
						container()->height(16),
						section("出处", detail->source),
						section("组成", detail->composition),
						section("用法", detail->usage),
						section("功效", detail->function),
						section("主治", detail->indication),
						section("备注", detail->note)
					})->vertical()
					->spacing(16)
					->padding(20, 30, 20, 10);
			}

			auto root =
				container(bodyWidget)->alignment(Alignment::Stretch)
				->background(cardBg, 0.0f);
			return root->build();
		});
	// 重要：首次构建一次（避免初始为空）
	m_detailHost->requestRebuild();

	// 3) 监听 VM 变化 => 刷新树/重建详情
	QObject::connect(m_vm.get(), &FormulaViewModel::selectedChanged, [this](int)
		{
			if (m_tree) m_tree->reloadData();
			if (m_detailHost) m_detailHost->requestRebuild();
		});
	QObject::connect(m_vm.get(), &FormulaViewModel::dataChanged, [this]
		{
			if (m_tree) m_tree->reloadData();
			if (m_detailHost) m_detailHost->requestRebuild();
		});
	QObject::connect(m_vm.get(), &FormulaViewModel::nodeExpandChanged, [this](int, bool)
		{
			if (m_tree) m_tree->reloadData();
		});

	// 4) 加载示例数据
	m_vm->loadSampleData();

	// 5) 初始调色 + 构建布局子项
	applyPalettes();
	buildChildren();
}

UiFormulaView::~UiFormulaView() = default;

void UiFormulaView::buildChildren()
{
	// 清空旧子项
	clearChildren();

	// Panel 属性
	setSpacing(0);

	// 分割条颜色随主题
	const QColor splitClr = m_isDark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 25);
	m_splitter = std::make_unique<VSplitter>(splitClr, 1);

	// 创建宽度提示包装器（默认左侧 320px，右侧先置 400px，实际会在 setViewportRect 中根据比例更新）
	m_treeWrap = std::make_unique<WidthHint>(m_tree.get(), 320);
	m_detailWrap = std::make_unique<WidthHint>(m_detailHost.get(), 400);

	// 添加子项：左树（Stretch 交叉轴拉伸）、分割条（1px）、右详情（Stretch）
	addChild(m_treeWrap.get(), CrossAlign::Stretch);
	addChild(m_splitter.get(), CrossAlign::Stretch);
	addChild(m_detailWrap.get(), CrossAlign::Stretch);
}

void UiFormulaView::setDarkTheme(bool dark)
{
	if (m_isDark == dark) return;
	m_isDark = dark;
	applyPalettes();
	if (m_detailHost) m_detailHost->requestRebuild();
	UiPanel::onThemeChanged(m_isDark);
}

void UiFormulaView::applyPalettes() const
{
	if (!m_tree) return;
	if (m_isDark)
	{
		m_tree->setPalette(UiTreeList::Palette{
			.bg = QColor(28, 34, 42, 0),
			.itemHover = QColor(255, 255, 255, 15),
			.itemSelected = QColor(0, 122, 255, 35),
			.expandIcon = QColor(180, 185, 190, 200),
			.textPrimary = QColor(240, 245, 250, 255),
			.textSecondary = QColor(180, 190, 200, 220),
			.separator = QColor(255, 255, 255, 20)
			});
	}
	else
	{
		m_tree->setPalette(UiTreeList::Palette{
			.bg = QColor(255, 255, 255, 0),
			.itemHover = QColor(0, 0, 0, 10),
			.itemSelected = QColor(0, 122, 255, 25),
			.expandIcon = QColor(100, 105, 110, 200),
			.textPrimary = QColor(32, 38, 46, 255),
			.textSecondary = QColor(100, 110, 120, 200),
			.separator = QColor(0, 0, 0, 25)
			});
	}
}

void UiFormulaView::onThemeChanged(bool isDark)
{
	m_isDark = isDark;
	applyPalettes();
	if (m_detailHost) m_detailHost->requestRebuild();

	// 交给 UiPanel 继续把主题往子项传
	UiPanel::onThemeChanged(isDark);
}

// 在获取到页面 viewport 时，按比例计算左右两侧的“期望宽度”，下发给包装器
void UiFormulaView::setViewportRect(const QRect& r)
{
	UiPanel::setViewportRect(r);

	const int totalW = std::max(0, r.width());
	constexpr int splitterW = 1;
	// 可设置一个左侧最小/最大宽度，避免过窄或过宽
	constexpr int minLeft = 220;
	const int maxLeft = std::max(minLeft, totalW - 300);

	int leftW = static_cast<int>(std::round(static_cast<double>(totalW) * m_leftRatio));
	leftW = std::clamp(leftW, minLeft, maxLeft);

	const int rightW = std::max(0, totalW - splitterW - leftW);

	if (m_treeWrap) m_treeWrap->setPreferredWidth(leftW);
	if (m_detailWrap) m_detailWrap->setPreferredWidth(rightW);
}
