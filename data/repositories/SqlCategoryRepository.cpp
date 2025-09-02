#include "SqlCategoryRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <qdebug.h>

QVector<Category> SqlCategoryRepository::listAll()
{
    QVector<Category> results;
    if (!m_db.isValid() || !m_db.isOpen()) return results;

    QSqlQuery q(m_db);
    if (!q.exec("SELECT Id, FirstCategory, SecondCategory FROM Category ORDER BY Id")) {
        qWarning() << "SqlCategoryRepository::listAll failed:" << q.lastError();
        return results;
    }

    while (q.next()) {
        Category cat;
        cat.Id = q.value(0).toInt();
        cat.FirstCategory = q.value(1).toString();
        cat.SecondCategory = q.value(2).toString();
        results.append(cat);
    }
    return results;
}

std::optional<Category> SqlCategoryRepository::getById(int id)
{
    if (!m_db.isValid() || !m_db.isOpen()) return std::nullopt;

    QSqlQuery q(m_db);
    q.prepare("SELECT Id, FirstCategory, SecondCategory FROM Category WHERE Id = ?");
    q.addBindValue(id);
    
    if (!q.exec()) {
        qWarning() << "SqlCategoryRepository::getById failed:" << q.lastError();
        return std::nullopt;
    }

    if (q.next()) {
        Category cat;
        cat.Id = q.value(0).toInt();
        cat.FirstCategory = q.value(1).toString();
        cat.SecondCategory = q.value(2).toString();
        return cat;
    }
    return std::nullopt;
}