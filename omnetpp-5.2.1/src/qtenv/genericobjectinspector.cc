//==========================================================================
//  GENERICOBJECTINSPECTOR.CC - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <cstring>
#include <cmath>
#include "omnetpp/cregistrationlist.h"
#include "common/stringutil.h"
#include "common/stlutil.h"
#include "qtenv.h"
#include "inspectorfactory.h"
#include "moduleinspector.h"
#include "loginspector.h"
#include "genericobjectinspector.h"
#include "genericobjecttreemodel.h"
#include "highlighteritemdelegate.h"
#include "displayupdatecontroller.h"
#include "inspectorutil.h"
#include "envir/objectprinter.h"
#include "envir/visitor.h"
#include <QTreeView>
#include <QDebug>
#include <QGridLayout>
#include <QMessageBox>

#define emit

using namespace omnetpp::common;

namespace omnetpp {
namespace qtenv {

void _dummy_for_genericobjectinspector() {}

class GenericObjectInspectorFactory : public InspectorFactory
{
  public:
    GenericObjectInspectorFactory(const char *name) : InspectorFactory(name) {}

    bool supportsObject(cObject *obj) override { return true; }
    InspectorType getInspectorType() override { return INSP_OBJECT; }
    double getQualityAsDefault(cObject *object) override { return 1.0; }
    Inspector *createInspector(QWidget *parent, bool isTopLevel) override { return new GenericObjectInspector(parent, isTopLevel, this); }
};

Register_InspectorFactory(GenericObjectInspectorFactory);

//---- GenericObjectInspector implementation ----

const std::vector<std::string> GenericObjectInspector::containerTypes = {
    "cArray", "cQueue", "cFutureEventSet", "cSimpleModule",
    "cModule", "cChannel", "cRegistrationList", "cCanvas"
};

const QString GenericObjectInspector::PREF_MODE = "mode";

GenericObjectInspector::GenericObjectInspector(QWidget *parent, bool isTopLevel, InspectorFactory *f) : Inspector(parent, isTopLevel, f)
{
    treeView = new QTreeView(this);

    // various cosmetics
    treeView->setHeaderHidden(true);
    treeView->setAttribute(Qt::WA_MacShowFocusRect, false);
    treeView->setUniformRowHeights(true);

    auto delegate = new HighlighterItemDelegate(treeView);
    treeView->setItemDelegate(delegate);
    // pausing the animation (and simulation) while editing is in progress
    auto duc = getQtenv()->getDisplayUpdateController();
    connect(delegate, SIGNAL(editorCreated()), duc, SLOT(pause()));
    connect(delegate, SIGNAL(editorDestroyed()), duc, SLOT(resume()));

    // these enable horizontal scrolling
    treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeView->header()->setStretchLastSection(true);
    treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QToolBar *toolbar = new QToolBar();

    if (!isTopLevel) {
        // aligning right
        QWidget *spacer = new QWidget();
        spacer->setAutoFillBackground(true);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        toolbar->addWidget(spacer);
    }

    // mode selection
    toGroupedModeAction = toolbar->addAction(QIcon(":/tools/treemode_grouped"), "Switch to grouped mode", this, SLOT(toGroupedMode()));
    toGroupedModeAction->setCheckable(true);
    toFlatModeAction = toolbar->addAction(QIcon(":/tools/treemode_flat"), "Switch to flat mode", this, SLOT(toFlatMode()));
    toFlatModeAction->setCheckable(true);
    toChildrenModeAction = toolbar->addAction(QIcon(":/tools/treemode_children"), "Switch to children mode", this, SLOT(toChildrenMode()));
    toChildrenModeAction->setCheckable(true);
    toInheritanceModeAction = toolbar->addAction(QIcon(":/tools/treemode_inher"), "Switch to inheritance mode", this, SLOT(toInheritanceMode()));
    toInheritanceModeAction->setCheckable(true);

    toolbar->addSeparator();

    if (isTopLevel) {
        addTopLevelToolBarActions(toolbar);

        // this is to fill the remaining space on the toolbar, replacing the ugly default gradient on Mac
        QWidget *stretch = new QWidget(toolbar);
        stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        stretch->setAutoFillBackground(true);
        toolbar->addWidget(stretch);
    }
    else {
        goBackAction = toolbar->addAction(QIcon(":/tools/back"), "Back", this, SLOT(goBack()));
        goForwardAction = toolbar->addAction(QIcon(":/tools/forward"), "Forward", this, SLOT(goForward()));
        goUpAction = toolbar->addAction(QIcon(":/tools/parent"), "Go to parent module", this, SLOT(inspectParent()));
    }

    toolbar->setAutoFillBackground(true);

    layout->addWidget(toolbar);
    layout->addWidget(treeView, 1);
    layout->setMargin(0);
    layout->setSpacing(0);
    parent->setMinimumSize(20, 20);

    doSetMode(mode);
    recreateModel();

    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createContextMenu(QPoint)));
    connect(treeView, SIGNAL(activated(QModelIndex)), this, SLOT(onTreeViewActivated(QModelIndex)));

    // getting the data into any items newly brought into view
    connect(treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(gatherVisibleDataIfSafe()));
    connect(treeView, SIGNAL(collapsed(QModelIndex)), this, SLOT(gatherVisibleDataIfSafe()));
    connect(treeView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(gatherVisibleDataIfSafe()));
    connect(treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(gatherVisibleDataIfSafe()));
}

GenericObjectInspector::~GenericObjectInspector()
{
    delete model;
}

void GenericObjectInspector::recreateModel()
{
    GenericObjectTreeModel *newModel = new GenericObjectTreeModel(object, mode, this);
    treeView->setModel(newModel);

    // expanding the top level item
    treeView->expand(newModel->index(0, 0, QModelIndex()));

    delete model;
    model = newModel;

    gatherVisibleDataIfSafe();

    connect(model, SIGNAL(dataEdited(const QModelIndex&)), this, SLOT(onDataEdited()));
}

void GenericObjectInspector::doSetMode(Mode mode)
{
    if (this->mode != mode) {
        this->mode = mode;
        setPref(PREF_MODE, (int)mode);
    }

    toGroupedModeAction->setChecked(mode == Mode::GROUPED);
    toFlatModeAction->setChecked(mode == Mode::FLAT);
    toChildrenModeAction->setChecked(mode == Mode::CHILDREN);
    toInheritanceModeAction->setChecked(mode == Mode::INHERITANCE);
}

void GenericObjectInspector::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
        case Qt::XButton1: goBack(); break;
        case Qt::XButton2: goForward(); break;
        default: /* shut up, compiler! */ break;
    }
}

void GenericObjectInspector::resizeEvent(QResizeEvent *event)
{
    Inspector::resizeEvent(event);
    gatherVisibleDataIfSafe();
}

void GenericObjectInspector::closeEvent(QCloseEvent *event)
{
    setPref(PREF_MODE, (int)mode);
    Inspector::closeEvent(event);
}

void GenericObjectInspector::onTreeViewActivated(const QModelIndex &index)
{
    auto object = model->getCObjectPointer(index);
    if (!object)
        return;

    InspectorFactory *factory = findInspectorFactoryFor(object, INSP_DEFAULT);
    if (!factory) {
        getQtenv()->confirm(Qtenv::INFO, opp_stringf("Class '%s' has no associated inspectors.", object->getClassName()).c_str());
        return;
    }

    int preferredType = factory->getInspectorType();
    if (preferredType != INSP_OBJECT)
        getQtenv()->inspect(object);
    else
        setObject(object);
}

void GenericObjectInspector::onDataEdited()
{
    getQtenv()->callRefreshDisplaySafe();
    getQtenv()->callRefreshInspectors();
}

void GenericObjectInspector::gatherVisibleDataIfSafe()
{
    bool changed = model->gatherMissingDataIfSafeIn(treeView);
    if (changed) {
        // because properly doing it is super slow
        treeView->dataChanged(QModelIndex(), QModelIndex());
        treeView->resizeColumnToContents(0); // and this is needed because of it
    }
}

void GenericObjectInspector::createContextMenu(QPoint pos)
{
    cObject *object = model->getCObjectPointer(treeView->indexAt(pos));
    if (object) {
        QVector<cObject *> objects;
        objects.push_back(object);
        QMenu *menu = InspectorUtil::createInspectorContextMenu(objects, this);
        menu->exec(treeView->mapToGlobal(pos));
        delete menu;
    }
}

void GenericObjectInspector::setMode(Mode mode)
{
    if (this->mode != mode) {
        doSetMode(mode);
        recreateModel();
    }
}

void GenericObjectInspector::doSetObject(cObject *obj)
{
    Inspector::doSetObject(obj);

    if (!obj) {
        doSetMode((Mode)getPref(PREF_MODE, (int)Mode::GROUPED).toInt());
        recreateModel();
        return;
    }

    QSet<QString> expanded = model->getExpandedNodesIn(treeView);

    bool isContainerLike = contains(containerTypes, std::string(getObjectBaseClass(obj)));
    auto defaultMode = isContainerLike ? Mode::CHILDREN : Mode::GROUPED;

    doSetMode((Mode)getPref(PREF_MODE, (int)defaultMode).toInt());
    recreateModel();

    model->expandNodesIn(treeView, expanded);
}

void GenericObjectInspector::refresh()
{
    Inspector::refresh();
    if (object) {
        QString selected = model->getSelectedNodeIn(treeView);

        QSet<QString> expanded = model->getExpandedNodesIn(treeView);
        model->refreshTreeStructure();

        model->expandNodesIn(treeView, expanded);
        if (!selected.isEmpty())
            model->selectNodeIn(treeView, selected);

        model->updateDataIn(treeView);

        // this is a hack, proper item-wise datachanged is super slow
        treeView->dataChanged(QModelIndex(), QModelIndex());
        treeView->resizeColumnToContents(0);
    }
}

}  // namespace qtenv
}  // namespace omnetpp
