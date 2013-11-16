/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2008-2013 Calle Laakkonen

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#include <QItemSelection>
#include <QListView>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

#include "layerlistwidget.h"
#include "layerlistdelegate.h"
#include "layerlistitem.h"

#include "canvasscene.h"
#include "core/layerstack.h"
#include "core/layer.h"

namespace widgets {

LayerListWidget::LayerListWidget(QWidget *parent)
	: QDockWidget(tr("Layers"), parent)
{
	_list = new QListView(this);
	setWidget(_list);

	_list->setDragEnabled(true);
	_list->viewport()->setAcceptDrops(true);
	// Disallow automatic selections. We handle them ourselves in the delegate.
	_list->setSelectionMode(QAbstractItemView::NoSelection);

	LayerListDelegate *del = new LayerListDelegate(this);
	_list->setItemDelegate(del);
	
	_model = new LayerListModel(this);
	_list->setModel(_model);

	// Connect signals from controls buttons
	// These signals may be processed and are re-emitted.
	connect(del, SIGNAL(newLayer()), this, SLOT(newLayer()));
	connect(del, SIGNAL(deleteLayer(const dpcore::Layer*)), this,
			SLOT(deleteLayer(const dpcore::Layer*)));
	connect(del, SIGNAL(layerToggleHidden(int)), this,
			SIGNAL(layerToggleHidden(int)));
	connect(del, SIGNAL(renameLayer(int, const QString&)), this,
			SIGNAL(renameLayer(int, const QString&)));
	connect(del, SIGNAL(changeOpacity(int, int)), this,
			SIGNAL(opacityChange(int,int)));
	connect(del, SIGNAL(select(const QModelIndex&)),
			this, SLOT(selected(const QModelIndex&)));
}

/**
 * This is called to synchronize the UI with changes that have happened
 * due to things like layer deletion and network events.
 * @param id layer ID
 */
void LayerListWidget::setSelection(int id)
{
#if 0
	dpcore::LayerStack *layers = static_cast<dpcore::LayerStack*>(_list->model());
	int row = layers->layers() - layers->id2index(id);
	QModelIndexList sel = _list->selectionModel()->selectedIndexes();
	if(sel.isEmpty() || sel.first().row() != row) {
		_list->selectionModel()->clear();
		_list->selectionModel()->select(layers->index(row,0),
				QItemSelectionModel::Select);
	}
#endif
}

#if 0
/**
 * A layer was selected via delegate. Update the UI and emit a signal
 * to inform the Controller of the new selection.
 */
void LayerListWidget::selected(const QModelIndex& index)
{
	const dpcore::Layer *layer = index.data().value<dpcore::Layer*>();
	_list->selectionModel()->clear();
	_list->selectionModel()->select(index, QItemSelectionModel::Select);
	emit selected(layer->id());
}

/**
 * Check if it was the currently selected layer that was just moved.
 * If so, update the selection to reflect the new position. The real selection
 * has not changed, so only the UI needs to be updated.
 */
void LayerListWidget::moved(const QModelIndex& from, const QModelIndex& to)
{
	QModelIndex sel = _list->selectionModel()->selection().indexes().first();
	if(from == sel) {
		_list->selectionModel()->clear();
		_list->selectionModel()->select(to, QItemSelectionModel::Select);
	}
}

/**
 * Opacity was changed via UI. Emit a signal to inform the Controller.
 */
void LayerListWidget::opacityChanged(int opacity)
{
	const dpcore::Layer *layer = _list->selectionModel()->selection().indexes().first().data().value<dpcore::Layer*>();
	emit opacityChange(layer->id(), opacity);
}

/**
 * New layer button was pressed
 */
void LayerListWidget::newLayer()
{
	bool ok;
	QString name = QInputDialog::getText(this, tr("Add a new layer"),
			tr("Layer name:"), QLineEdit::Normal, "", &ok);
	if(ok) {
		if(name.isEmpty())
			name = tr("Unnamed layer");
		emit newLayer(name);
	}
}

/**
 * Delete layer button was pressed
 */
void LayerListWidget::deleteLayer(const dpcore::Layer *layer)
{
	QMessageBox box(QMessageBox::Question, tr("Delete layer"),
			tr("Really delete \"%1\"?").arg(layer->name()),
			QMessageBox::NoButton, this);

	box.addButton(tr("Delete"), QMessageBox::DestructiveRole);

	// Offer the choice to merge down only if there is a layer
	// below this one.
	QPushButton *merge = 0;
	if(static_cast<dpcore::LayerStack*>(_list->model())->isBottommost(layer)==false) {
		merge = box.addButton(tr("Merge down"), QMessageBox::DestructiveRole);
		box.setInformativeText(tr("Press merge down to merge the layer with the first visible layer below instead of deleting."));
	}

	QPushButton *cancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);

	box.setDefaultButton(cancel);
	box.exec();

	QAbstractButton *choice = box.clickedButton();
	if(choice != cancel)
		emit deleteLayer(layer->id(), choice==merge);
}
#endif

}

