#include "SqlDrugRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <qdebug.h>

namespace {
    Drug drugFromQuery(const QSqlQuery& q) {
        Drug drug;
        drug.Id = q.value("Id").toInt();
        drug.Name = q.value("Name").toString();
        drug.EnglishName = q.value("EnglishName").toString();
        drug.LatinName = q.value("LatinName").toString();
        drug.Category = q.value("Category").toString();
        drug.Origin = q.value("Origin").toString();
        drug.Properties = q.value("Properties").toString();
        drug.Quality = q.value("Quality").toString();
        drug.Taste = q.value("Taste").toString();
        drug.Meridian = q.value("Meridian").toString();
        drug.Effect = q.value("Effect").toString();
        drug.Notes = q.value("Notes").toString();
        drug.Processed = q.value("Processed").toString();
        drug.Source = q.value("Source").toString();
        return drug;
    }
}

QVector<Drug> SqlDrugRepository::listAll()
{
    QVector<Drug> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    if (!q.exec("SELECT Id, Name, EnglishName, LatinName, Category, Origin, Properties, Quality, Taste, Meridian, Effect, Notes, Processed, Source FROM Drug ORDER BY Id")) {
        qWarning() << "SqlDrugRepository::listAll failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        results.append(drugFromQuery(q));
    }
    return results;
}

std::optional<Drug> SqlDrugRepository::getById(int id)
{
    if (!m_db.isValid() || !m_db.isOpen()) return std::nullopt;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, Name, EnglishName, LatinName, Category, Origin, Properties, Quality, Taste, Meridian, Effect, Notes, Processed, Source FROM Drug WHERE Id = ?");
    q.addBindValue(id);
    
    if (!q.exec()) {
        qWarning() << "SqlDrugRepository::getById failed:" << q.lastError();
        return std::nullopt;
    }

    if (q.next()) {
        return drugFromQuery(q);
    }
    return std::nullopt;
}

QVector<Drug> SqlDrugRepository::findByCategoryText(const QString& categoryText)
{
    QVector<Drug> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, Name, EnglishName, LatinName, Category, Origin, Properties, Quality, Taste, Meridian, Effect, Notes, Processed, Source FROM Drug WHERE Category = ? ORDER BY Id");
    q.addBindValue(categoryText);
    
    if (!q.exec()) {
        qWarning() << "SqlDrugRepository::findByCategoryText failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        results.append(drugFromQuery(q));
    }
    return results;
}

QVector<DrugImage> SqlDrugRepository::imagesForDrug(int drugId)
{
    QVector<DrugImage> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, DrugId, Image FROM DrugImage WHERE DrugId = ? ORDER BY Id");
    q.addBindValue(drugId);
    
    if (!q.exec()) {
        qWarning() << "SqlDrugRepository::imagesForDrug failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        DrugImage img;
        img.Id = q.value(0).toInt();
        img.DrugId = q.value(1).toInt();
        img.Image = q.value(2).toByteArray();
        results.append(img);
    }
    return results;
}