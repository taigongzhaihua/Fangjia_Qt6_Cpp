#pragma once
#include "IDrugRepository.h"
#include <QSqlDatabase>

class SqlDrugRepository final : public IDrugRepository {
public:
	explicit SqlDrugRepository(QSqlDatabase db = QSqlDatabase::database("app")) : m_db(db) {}
	QVector<Drug> listAll() override;
	std::optional<Drug> getById(int id) override;
	QVector<Drug> findByCategoryText(const QString& categoryText) override;
	QVector<DrugImage> imagesForDrug(int drugId) override;
private:
	QSqlDatabase m_db;
};