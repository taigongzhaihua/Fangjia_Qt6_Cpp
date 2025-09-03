#include "SqliteDatabase.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDir>
#include <qdebug.h>

namespace {
    constexpr auto kConnName = "app";
    constexpr auto kFileName = "fangjia.db";

    static const char* kSchemaSql = R"SQL(
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Category (
    Id             INTEGER PRIMARY KEY AUTOINCREMENT,
    FirstCategory  TEXT,
    SecondCategory TEXT
);

CREATE TABLE IF NOT EXISTS Drug (
    Id          INTEGER PRIMARY KEY AUTOINCREMENT,
    Name        TEXT,
    EnglishName TEXT,
    LatinName   TEXT,
    Category    TEXT,
    Origin      TEXT,
    Properties  TEXT,
    Quality     TEXT,
    Taste       TEXT,
    Meridian    TEXT,
    Effect      TEXT,
    Notes       TEXT,
    Processed   TEXT,
    Source      TEXT
);

CREATE TABLE IF NOT EXISTS DrugImage (
    Id     INTEGER PRIMARY KEY AUTOINCREMENT,
    DrugId INTEGER,
    Image  BLOB,
    FOREIGN KEY (DrugId) REFERENCES Drug (Id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS Formulation (
    Id          INTEGER PRIMARY KEY AUTOINCREMENT,
    Name        TEXT,
    CategoryId  INTEGER,
    Usage       TEXT,
    Effect      TEXT,
    Indication  TEXT,
    Disease     TEXT,
    Application TEXT,
    Supplement  TEXT,
    Song        TEXT,
    Notes       TEXT,
    Source      TEXT,
    FOREIGN KEY (CategoryId) REFERENCES Category (Id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS FormulationComposition (
    Id            INTEGER PRIMARY KEY AUTOINCREMENT,
    FormulationId INTEGER,
    DrugID        INTEGER REFERENCES Drug (Id),
    DrugName      TEXT,
    Effect        TEXT,
    Position      TEXT,
    Notes         TEXT,
    FOREIGN KEY (FormulationId) REFERENCES Formulation (Id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS FormulationImage (
    Id            INTEGER PRIMARY KEY AUTOINCREMENT,
    FormulationId INTEGER,
    Image         BLOB,
    FOREIGN KEY (FormulationId) REFERENCES Formulation (Id) ON DELETE CASCADE
);
)SQL";
}

QString SqliteDatabase::defaultDbPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QDir::separator() + kFileName;
}

QSqlDatabase SqliteDatabase::openDefault()
{
    QSqlDatabase db;
    if (QSqlDatabase::contains(kConnName)) {
        db = QSqlDatabase::database(kConnName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", kConnName);
        db.setDatabaseName(defaultDbPath());
    }

    if (!db.isOpen() && !db.open()) {
        qWarning() << "Failed to open SQLite DB:" << db.lastError();
        return QSqlDatabase();
    }

    // Enforce foreign keys for the connection
    QSqlQuery pragma(db);
    pragma.exec("PRAGMA foreign_keys = ON;");

    if (!ensureSchema(db)) {
        qWarning() << "Failed to ensure schema";
    }

    return db;
}

bool SqliteDatabase::ensureSchema(QSqlDatabase& db)
{
    if (!db.isValid() || !db.isOpen()) return false;

    // Split schema into individual statements and execute them separately
    // This avoids "Unable to execute multiple statements at a time" error
    QString schemaStr = QString::fromUtf8(kSchemaSql);
    QStringList statements = schemaStr.split(';', Qt::SkipEmptyParts);
    
    QSqlQuery q(db);
    for (const QString& statement : statements) {
        QString trimmedStatement = statement.trimmed();
        if (trimmedStatement.isEmpty()) continue;
        
        if (!q.exec(trimmedStatement)) {
            qWarning() << "Schema exec error:" << q.lastError() << "for statement:" << trimmedStatement.left(50) + "...";
            return false;
        }
    }
    return true;
}