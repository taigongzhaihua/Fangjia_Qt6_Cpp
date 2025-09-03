#include "FormulaRepository.h"
#include "SqliteDatabase.h"
#include "entities/Formula.h"
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
		// Use the shared database connection from DatabaseBootstrapper
		// rather than creating a separate connection
		// Note: dbPath parameter is ignored for compatibility with existing code
		QSqlDatabase db = SqliteDatabase::openDefault();
		if (db.isValid() && db.isOpen()) {
			m_database = std::make_unique<QSqlDatabase>(db);
			m_isInitialized = true;
			qDebug() << "[FormulaRepository] Using shared database connection:" << db.databaseName();
		} else {
			qWarning() << "[FormulaRepository] Failed to get shared database connection";
			m_isInitialized = false;
		}
	}

	FormulaRepository::~FormulaRepository()
	{
		// Don't close the shared database connection in destructor
		// The connection is managed by SqliteDatabase::openDefault()
	}

	bool FormulaRepository::isAvailable() const
	{
		return m_isInitialized && m_database && m_database->isOpen();
	}

	std::vector<std::string> FormulaRepository::fetchFirstCategories()
	{
		std::vector<std::string> categories;

		if (!isAvailable()) {
			qWarning() << "[FormulaRepository] Repository not available for fetchFirstCategories";
			return categories;
		}

		QSqlQuery query(*m_database);
		const QString sql = "SELECT DISTINCT FirstCategory FROM Category WHERE FirstCategory IS NOT NULL AND FirstCategory != '' ORDER BY FirstCategory";
		
		if (!query.prepare(sql)) {
			qWarning() << "[FormulaRepository] Failed to prepare first categories query:" << query.lastError().text();
			return categories;
		}

		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load first categories:" << query.lastError().text();
			return categories;
		}

		while (query.next()) {
			QString category = query.value("FirstCategory").toString();
			if (!category.isEmpty()) {
				categories.push_back(qStringToStdString(category));
			}
		}

		qDebug() << "[FormulaRepository] Loaded" << categories.size() << "first categories";
		return categories;
	}

	std::vector<domain::entities::FormulaNode> FormulaRepository::loadFormulaTree()
	{
		std::vector<domain::entities::FormulaNode> nodes;

		if (!isAvailable()) {
			qWarning() << "[FormulaRepository] Repository not available for loadFormulaTree";
			return nodes;
		}

		QSqlQuery query(*m_database);

		// First, load first-level categories
		const QString firstCategorySql = "SELECT DISTINCT FirstCategory FROM Category WHERE FirstCategory IS NOT NULL AND FirstCategory != '' ORDER BY FirstCategory";
		if (!query.prepare(firstCategorySql)) {
			qWarning() << "[FormulaRepository] Failed to prepare first categories query:" << query.lastError().text();
			return nodes;
		}
		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load first categories:" << query.lastError().text();
			return nodes;
		}

		while (query.next()) {
			domain::entities::FormulaNode categoryNode;
			QString firstCategory = query.value("FirstCategory").toString();
			categoryNode.id = qStringToStdString(firstCategory);
			categoryNode.label = qStringToStdString(firstCategory);
			categoryNode.level = 0;
			categoryNode.parentId = "";
			categoryNode.hasDetail = false;
			nodes.push_back(std::move(categoryNode));
		}

		// Then, load second-level categories
		query.prepare("SELECT DISTINCT FirstCategory, SecondCategory FROM Category WHERE SecondCategory IS NOT NULL AND SecondCategory != '' ORDER BY FirstCategory, SecondCategory");
		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load second categories:" << query.lastError().text();
			return nodes;
		}

		while (query.next()) {
			domain::entities::FormulaNode subCategoryNode;
			QString firstCategory = query.value("FirstCategory").toString();
			QString secondCategory = query.value("SecondCategory").toString();
			subCategoryNode.id = qStringToStdString(firstCategory + "_" + secondCategory);
			subCategoryNode.label = qStringToStdString(secondCategory);
			subCategoryNode.level = 1;
			subCategoryNode.parentId = qStringToStdString(firstCategory);
			subCategoryNode.hasDetail = false;
			nodes.push_back(std::move(subCategoryNode));
		}

		// Finally, load formulations
		query.prepare(R"(
			SELECT f.Id, f.Name, c.FirstCategory, c.SecondCategory 
			FROM Formulation f 
			JOIN Category c ON f.CategoryId = c.Id 
			ORDER BY c.FirstCategory, c.SecondCategory, f.Name
		)");
		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load formulations:" << query.lastError().text();
			return nodes;
		}

		while (query.next()) {
			domain::entities::FormulaNode formulaNode;
			int formulationId = query.value("Id").toInt();
			QString name = query.value("Name").toString();
			QString firstCategory = query.value("FirstCategory").toString();
			QString secondCategory = query.value("SecondCategory").toString();

			formulaNode.id = qStringToStdString(QString::number(formulationId));
			formulaNode.label = qStringToStdString(name);
			formulaNode.level = 2;
			formulaNode.parentId = qStringToStdString(firstCategory + "_" + secondCategory);
			
			// Load the detail for this formula
			formulaNode.detail = loadFormulaDetail(formulaNode.id);
			formulaNode.hasDetail = formulaNode.detail.hasDetail;
			
			nodes.push_back(std::move(formulaNode));
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
		
		// Load formulation details from the real schema
		query.prepare("SELECT * FROM Formulation WHERE Id = ?");
		query.addBindValue(stdStringToQString(formulaId).toInt());

		if (!query.exec()) {
			qWarning() << "[FormulaRepository] Failed to load formulation detail:" << query.lastError().text();
			return detail;
		}

		if (query.next()) {
			detail.id = formulaId;
			detail.name = qStringToStdString(query.value("Name").toString());
			detail.source = qStringToStdString(query.value("Source").toString());
			detail.usage = qStringToStdString(query.value("Usage").toString());
			detail.function = qStringToStdString(query.value("Effect").toString());
			detail.indication = qStringToStdString(query.value("Indication").toString());
			detail.note = qStringToStdString(query.value("Notes").toString());

			// Build composition from FormulationComposition table
			QSqlQuery compQuery(*m_database);
			compQuery.prepare("SELECT DrugName FROM FormulationComposition WHERE FormulationId = ? ORDER BY Position");
			compQuery.addBindValue(stdStringToQString(formulaId).toInt());
			
			if (compQuery.exec()) {
				QStringList drugs;
				while (compQuery.next()) {
					QString drugName = compQuery.value("DrugName").toString();
					if (!drugName.isEmpty()) {
						drugs.append(drugName);
					}
				}
				detail.composition = qStringToStdString(drugs.join("、"));
			}

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