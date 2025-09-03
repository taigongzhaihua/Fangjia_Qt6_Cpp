#include "DatabasePopulator.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace data::utils {

bool DatabasePopulator::populateSampleData(QSqlDatabase& db)
{
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "DatabasePopulator: Invalid database connection";
        return false;
    }

    // Check if data already exists
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM Category");
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "DatabasePopulator: Sample data already exists, skipping";
        return true;
    }

    db.transaction();

    try {
        if (!createSampleCategories(db)) {
            db.rollback();
            return false;
        }

        if (!createSampleFormulations(db)) {
            db.rollback();
            return false;
        }

        if (!createSampleCompositions(db)) {
            db.rollback();
            return false;
        }

        db.commit();
        qDebug() << "DatabasePopulator: Sample data created successfully";
        return true;

    } catch (const std::exception& e) {
        db.rollback();
        qWarning() << "DatabasePopulator: Failed to create sample data:" << e.what();
        return false;
    }
}

bool DatabasePopulator::createSampleCategories(QSqlDatabase& db)
{
    QSqlQuery query(db);
    
    // 解表剂分类
    query.prepare("INSERT INTO Category (FirstCategory, SecondCategory) VALUES (?, ?)");
    query.addBindValue("解表剂");
    query.addBindValue("辛温解表");
    if (!query.exec()) {
        qWarning() << "Failed to insert category:" << query.lastError().text();
        return false;
    }

    query.prepare("INSERT INTO Category (FirstCategory, SecondCategory) VALUES (?, ?)");
    query.addBindValue("解表剂");
    query.addBindValue("辛凉解表");
    if (!query.exec()) {
        qWarning() << "Failed to insert category:" << query.lastError().text();
        return false;
    }

    // 泻下剂分类
    query.prepare("INSERT INTO Category (FirstCategory, SecondCategory) VALUES (?, ?)");
    query.addBindValue("泻下剂");
    query.addBindValue("寒下");
    if (!query.exec()) {
        qWarning() << "Failed to insert category:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabasePopulator::createSampleFormulations(QSqlDatabase& db)
{
    QSqlQuery query(db);
    
    // 麻黄汤 (CategoryId = 1, 解表剂-辛温解表)
    query.prepare(R"(
        INSERT INTO Formulation (Name, CategoryId, Usage, Effect, Indication, Source, Notes) 
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue("麻黄汤");
    query.addBindValue(1);
    query.addBindValue("水煎服，温覆取微汗");
    query.addBindValue("发汗解表，宣肺平喘");
    query.addBindValue("外感风寒表实证。恶寒发热，头身疼痛，无汗而喘，舌苔薄白，脉浮紧");
    query.addBindValue("《伤寒论》");
    query.addBindValue("本方为辛温发汗之峻剂，故《伤寒论》强调'温服八合，覆取微似汗'");
    if (!query.exec()) {
        qWarning() << "Failed to insert formulation:" << query.lastError().text();
        return false;
    }

    // 桂枝汤 (CategoryId = 1, 解表剂-辛温解表)
    query.prepare(R"(
        INSERT INTO Formulation (Name, CategoryId, Usage, Effect, Indication, Source, Notes) 
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue("桂枝汤");
    query.addBindValue(1);
    query.addBindValue("温服，啜粥，温覆取微汗");
    query.addBindValue("解肌发表，调和营卫");
    query.addBindValue("外感风寒表虚证。恶风发热，汗出头痛，鼻鸣干呕，舌苔薄白，脉浮缓");
    query.addBindValue("《伤寒论》");
    query.addBindValue("群方之冠，调和营卫之总方");
    if (!query.exec()) {
        qWarning() << "Failed to insert formulation:" << query.lastError().text();
        return false;
    }

    // 桑菊饮 (CategoryId = 2, 解表剂-辛凉解表)
    query.prepare(R"(
        INSERT INTO Formulation (Name, CategoryId, Usage, Effect, Indication, Source, Notes) 
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue("桑菊饮");
    query.addBindValue(2);
    query.addBindValue("水煎服");
    query.addBindValue("疏风清热，宣肺止咳");
    query.addBindValue("风温初起，但咳，身热不甚，口微渴，脉浮数");
    query.addBindValue("《温病条辨》");
    query.addBindValue("本方为辛凉轻剂，治疗风温初起，邪在肺卫");
    if (!query.exec()) {
        qWarning() << "Failed to insert formulation:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabasePopulator::createSampleCompositions(QSqlDatabase& db)
{
    QSqlQuery query(db);

    // 麻黄汤组成 (FormulationId = 1)
    const QStringList mahuangTangDrugs = {"麻黄9g", "桂枝6g", "杏仁9g", "甘草3g"};
    for (int i = 0; i < mahuangTangDrugs.size(); ++i) {
        query.prepare(R"(
            INSERT INTO FormulationComposition (FormulationId, DrugName, Position) 
            VALUES (?, ?, ?)
        )");
        query.addBindValue(1);
        query.addBindValue(mahuangTangDrugs[i]);
        query.addBindValue(i + 1);
        if (!query.exec()) {
            qWarning() << "Failed to insert composition:" << query.lastError().text();
            return false;
        }
    }

    // 桂枝汤组成 (FormulationId = 2)
    const QStringList guizhiTangDrugs = {"桂枝9g", "芍药9g", "生姜9g", "大枣12枚", "甘草6g"};
    for (int i = 0; i < guizhiTangDrugs.size(); ++i) {
        query.prepare(R"(
            INSERT INTO FormulationComposition (FormulationId, DrugName, Position) 
            VALUES (?, ?, ?)
        )");
        query.addBindValue(2);
        query.addBindValue(guizhiTangDrugs[i]);
        query.addBindValue(i + 1);
        if (!query.exec()) {
            qWarning() << "Failed to insert composition:" << query.lastError().text();
            return false;
        }
    }

    // 桑菊饮组成 (FormulationId = 3)
    const QStringList sangjuYinDrugs = {"桑叶7.5g", "菊花3g", "杏仁6g", "连翘5g", "薄荷2.5g", "苦桔梗6g", "甘草2.5g", "芦根6g"};
    for (int i = 0; i < sangjuYinDrugs.size(); ++i) {
        query.prepare(R"(
            INSERT INTO FormulationComposition (FormulationId, DrugName, Position) 
            VALUES (?, ?, ?)
        )");
        query.addBindValue(3);
        query.addBindValue(sangjuYinDrugs[i]);
        query.addBindValue(i + 1);
        if (!query.exec()) {
            qWarning() << "Failed to insert composition:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

} // namespace data::utils