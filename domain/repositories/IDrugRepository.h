#pragma once
#include "domain/entities/Drug.h"
#include <qvector.h>
#include <optional>

struct IDrugRepository {
    virtual ~IDrugRepository() = default;
    virtual QVector<Drug> listAll() = 0;
    virtual std::optional<Drug> getById(int id) = 0;
    virtual QVector<Drug> findByCategoryText(const QString& categoryText) = 0; // matches Drug.Category TEXT
    virtual QVector<DrugImage> imagesForDrug(int drugId) = 0;
};