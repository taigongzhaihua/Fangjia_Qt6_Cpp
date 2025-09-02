#include "FormulaRepository.h"
#include <entities/Formula.h>
#include <exception>
#include <memory>
#include <qlogging.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qstandardpaths.h>
#include <qstring.h>
#include <qvariant.h>
#include <string>
#include <utility>
#include <vector>

namespace data::repositories
{

	FormulaRepository::FormulaRepository(const QString& dbPath)
		: m_isInitialized(false)
	{
		if (dbPath.isEmpty()) {
			// Use conventional location: data/sources/local/formulas.db
			const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
			m_dbPath = appDataPath + "/formulas.db";
		}
		else {
			m_dbPath = dbPath;
		}

		m_isInitialized = initializeDatabase();
	}

	FormulaRepository::~FormulaRepository()
	{
		if (m_database) {
			m_database->close();
		}
	}

	bool FormulaRepository::isAvailable() const
	{
		return m_isInitialized && m_database && m_database->isOpen();
	}

	bool FormulaRepository::initializeDatabase()
	{
		try {
			m_database = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", "formulas"));
			m_database->setDatabaseName(m_dbPath);

			if (!m_database->open()) {
				qWarning() << "[FormulaRepository] Failed to open database:" << m_database->lastError().text() << "\n";
				return false;
			}

			// Create tables if they don't exist
			QSqlQuery query(*m_database);

			// Formula nodes table for hierarchical structure
			const QString createNodesTable = R"(
            CREATE TABLE IF NOT EXISTS formula_nodes (
                id TEXT PRIMARY KEY,
                label TEXT NOT NULL,
                level INTEGER NOT NULL,
                parent_id TEXT,
                sort_order INTEGER DEFAULT 0
            )
        )";

			if (!query.exec(createNodesTable)) {
				qWarning() << "[FormulaRepository] Failed to create nodes table:" << query.lastError().text() << "\n";
				return false;
			}

			// Formula details table
			const QString createDetailsTable = R"(
            CREATE TABLE IF NOT EXISTS formula_details (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                source TEXT,
                composition TEXT,
                usage TEXT,
                function TEXT,
                indication TEXT,
                note TEXT
            )
        )";

			if (!query.exec(createDetailsTable)) {
				qWarning() << "[FormulaRepository] Failed to create details table:" << query.lastError().text() << "\n";
				return false;
			}

			// Check if we need to populate sample data
			query.prepare("SELECT COUNT(*) FROM formula_nodes");
			if (query.exec() && query.next() && query.value(0).toInt() == 0) {
				createSampleData();
			}

			return true;

		}
		catch (const std::exception& e) {
			qWarning() << "[FormulaRepository] Database initialization failed:" << e.what() << "\n";
			return false;
		}
	}

	void FormulaRepository::createSampleData() const
	{
		if (!m_database || !m_database->isOpen()) {
			return;
		}

		QSqlQuery query(*m_database);

		// Start transaction for better performance
		m_database->transaction();

		try {
			// Insert category nodes
			query.prepare("INSERT INTO formula_nodes (id, label, level, parent_id, sort_order) VALUES (?, ?, ?, ?, ?)");

			// 解表剂
			query.addBindValue("jiebiao");
			query.addBindValue("解表剂");
			query.addBindValue(0);
			query.addBindValue(QVariant());
			query.addBindValue(1);
			query.exec();

			// 辛温解表
			query.addBindValue("xinwen");
			query.addBindValue("辛温解表");
			query.addBindValue(1);
			query.addBindValue("jiebiao");
			query.addBindValue(1);
			query.exec();

			// 辛凉解表  
			query.addBindValue("xinliang");
			query.addBindValue("辛凉解表");
			query.addBindValue(1);
			query.addBindValue("jiebiao");
			query.addBindValue(2);
			query.exec();

			// Formula nodes
			query.addBindValue("mahuangtang");
			query.addBindValue("麻黄汤");
			query.addBindValue(2);
			query.addBindValue("xinwen");
			query.addBindValue(1);
			query.exec();

			query.addBindValue("guizhitang");
			query.addBindValue("桂枝汤");
			query.addBindValue(2);
			query.addBindValue("xinwen");
			query.addBindValue(2);
			query.exec();

			query.addBindValue("sangjuyin");
			query.addBindValue("桑菊饮");
			query.addBindValue(2);
			query.addBindValue("xinliang");
			query.addBindValue(1);
			query.exec();

			// Insert formula details
			query.prepare(R"(
            INSERT INTO formula_details (id, name, source, composition, usage, function, indication, note) 
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        )");

			// 麻黄汤
			query.addBindValue("mahuangtang");
			query.addBindValue("麻黄汤");
			query.addBindValue("《伤寒论》");
			query.addBindValue("麻黄9g、桂枝6g、杏仁9g、甘草3g");
			query.addBindValue("水煎服，温覆取微汗");
			query.addBindValue("发汗解表，宣肺平喘");
			query.addBindValue("外感风寒表实证。恶寒发热，头身疼痛，无汗而喘，舌苔薄白，脉浮紧");
			query.addBindValue("本方为辛温发汗之峻剂，故《伤寒论》强调'温服八合，覆取微似汗'");
			query.exec();

			// 桂枝汤
			query.addBindValue("guizhitang");
			query.addBindValue("桂枝汤");
			query.addBindValue("《伤寒论》");
			query.addBindValue("桂枝9g、芍药9g、生姜9g、大枣12枚、甘草6g");
			query.addBindValue("温服，啜粥，温覆取微汗");
			query.addBindValue("解肌发表，调和营卫");
			query.addBindValue("外感风寒表虚证。恶风发热，汗出头痛，鼻鸣干呕，舌苔薄白，脉浮缓");
			query.addBindValue("群方之冠，调和营卫之总方");
			query.exec();

			// 桑菊饮
			query.addBindValue("sangjuyin");
			query.addBindValue("桑菊饮");
			query.addBindValue("《温病条辨》");
			query.addBindValue("桑叶7.5g、菊花3g、杏仁6g、连翘5g、薄荷2.5g、苦桔梗6g、甘草2.5g、芦根6g");
			query.addBindValue("水煎服");
			query.addBindValue("疏风清热，宣肺止咳");
			query.addBindValue("风温初起，但咳，身热不甚，口微渴，脉浮数");
			query.addBindValue("本方为辛凉轻剂，治疗风温初起，邪在肺卫");
			query.exec();

			m_database->commit();
			qDebug() << "[FormulaRepository] Sample data created successfully" << "\n";

		}
		catch (const std::exception& e) {
			m_database->rollback();
			qWarning() << "[FormulaRepository] Failed to create sample data:" << e.what() << "\n";
		}
	}

	std::vector<domain::entities::FormulaNode> FormulaRepository::loadFormulaTree()
	{
		std::vector<domain::entities::FormulaNode> nodes;

		if (!isAvailable()) {
			return nodes;
		}

		QSqlQuery query(*m_database);
		query.prepare("SELECT id, label, level, parent_id FROM formula_nodes ORDER BY level, sort_order, label");

		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load formula tree:" << query.lastError().text() << "\n";
			return nodes;
		}

		while (query.next()) {
			domain::entities::FormulaNode node;
			node.id = qStringToStdString(query.value("id").toString());
			node.label = qStringToStdString(query.value("label").toString());
			node.level = query.value("level").toInt();
			node.parentId = qStringToStdString(query.value("parent_id").toString());
			node.hasDetail = false;  // Will be populated separately for formulas

			// For formula nodes (level 2), load the detail
			if (node.level == 2) {
				node.detail = loadFormulaDetail(node.id);
				node.hasDetail = node.detail.hasDetail;
			}

			nodes.push_back(std::move(node));
		}

		return nodes;
	}

	domain::entities::FormulaDetail FormulaRepository::loadFormulaDetail(const std::string& formulaId)
	{
		domain::entities::FormulaDetail detail;
		detail.hasDetail = false;

		if (!isAvailable()) {
			return detail;
		}

		QSqlQuery query(*m_database);
		query.prepare("SELECT * FROM formula_details WHERE id = ?");
		query.addBindValue(stdStringToQString(formulaId));

		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load formula detail:" << query.lastError().text();
			return detail;
		}

		if (query.next()) {
			detail.id = qStringToStdString(query.value("id").toString());
			detail.name = qStringToStdString(query.value("name").toString());
			detail.source = qStringToStdString(query.value("source").toString());
			detail.composition = qStringToStdString(query.value("composition").toString());
			detail.usage = qStringToStdString(query.value("usage").toString());
			detail.function = qStringToStdString(query.value("function").toString());
			detail.indication = qStringToStdString(query.value("indication").toString());
			detail.note = qStringToStdString(query.value("note").toString());
			detail.hasDetail = true;
		}

		return detail;
	}

	std::string FormulaRepository::qStringToStdString(const QString& qstr) const
	{
		return qstr.toStdString();
	}

	QString FormulaRepository::stdStringToQString(const std::string& str) const
	{
		return QString::fromStdString(str);
	}

} // namespace data::repositories