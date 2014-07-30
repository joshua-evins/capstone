/****************************************************************************
** Meta object code from reading C++ file 'Slider.h'
**
** Created: Wed Oct 10 13:16:05 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Slider.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Slider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Neumont__Slider[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   17,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Neumont__Slider[] = {
    "Neumont::Slider\0\0newValue\0valueChanged(float)\0"
    "sliderValueChanged()\0"
};

void Neumont::Slider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Slider *_t = static_cast<Slider *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->sliderValueChanged(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Neumont::Slider::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Neumont::Slider::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Neumont__Slider,
      qt_meta_data_Neumont__Slider, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Neumont::Slider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Neumont::Slider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Neumont::Slider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Neumont__Slider))
        return static_cast<void*>(const_cast< Slider*>(this));
    return QWidget::qt_metacast(_clname);
}

int Neumont::Slider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Neumont::Slider::valueChanged(float _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
