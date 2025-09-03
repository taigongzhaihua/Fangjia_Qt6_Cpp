#include "DatabaseBootstrapper.h"
#include "SqliteDatabase.h"
#include "utils/DatabasePopulator.h"
#include <QSqlDatabase>
#include <qdebug.h>

namespace Data {
void DatabaseBootstrapper::initialize()
{
    auto db = SqliteDatabase::openDefault();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "DatabaseBootstrapper: failed to open default DB";
    } else {
        qInfo() << "DatabaseBootstrapper: DB ready at" << db.databaseName();
        
        // Populate with sample data if needed
        if (!data::utils::DatabasePopulator::populateSampleData(db)) {
            qWarning() << "DatabaseBootstrapper: failed to populate sample data";
        }
    }
}
}