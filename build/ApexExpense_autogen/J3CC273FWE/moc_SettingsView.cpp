/****************************************************************************
** Meta object code from reading C++ file 'SettingsView.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/views/SettingsView.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SettingsView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SettingsView_t {
    QByteArrayData data[11];
    char stringdata0[152];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SettingsView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SettingsView_t qt_meta_stringdata_SettingsView = {
    {
QT_MOC_LITERAL(0, 0, 12), // "SettingsView"
QT_MOC_LITERAL(1, 13, 11), // "refreshData"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 17), // "onCurrencyChanged"
QT_MOC_LITERAL(4, 44, 6), // "symbol"
QT_MOC_LITERAL(5, 51, 18), // "onExportJsonBackup"
QT_MOC_LITERAL(6, 70, 18), // "onImportJsonBackup"
QT_MOC_LITERAL(7, 89, 16), // "onLoadSampleData"
QT_MOC_LITERAL(8, 106, 14), // "onClearAllData"
QT_MOC_LITERAL(9, 121, 13), // "onAddCategory"
QT_MOC_LITERAL(10, 135, 16) // "onDeleteCategory"

    },
    "SettingsView\0refreshData\0\0onCurrencyChanged\0"
    "symbol\0onExportJsonBackup\0onImportJsonBackup\0"
    "onLoadSampleData\0onClearAllData\0"
    "onAddCategory\0onDeleteCategory"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SettingsView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x0a /* Public */,
       3,    1,   55,    2, 0x08 /* Private */,
       5,    0,   58,    2, 0x08 /* Private */,
       6,    0,   59,    2, 0x08 /* Private */,
       7,    0,   60,    2, 0x08 /* Private */,
       8,    0,   61,    2, 0x08 /* Private */,
       9,    0,   62,    2, 0x08 /* Private */,
      10,    0,   63,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SettingsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SettingsView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->refreshData(); break;
        case 1: _t->onCurrencyChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onExportJsonBackup(); break;
        case 3: _t->onImportJsonBackup(); break;
        case 4: _t->onLoadSampleData(); break;
        case 5: _t->onClearAllData(); break;
        case 6: _t->onAddCategory(); break;
        case 7: _t->onDeleteCategory(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SettingsView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SettingsView.data,
    qt_meta_data_SettingsView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SettingsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SettingsView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SettingsView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SettingsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
