/****************************************************************************
** Meta object code from reading C++ file 'maxload_batch.h'
**
** Created: Mon Jun 2 21:59:35 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "maxload_batch.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'maxload_batch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MyDlg[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       7,    6,    6,    6, 0x0a,
      26,    6,    6,    6, 0x0a,
      44,    6,    6,    6, 0x0a,
      58,    6,    6,    6, 0x0a,
      69,    6,    6,    6, 0x0a,
      81,    6,    6,    6, 0x0a,
      97,    6,    6,    6, 0x0a,
     110,    6,    6,    6, 0x0a,
     126,    6,    6,    6, 0x0a,
     151,    6,    6,    6, 0x0a,
     175,    6,    6,    6, 0x0a,
     197,    6,    6,    6, 0x0a,
     221,    6,    6,    6, 0x0a,
     243,    6,    6,    6, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MyDlg[] = {
    "MyDlg\0\0writestripconfig()\0readstripconfig()\0"
    "updatestrip()\0runstrip()\0killstrip()\0"
    "shutdownstrip()\0closestrip()\0"
    "onoff_tab1(int)\0start_together_tab1(int)\0"
    "blocksize_tab1(QString)\0filenum_tab1(QString)\0"
    "clientnum_tab1(QString)\0cbuffer_tab1(QString)\0"
    "sbuffer_tab1(QString)\0"
};

void MyDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MyDlg *_t = static_cast<MyDlg *>(_o);
        switch (_id) {
        case 0: _t->writestripconfig(); break;
        case 1: _t->readstripconfig(); break;
        case 2: _t->updatestrip(); break;
        case 3: _t->runstrip(); break;
        case 4: _t->killstrip(); break;
        case 5: _t->shutdownstrip(); break;
        case 6: _t->closestrip(); break;
        case 7: _t->onoff_tab1((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->start_together_tab1((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->blocksize_tab1((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 10: _t->filenum_tab1((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 11: _t->clientnum_tab1((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 12: _t->cbuffer_tab1((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 13: _t->sbuffer_tab1((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MyDlg::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MyDlg::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_MyDlg,
      qt_meta_data_MyDlg, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyDlg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyDlg))
        return static_cast<void*>(const_cast< MyDlg*>(this));
    return QDialog::qt_metacast(_clname);
}

int MyDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
