#pragma once
#include <qbytearray.h>
#include <qstring.h>

struct Drug {
    int Id{0};
    QString Name;
    QString EnglishName;
    QString LatinName;
    QString Category; // Note: schema stores category as TEXT
    QString Origin;
    QString Properties;
    QString Quality;
    QString Taste;
    QString Meridian;
    QString Effect;
    QString Notes;
    QString Processed;
    QString Source;
};

struct DrugImage {
    int Id{0};
    int DrugId{0};
    QByteArray Image; // raw blob
};