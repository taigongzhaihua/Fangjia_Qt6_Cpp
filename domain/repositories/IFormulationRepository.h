#pragma once
#include "domain/entities/Formulation.h"
#include <qvector.h>
#include <optional>

struct IFormulationRepository {
    virtual ~IFormulationRepository() = default;
    virtual QVector<Formulation> listAll() = 0;
    virtual std::optional<Formulation> getById(int id) = 0;
    virtual QVector<FormulationComposition> compositions(int formulationId) = 0;
    virtual QVector<FormulationImage> images(int formulationId) = 0;
};