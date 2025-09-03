#pragma once
#include "repositories/IFormulationRepository.h"
#include <QSqlDatabase>

class SqlFormulationRepository final : public IFormulationRepository {
public:
    explicit SqlFormulationRepository(QSqlDatabase db = QSqlDatabase::database("app")) : m_db(db) {}
    QVector<Formulation> listAll() override;
    std::optional<Formulation> getById(int id) override;
    QVector<FormulationComposition> compositions(int formulationId) override;
    QVector<FormulationImage> images(int formulationId) override;
private:
    QSqlDatabase m_db;
};