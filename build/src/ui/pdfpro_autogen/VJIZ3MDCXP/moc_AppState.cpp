/****************************************************************************
** Meta object code from reading C++ file 'AppState.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/app/AppState.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppState.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6pdfpro2ui8AppStateE_t {};
} // unnamed namespace

template <> constexpr inline auto pdfpro::ui::AppState::qt_create_metaobjectdata<qt_meta_tag_ZN6pdfpro2ui8AppStateE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "pdfpro::ui::AppState",
        "recentFilesChanged",
        "",
        "darkModeChanged",
        "defaultZoomChanged",
        "lastDirectoryChanged",
        "sidebarVisibleChanged",
        "sidebarTabChanged",
        "enginesStarted",
        "enginesStopped",
        "documentOpened",
        "DocumentHandle",
        "handle",
        "documentClosed",
        "currentDocumentChanged",
        "undoRedoChanged",
        "engineProgress",
        "progress",
        "message",
        "engineError",
        "error",
        "recentFiles",
        "darkMode",
        "defaultZoom",
        "lastDirectory",
        "sidebarVisible",
        "sidebarTab"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'recentFilesChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'darkModeChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'defaultZoomChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastDirectoryChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sidebarVisibleChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sidebarTabChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'enginesStarted'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'enginesStopped'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'documentOpened'
        QtMocHelpers::SignalData<void(DocumentHandle)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'documentClosed'
        QtMocHelpers::SignalData<void(DocumentHandle)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'currentDocumentChanged'
        QtMocHelpers::SignalData<void(DocumentHandle)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'undoRedoChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'engineProgress'
        QtMocHelpers::SignalData<void(double, const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 17 }, { QMetaType::QString, 18 },
        }}),
        // Signal 'engineError'
        QtMocHelpers::SignalData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 20 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'recentFiles'
        QtMocHelpers::PropertyData<QStringList>(21, QMetaType::QStringList, QMC::DefaultPropertyFlags, 0),
        // property 'darkMode'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'defaultZoom'
        QtMocHelpers::PropertyData<double>(23, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'lastDirectory'
        QtMocHelpers::PropertyData<QString>(24, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'sidebarVisible'
        QtMocHelpers::PropertyData<bool>(25, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'sidebarTab'
        QtMocHelpers::PropertyData<int>(26, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AppState, qt_meta_tag_ZN6pdfpro2ui8AppStateE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject pdfpro::ui::AppState::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6pdfpro2ui8AppStateE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6pdfpro2ui8AppStateE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6pdfpro2ui8AppStateE_t>.metaTypes,
    nullptr
} };

void pdfpro::ui::AppState::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AppState *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->recentFilesChanged(); break;
        case 1: _t->darkModeChanged(); break;
        case 2: _t->defaultZoomChanged(); break;
        case 3: _t->lastDirectoryChanged(); break;
        case 4: _t->sidebarVisibleChanged(); break;
        case 5: _t->sidebarTabChanged(); break;
        case 6: _t->enginesStarted(); break;
        case 7: _t->enginesStopped(); break;
        case 8: _t->documentOpened((*reinterpret_cast<std::add_pointer_t<DocumentHandle>>(_a[1]))); break;
        case 9: _t->documentClosed((*reinterpret_cast<std::add_pointer_t<DocumentHandle>>(_a[1]))); break;
        case 10: _t->currentDocumentChanged((*reinterpret_cast<std::add_pointer_t<DocumentHandle>>(_a[1]))); break;
        case 11: _t->undoRedoChanged(); break;
        case 12: _t->engineProgress((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 13: _t->engineError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::recentFilesChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::darkModeChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::defaultZoomChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::lastDirectoryChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::sidebarVisibleChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::sidebarTabChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::enginesStarted, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::enginesStopped, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)(DocumentHandle )>(_a, &AppState::documentOpened, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)(DocumentHandle )>(_a, &AppState::documentClosed, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)(DocumentHandle )>(_a, &AppState::currentDocumentChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)()>(_a, &AppState::undoRedoChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)(double , const QString & )>(_a, &AppState::engineProgress, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppState::*)(const QString & )>(_a, &AppState::engineError, 13))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QStringList*>(_v) = _t->recentFiles(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->darkMode(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->defaultZoom(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->lastDirectory(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->sidebarVisible(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->sidebarTab(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setDarkMode(*reinterpret_cast<bool*>(_v)); break;
        case 2: _t->setDefaultZoom(*reinterpret_cast<double*>(_v)); break;
        case 3: _t->setLastDirectory(*reinterpret_cast<QString*>(_v)); break;
        case 4: _t->setSidebarVisible(*reinterpret_cast<bool*>(_v)); break;
        case 5: _t->setSidebarTab(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *pdfpro::ui::AppState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *pdfpro::ui::AppState::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6pdfpro2ui8AppStateE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int pdfpro::ui::AppState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void pdfpro::ui::AppState::recentFilesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void pdfpro::ui::AppState::darkModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void pdfpro::ui::AppState::defaultZoomChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void pdfpro::ui::AppState::lastDirectoryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void pdfpro::ui::AppState::sidebarVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void pdfpro::ui::AppState::sidebarTabChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void pdfpro::ui::AppState::enginesStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void pdfpro::ui::AppState::enginesStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void pdfpro::ui::AppState::documentOpened(DocumentHandle _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void pdfpro::ui::AppState::documentClosed(DocumentHandle _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void pdfpro::ui::AppState::currentDocumentChanged(DocumentHandle _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void pdfpro::ui::AppState::undoRedoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void pdfpro::ui::AppState::engineProgress(double _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1, _t2);
}

// SIGNAL 13
void pdfpro::ui::AppState::engineError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}
QT_WARNING_POP
