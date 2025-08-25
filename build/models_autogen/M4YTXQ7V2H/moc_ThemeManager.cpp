/****************************************************************************
** Meta object code from reading C++ file 'ThemeManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/models/ThemeManager.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ThemeManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_ThemeManager_t {
    uint offsetsAndSizes[22];
    char stringdata0[13];
    char stringdata1[28];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[7];
    char stringdata5[12];
    char stringdata6[10];
    char stringdata7[5];
    char stringdata8[13];
    char stringdata9[6];
    char stringdata10[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ThemeManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_ThemeManager_t qt_meta_stringdata_ThemeManager = {
    {
        QT_MOC_LITERAL(0, 12),  // "ThemeManager"
        QT_MOC_LITERAL(13, 27),  // "effectiveColorSchemeChanged"
        QT_MOC_LITERAL(41, 0),  // ""
        QT_MOC_LITERAL(42, 15),  // "Qt::ColorScheme"
        QT_MOC_LITERAL(58, 6),  // "scheme"
        QT_MOC_LITERAL(65, 11),  // "modeChanged"
        QT_MOC_LITERAL(77, 9),  // "ThemeMode"
        QT_MOC_LITERAL(87, 4),  // "mode"
        QT_MOC_LITERAL(92, 12),  // "FollowSystem"
        QT_MOC_LITERAL(105, 5),  // "Light"
        QT_MOC_LITERAL(111, 4)   // "Dark"
    },
    "ThemeManager",
    "effectiveColorSchemeChanged",
    "",
    "Qt::ColorScheme",
    "scheme",
    "modeChanged",
    "ThemeMode",
    "mode",
    "FollowSystem",
    "Light",
    "Dark"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ThemeManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       1,   32, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   26,    2, 0x06,    1 /* Public */,
       5,    1,   29,    2, 0x06,    3 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,

 // enums: name, alias, flags, count, data
       6,    6, 0x2,    3,   37,

 // enum data: key, value
       8, uint(ThemeManager::ThemeMode::FollowSystem),
       9, uint(ThemeManager::ThemeMode::Light),
      10, uint(ThemeManager::ThemeMode::Dark),

       0        // eod
};

Q_CONSTINIT const QMetaObject ThemeManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ThemeManager.offsetsAndSizes,
    qt_meta_data_ThemeManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ThemeManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ThemeManager, std::true_type>,
        // method 'effectiveColorSchemeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Qt::ColorScheme, std::false_type>,
        // method 'modeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ThemeMode, std::false_type>
    >,
    nullptr
} };

void ThemeManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ThemeManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->effectiveColorSchemeChanged((*reinterpret_cast< std::add_pointer_t<Qt::ColorScheme>>(_a[1]))); break;
        case 1: _t->modeChanged((*reinterpret_cast< std::add_pointer_t<ThemeMode>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ThemeManager::*)(Qt::ColorScheme );
            if (_t _q_method = &ThemeManager::effectiveColorSchemeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ThemeManager::*)(ThemeMode );
            if (_t _q_method = &ThemeManager::modeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *ThemeManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThemeManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ThemeManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ThemeManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ThemeManager::effectiveColorSchemeChanged(Qt::ColorScheme _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ThemeManager::modeChanged(ThemeMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
