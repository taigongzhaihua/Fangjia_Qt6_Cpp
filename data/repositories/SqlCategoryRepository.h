#pragma once
#include "ICategoryRepository.h"
#include <QSqlDatabase>

class SqlCategoryRepository final : public ICategoryRepository {
public:
	explicit SqlCategoryRepository(QSqlDatabase db = QSqlDatabase::database("app")) : m_db(db) {}
	QVector<Category> listAll() override;
	std::optional<Category> getById(int id) override;
private:
	QSqlDatabase m_db;
};