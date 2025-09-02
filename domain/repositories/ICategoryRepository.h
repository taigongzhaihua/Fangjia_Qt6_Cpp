#pragma once
#include "domain/entities/Category.h"
#include <qvector.h>
#include <optional>

struct ICategoryRepository {
    virtual ~ICategoryRepository() = default;
    virtual QVector<Category> listAll() = 0;
    virtual std::optional<Category> getById(int id) = 0;
};