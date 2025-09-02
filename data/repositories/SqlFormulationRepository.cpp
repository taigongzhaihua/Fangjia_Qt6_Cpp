#include "SqlFormulationRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <qdebug.h>

namespace {
    Formulation formulationFromQuery(const QSqlQuery& q) {
        Formulation formulation;
        formulation.Id = q.value("Id").toInt();
        formulation.Name = q.value("Name").toString();
        formulation.CategoryId = q.value("CategoryId").toInt();
        formulation.Usage = q.value("Usage").toString();
        formulation.Effect = q.value("Effect").toString();
        formulation.Indication = q.value("Indication").toString();
        formulation.Disease = q.value("Disease").toString();
        formulation.Application = q.value("Application").toString();
        formulation.Supplement = q.value("Supplement").toString();
        formulation.Song = q.value("Song").toString();
        formulation.Notes = q.value("Notes").toString();
        formulation.Source = q.value("Source").toString();
        return formulation;
    }
}

QVector<Formulation> SqlFormulationRepository::listAll()
{
    QVector<Formulation> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    if (!q.exec("SELECT Id, Name, CategoryId, Usage, Effect, Indication, Disease, Application, Supplement, Song, Notes, Source FROM Formulation ORDER BY Id")) {
        qWarning() << "SqlFormulationRepository::listAll failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        results.append(formulationFromQuery(q));
    }
    return results;
}

std::optional<Formulation> SqlFormulationRepository::getById(int id)
{
    if (!m_db.isValid() || !m_db.isOpen()) return std::nullopt;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, Name, CategoryId, Usage, Effect, Indication, Disease, Application, Supplement, Song, Notes, Source FROM Formulation WHERE Id = ?");
    q.addBindValue(id);
    
    if (!q.exec()) {
        qWarning() << "SqlFormulationRepository::getById failed:" << q.lastError();
        return std::nullopt;
    }

    if (q.next()) {
        return formulationFromQuery(q);
    }
    return std::nullopt;
}

QVector<FormulationComposition> SqlFormulationRepository::compositions(int formulationId)
{
    QVector<FormulationComposition> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, FormulationId, DrugID, DrugName, Effect, Position, Notes FROM FormulationComposition WHERE FormulationId = ? ORDER BY Id");
    q.addBindValue(formulationId);
    
    if (!q.exec()) {
        qWarning() << "SqlFormulationRepository::compositions failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        FormulationComposition comp;
        comp.Id = q.value(0).toInt();
        comp.FormulationId = q.value(1).toInt();
        comp.DrugID = q.value(2).toInt();
        comp.DrugName = q.value(3).toString();
        comp.Effect = q.value(4).toString();
        comp.Position = q.value(5).toString();
        comp.Notes = q.value(6).toString();
        results.append(comp);
    }
    return results;
}

QVector<FormulationImage> SqlFormulationRepository::images(int formulationId)
{
    QVector<FormulationImage> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, FormulationId, Image FROM FormulationImage WHERE FormulationId = ? ORDER BY Id");
    q.addBindValue(formulationId);
    
    if (!q.exec()) {
        qWarning() << "SqlFormulationRepository::images failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        FormulationImage img;
        img.Id = q.value(0).toInt();
        img.FormulationId = q.value(1).toInt();
        img.Image = q.value(2).toByteArray();
        results.append(img);
    }
    return results;
}