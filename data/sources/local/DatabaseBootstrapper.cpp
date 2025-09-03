#include "DatabaseBootstrapper.h"
#include "SqliteDatabase.h"
#include "DatabasePopulator.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <qdebug.h>

namespace Data {
	void DatabaseBootstrapper::initialize()
	{
		qDebug() << "DatabaseBootstrapper: Starting database initialization...";
		auto db = SqliteDatabase::openDefault();
		if (!db.isValid() || !db.isOpen()) {
			qWarning() << "DatabaseBootstrapper: failed to open default DB";
			qWarning() << "DatabaseBootstrapper: DB error:" << db.lastError().text();
		}
		else {
			qInfo() << "DatabaseBootstrapper: DB ready at" << db.databaseName();

			// Populate with sample data if needed
			qDebug() << "DatabaseBootstrapper: Attempting to populate sample data...";
			if (!data::utils::DatabasePopulator::populateSampleData(db)) {
				qWarning() << "DatabaseBootstrapper: failed to populate sample data";
			}
			else {
				qInfo() << "DatabaseBootstrapper: Sample data population completed successfully";
			}
		}
		qDebug() << "DatabaseBootstrapper: Initialization completed.";
	}
}