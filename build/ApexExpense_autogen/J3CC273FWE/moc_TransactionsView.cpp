/****************************************************************************
** Meta object code from reading C++ file 'TransactionsView.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/views/TransactionsView.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TransactionsView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TransactionsView_t {
    QByteArrayData data[15];
    char stringdata0[201];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TransactionsView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TransactionsView_t qt_meta_stringdata_TransactionsView = {
    {
QT_MOC_LITERAL(0, 0, 16), // "TransactionsView"
QT_MOC_LITERAL(1, 17, 11), // "refreshData"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 13), // "openAddDialog"
QT_MOC_LITERAL(4, 44, 11), // "defaultType"
QT_MOC_LITERAL(5, 56, 15), // "onSearchChanged"
QT_MOC_LITERAL(6, 72, 19), // "onDateFilterChanged"
QT_MOC_LITERAL(7, 92, 5), // "index"
QT_MOC_LITERAL(8, 98, 13), // "onEditClicked"
QT_MOC_LITERAL(9, 112, 15), // "onDeleteClicked"
QT_MOC_LITERAL(10, 128, 18), // "onExportCsvClicked"
QT_MOC_LITERAL(11, 147, 18), // "onImportCsvClicked"
QT_MOC_LITERAL(12, 166, 23), // "onTableRowDoubleClicked"
QT_MOC_LITERAL(13, 190, 3), // "row"
QT_MOC_LITERAL(14, 194, 6) // "column"

    },
    "TransactionsView\0refreshData\0\0"
    "openAddDialog\0defaultType\0onSearchChanged\0"
    "onDateFilterChanged\0index\0onEditClicked\0"
    "onDeleteClicked\0onExportCsvClicked\0"
    "onImportCsvClicked\0onTableRowDoubleClicked\0"
    "row\0column"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TransactionsView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    1,   65,    2, 0x0a /* Public */,
       3,    0,   68,    2, 0x2a /* Public | MethodCloned */,
       5,    0,   69,    2, 0x08 /* Private */,
       6,    1,   70,    2, 0x08 /* Private */,
       8,    0,   73,    2, 0x08 /* Private */,
       9,    0,   74,    2, 0x08 /* Private */,
      10,    0,   75,    2, 0x08 /* Private */,
      11,    0,   76,    2, 0x08 /* Private */,
      12,    2,   77,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   13,   14,

       0        // eod
};

void TransactionsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TransactionsView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->refreshData(); break;
        case 1: _t->openAddDialog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->openAddDialog(); break;
        case 3: _t->onSearchChanged(); break;
        case 4: _t->onDateFilterChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onEditClicked(); break;
        case 6: _t->onDeleteClicked(); break;
        case 7: _t->onExportCsvClicked(); break;
        case 8: _t->onImportCsvClicked(); break;
        case 9: _t->onTableRowDoubleClicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TransactionsView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_TransactionsView.data,
    qt_meta_data_TransactionsView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TransactionsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TransactionsView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TransactionsView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TransactionsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
