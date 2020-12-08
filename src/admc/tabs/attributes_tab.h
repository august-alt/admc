/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ATTRIBUTES_TAB_H
#define ATTRIBUTES_TAB_H

#include "tabs/details_tab.h"

#include <QSortFilterProxyModel>
#include <QHash>
#include <QString>

class QStandardItemModel;
class QStandardItem;
class AttributesTabProxy;
class QTreeView;

enum AttributeFilter {
    AttributeFilter_HideUnset,
    AttributeFilter_HideSystemOnly,
};

// Show attributes of target as a list of attribute names and values
// Values are editable
class AttributesTab final : public DetailsTab {
Q_OBJECT

public:
    AttributesTab();

    void load(const AdObject &object) override;
    void apply(const QString &target) const override;

private slots:
    void on_double_clicked(const QModelIndex &proxy_index);
    void open_filter_dialog();

private:
    QTreeView *view;
    QStandardItemModel *model;
    AttributesTabProxy *proxy;
    QHash<QString, QList<QByteArray>> original;
    QHash<QString, QList<QByteArray>> current;

    void load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values);
    void showEvent(QShowEvent *event);
};

class AttributesTabProxy final : public QSortFilterProxyModel {

public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    QHash<AttributeFilter, bool> filters;
    QHash<QString, bool> unset_map;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* ATTRIBUTES_TAB_H */
