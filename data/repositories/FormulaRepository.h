#pragma once
#include "repositories/IFormulaRepository.h"
#include <memory>
#include <QString>

class QSqlDatabase;

namespace data::repositories
{

	/// SQLite-based implementation of formula repository
	/// Uses Qt SQL for database operations
	class FormulaRepository : public domain::repositories::IFormulaRepository {
	public:
		/// Constructor
		/// Parameters: dbPath - Path to SQLite database file, empty for default location
		explicit FormulaRepository(const QString& dbPath = QString());
		~FormulaRepository() override;

		// IFormulaRepository interface
		std::vector<std::string> fetchFirstCategories() override;
		std::vector<domain::entities::FormulaNode> loadFormulaTree() override;
		domain::entities::FormulaDetail loadFormulaDetail(const std::string& formulaId) override;
		bool isAvailable() const override;

	private:
		/// Convert QString to std::string safely
		std::string qStringToStdString(const QString& qstr) const;

		/// Convert std::string to QString safely
		QString stdStringToQString(const std::string& str) const;

	private:
		std::unique_ptr<QSqlDatabase> m_database;
		bool m_isInitialized;
	};

} // namespace data::repositories