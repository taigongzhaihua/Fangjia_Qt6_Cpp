#pragma once
#include <qbytearray.h>
#include <qstring.h>

struct Formulation {
    int Id{0};
    QString Name;
    int CategoryId{0};
    QString Usage;
    QString Effect;
    QString Indication;
    QString Disease;
    QString Application;
    QString Supplement;
    QString Song;
    QString Notes;
    QString Source;
};

struct FormulationComposition {
    int Id{0};
    int FormulationId{0};
    int DrugID{0};
    QString DrugName;
    QString Effect;
    QString Position;
    QString Notes;
};

struct FormulationImage {
    int Id{0};
    int FormulationId{0};
    QByteArray Image;
};