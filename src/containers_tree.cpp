
#include "containers_tree.h"
#include "ad_model.h"
#include "ad_filter.h"

#include <QTreeView>
#include <QLabel>

ContainersTree::ContainersTree(AdModel *model, QAction *advanced_view_toggle)
: EntryWidget(model, advanced_view_toggle)
{
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);

    proxy->only_show_containers = true;

    column_hidden[AdModel::Column::Category] = true;
    column_hidden[AdModel::Column::Description] = true;
    column_hidden[AdModel::Column::DN] = true;
    update_column_visibility();

    label->setText("Containers");

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersTree::on_selection_changed);
};

void ContainersTree::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QModelIndex source_index = proxy->mapToSource(index);

        emit selected_container_changed(source_index);
    }
}
