#pragma once
#include <qsqldatabase.h>
#include <qstring.h>

// Lightweight helper to open/create the app database and ensure schema.
// Usage: QSqlDatabase db = SqliteDatabase::openDefault();
class SqliteDatabase {
public:
    // Opens (and creates if missing) the default database at platform app-data path.
    // Connection name: "app". Returns a valid, open QSqlDatabase handle or an invalid one on failure.
    static QSqlDatabase openDefault();

    // Idempotent: ensure required tables exist and PRAGMA foreign_keys is ON.
    // Returns true on success.
    static bool ensureSchema(QSqlDatabase& db);

    // Returns the full absolute path for the default DB file.
    static QString defaultDbPath();
};