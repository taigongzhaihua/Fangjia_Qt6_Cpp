#include "FormulaViewModel.h"

FormulaViewModel::FormulaViewModel(QObject* parent)
    : QObject(parent)
{
}

FormulaViewModel::~FormulaViewModel()
{
    clearData();
}

void FormulaViewModel::clearData()
{
    // 清理详情内存
    for (auto& node : m_nodes) {
        delete node.detail;
        node.detail = nullptr;
    }
    m_nodes.clear();
    m_selectedIdx = -1;
    emit dataChanged();
}

void FormulaViewModel::loadSampleData()
{
    clearData();
    
    // 解表剂分类
    addCategory("jiebiao", "解表剂");
    int jiebiaoIdx = m_nodes.size() - 1;
    
    // 辛温解表
    addSubCategory("xinwen", "辛温解表", jiebiaoIdx);
    int xinwenIdx = m_nodes.size() - 1;
    
    // 添加方剂
    auto* mahuangDetail = new FormulaDetail{
        .name = "麻黄汤",
        .source = "《伤寒论》",
        .composition = "麻黄9g、桂枝6g、杏仁9g、甘草3g",
        .usage = "水煎服，温覆取微汗",
        .function = "发汗解表，宣肺平喘",
        .indication = "外感风寒表实证。恶寒发热，头身疼痛，无汗而喘，舌苔薄白，脉浮紧",
        .note = "本方为辛温发汗之峻剂，故《伤寒论》强调'温服八合，覆取微似汗'"
    };
    addFormula("mahuangtang", "麻黄汤", xinwenIdx, mahuangDetail);
    
    auto* guizhiDetail = new FormulaDetail{
        .name = "桂枝汤",
        .source = "《伤寒论》",
        .composition = "桂枝9g、芍药9g、生姜9g、大枣12枚、甘草6g",
        .usage = "温服，啜粥，温覆取微汗",
        .function = "解肌发表，调和营卫",
        .indication = "外感风寒表虚证。恶风发热，汗出头痛，鼻鸣干呕，舌苔薄白，脉浮缓",
        .note = "群方之冠，调和营卫之总方"
    };
    addFormula("guizhitang", "桂枝汤", xinwenIdx, guizhiDetail);
    
    // 辛凉解表
    addSubCategory("xinliang", "辛凉解表", jiebiaoIdx);
    int xinliangIdx = m_nodes.size() - 1;
    
    auto* sangjuDetail = new FormulaDetail{
        .name = "桑菊饮",
        .source = "《温病条辨》",
        .composition = "桑叶7.5g、菊花3g、杏仁6g、连翘5g、薄荷2.5g、苦桔梗6g、甘草2.5g、芦根6g",
        .usage = "水煎服",
        .function = "疏风清热，宣肺止咳",
        .indication = "风温初起，但咳，身热不甚，口微渴，脉浮数",
        .note = "本方为辛凉轻剂，治疗风温初起，邪在肺卫"
    };
    addFormula("sangjuyin", "桑菊饮", xinliangIdx, sangjuDetail);
    
    // 泻下剂分类
    addCategory("xiexia", "泻下剂");
    int xiexiaIdx = m_nodes.size() - 1;
    
    // 寒下
    addSubCategory("hanxia", "寒下", xiexiaIdx);
    int hanxiaIdx = m_nodes.size() - 1;
    
    auto* dachengqiDetail = new FormulaDetail{
        .name = "大承气汤",
        .source = "《伤寒论》",
        .composition = "大黄12g、厚朴15g、枳实12g、芒硝9g",
        .usage = "水煎服，以利为度",
        .function = "峻下热结",
        .indication = "阳明腑实证。大便不通，频转矢气，脘腹痞满，腹痛拒按，按之硬，甚或潮热谵语，手足濈然汗出，舌苔黄燥起刺，或焦黑燥裂，脉沉实",
        .note = "本方为寒下峻剂，须有腑实证候方可使用"
    };
    addFormula("dachengqi", "大承气汤", hanxiaIdx, dachengqiDetail);
    
    emit dataChanged();
}

void FormulaViewModel::addCategory(const QString& id, const QString& label)
{
    TreeNode node{
        .id = id,
        .label = label,
        .level = 0,
        .expanded = false,
        .parentIndex = -1,
        .detail = nullptr
    };
    m_nodes.append(node);
}

void FormulaViewModel::addSubCategory(const QString& id, const QString& label, int parentIdx)
{
    TreeNode node{
        .id = id,
        .label = label,
        .level = 1,
        .expanded = false,
        .parentIndex = parentIdx,
        .detail = nullptr
    };
    m_nodes.append(node);
}

void FormulaViewModel::addFormula(const QString& id, const QString& label, int parentIdx, FormulaDetail* detail)
{
    TreeNode node{
        .id = id,
        .label = label,
        .level = 2,
        .expanded = false,
        .parentIndex = parentIdx,
        .detail = detail
    };
    m_nodes.append(node);
}

QVector<int> FormulaViewModel::childIndices(int parentIdx) const
{
    QVector<int> children;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].parentIndex == parentIdx) {
            children.append(i);
        }
    }
    return children;
}

void FormulaViewModel::setSelectedIndex(int idx)
{
    if (idx < -1 || idx >= m_nodes.size()) return;
    if (m_selectedIdx == idx) return;
    
    m_selectedIdx = idx;
    emit selectedChanged(m_selectedIdx);
}

void FormulaViewModel::toggleExpanded(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    setExpanded(idx, !m_nodes[idx].expanded);
}

void FormulaViewModel::setExpanded(int idx, bool expanded)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    if (m_nodes[idx].expanded == expanded) return;
    
    m_nodes[idx].expanded = expanded;
    emit nodeExpandChanged(idx, expanded);
}

const FormulaViewModel::FormulaDetail* FormulaViewModel::selectedFormula() const
{
    if (m_selectedIdx >= 0 && m_selectedIdx < m_nodes.size()) {
        return m_nodes[m_selectedIdx].detail;
    }
    return nullptr;
}