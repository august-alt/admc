/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "tabs/attributes_tab.h"
#include "tabs/ui_attributes_tab.h"

#include "adldap.h"
#include "editors/bool_editor.h"
#include "editors/datetime_editor.h"
#include "editors/multi_editor.h"
#include "editors/octet_editor.h"
#include "editors/string_editor.h"
#include "globals.h"
#include "settings.h"
#include "tabs/attributes_tab_filter_menu.h"
#include "tabs/attributes_tab_proxy.h"
#include "utils.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QStandardItemModel>

QString attribute_type_display_string(const AttributeType type);

AttributesTab::AttributesTab() {
    ui = new Ui::AttributesTab();
    ui->setupUi(this);

    model = new QStandardItemModel(0, AttributesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {AttributesColumn_Name, tr("Name")},
            {AttributesColumn_Value, tr("Value")},
            {AttributesColumn_Type, tr("Type")}
        });

    auto filter_menu = new AttributesTabFilterMenu(this);

    proxy = new AttributesTabProxy(filter_menu, this);

    proxy->setSourceModel(model);
    ui->view->setModel(proxy);

    ui->filter_button->setMenu(filter_menu);

    enable_widget_on_selection(ui->edit_button, ui->view);

    settings_restore_header_state(SETTING_attributes_tab_header_state, ui->view->header());

    const QHash<QString, QVariant> state = settings_get_variant(SETTING_attributes_tab_header_state).toHash();

    ui->view->header()->restoreState(state["header"].toByteArray());

    QItemSelectionModel *selection_model = ui->view->selectionModel();

    connect(
        selection_model, &QItemSelectionModel::selectionChanged,
        this, &AttributesTab::update_edit_and_view_buttons);
    update_edit_and_view_buttons();

    connect(
        ui->view, &QAbstractItemView::doubleClicked,
        this, &AttributesTab::on_double_click);
    connect(
        ui->edit_button, &QAbstractButton::clicked,
        this, &AttributesTab::edit_attribute);
    connect(
        ui->view_button, &QAbstractButton::clicked,
        this, &AttributesTab::view_attribute);
    connect(
        filter_menu, &AttributesTabFilterMenu::filter_changed,
        proxy, &AttributesTabProxy::invalidate);
}

AttributesTab::~AttributesTab() {
    settings_set_variant(SETTING_attributes_tab_header_state, ui->view->header()->saveState());

    delete ui;
}

QList<QStandardItem *> AttributesTab::get_selected_row() const {
    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    const QList<QModelIndex> selecteds = selection_model->selectedRows();

    if (selecteds.isEmpty()) {
        return QList<QStandardItem *>();
    }

    const QModelIndex proxy_index = selecteds[0];
    const QModelIndex index = proxy->mapToSource(proxy_index);

    const QList<QStandardItem *> row = [this, index]() {
        QList<QStandardItem *> out;
        for (int col = 0; col < AttributesColumn_COUNT; col++) {
            const QModelIndex item_index = index.siblingAtColumn(col);
            QStandardItem *item = model->itemFromIndex(item_index);
            out.append(item);
        }
        return out;
    }();

    return row;
}

void AttributesTab::update_edit_and_view_buttons() {
    const QList<QStandardItem *> selected_row = get_selected_row();

    const bool no_selection = selected_row.isEmpty();
    if (no_selection) {
        ui->edit_button->setVisible(true);
        ui->edit_button->setEnabled(false);

        ui->view_button->setVisible(false);
        ui->view_button->setEnabled(false);
    } else {
        const QString attribute = selected_row[AttributesColumn_Name]->text();
        const bool read_only = g_adconfig->get_attribute_is_system_only(attribute);

        if (read_only) {
            ui->edit_button->setVisible(false);
            ui->edit_button->setEnabled(false);

            ui->view_button->setVisible(true);
            ui->view_button->setEnabled(true);
        } else {
            ui->edit_button->setVisible(true);
            ui->edit_button->setEnabled(true);

            ui->view_button->setVisible(false);
            ui->view_button->setEnabled(false);
        }
    }
}

void AttributesTab::on_double_click() {
    const QList<QStandardItem *> selected_row = get_selected_row();
    const QString attribute = selected_row[AttributesColumn_Name]->text();
    const bool read_only = g_adconfig->get_attribute_is_system_only(attribute);

    if (read_only) {
        view_attribute();
    } else {
        edit_attribute();
    }
}

void AttributesTab::view_attribute() {
    const QList<QStandardItem *> row = get_selected_row();

    if (row.isEmpty()) {
        return;
    }

    const QString attribute = row[AttributesColumn_Name]->text();

    AttributeEditor *editor = get_editor(attribute);

    if (editor != nullptr) {
        return;
    }

    const QList<QByteArray> value_list = current[attribute];

    editor->set_attribute(attribute);
    editor->set_value_list(value_list);
    editor->set_read_only(true);
    editor->open();
}

void AttributesTab::edit_attribute() {
    const QList<QStandardItem *> row = get_selected_row();

    if (row.isEmpty()) {
        return;
    }

    const QString attribute = row[AttributesColumn_Name]->text();

    AttributeEditor *editor = get_editor(attribute);

    if (editor == nullptr) {
        return;
    }

    const QList<QByteArray> value_list = current[attribute];

    editor->set_attribute(attribute);
    editor->set_value_list(value_list);
    editor->set_read_only(false);
    editor->open();

    connect(
        editor, &QDialog::accepted,
        [this, editor, row, attribute]() {
            const QList<QByteArray> new_value_list = editor->get_value_list();

            current[attribute] = new_value_list;
            load_row(row, attribute, new_value_list);

            emit edited();
        });
}

void AttributesTab::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    original.clear();

    for (auto attribute : object.attributes()) {
        original[attribute] = object.get_values(attribute);
    }

    // Add attributes without values
    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    const QList<QString> optional_attributes = g_adconfig->get_optional_attributes(object_classes);
    for (const QString &attribute : optional_attributes) {
        if (!original.contains(attribute)) {
            original[attribute] = QList<QByteArray>();
        }
    }

    current = original;

    proxy->load(object);

    model->removeRows(0, model->rowCount());

    for (auto attribute : original.keys()) {
        const QList<QStandardItem *> row = make_item_row(AttributesColumn_COUNT);
        const QList<QByteArray> values = original[attribute];

        model->appendRow(row);
        load_row(row, attribute, values);
    }
}

bool AttributesTab::apply(AdInterface &ad, const QString &target) {
    bool total_success = true;

    for (const QString &attribute : current.keys()) {
        const QList<QByteArray> current_values = current[attribute];
        const QList<QByteArray> original_values = original[attribute];

        if (current_values != original_values) {
            const bool success = ad.attribute_replace_values(target, attribute, current_values);
            if (success) {
                original[attribute] = current_values;
            } else {
                total_success = false;
            }
        }
    }

    return total_success;
}

void AttributesTab::load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values) {
    const QString display_values = attribute_display_values(attribute, values, g_adconfig);
    const AttributeType type = g_adconfig->get_attribute_type(attribute);
    const QString type_display = attribute_type_display_string(type);

    row[AttributesColumn_Name]->setText(attribute);
    row[AttributesColumn_Value]->setText(display_values);
    row[AttributesColumn_Type]->setText(type_display);
}

// Select an appropriate editor by attribute type and by
// whether attribute is single or multi valued
AttributeEditor *AttributesTab::get_editor(const QString &attribute) {
    const bool single_valued = g_adconfig->get_attribute_is_single_valued(attribute);

    // Single/multi valued logic is separated out of the
    // switch statement for better flow
    auto octet_editor = [&]() -> AttributeEditor * {
        if (single_valued) {
            return new OctetEditor(this);
        } else {
            return new MultiEditor(this);
        }
    };

    auto string_editor = [&]() -> AttributeEditor * {
        if (single_valued) {
            return new StringEditor(this);
        } else {
            return new MultiEditor(this);
        }
    };

    auto bool_editor = [&]() -> AttributeEditor * {
        if (single_valued) {
            return new BoolEditor(this);
        } else {
            return new MultiEditor(this);
        }
    };

    auto datetime_editor = [&]() -> AttributeEditor * {
        if (single_valued) {
            return new DateTimeEditor(this);
        } else {
            return nullptr;
        }
    };

    const AttributeType type = g_adconfig->get_attribute_type(attribute);

    switch (type) {
        case AttributeType_Octet: return octet_editor();
        case AttributeType_Sid: return octet_editor();

        case AttributeType_Boolean: return bool_editor();

        case AttributeType_Unicode: return string_editor();
        case AttributeType_StringCase: return string_editor();
        case AttributeType_DSDN: return string_editor();
        case AttributeType_IA5: return string_editor();
        case AttributeType_Teletex: return string_editor();
        case AttributeType_ObjectIdentifier: return string_editor();
        case AttributeType_Integer: return string_editor();
        case AttributeType_Enumeration: return string_editor();
        case AttributeType_LargeInteger: return string_editor();
        case AttributeType_UTCTime: return datetime_editor();
        case AttributeType_GeneralizedTime: return datetime_editor();
        case AttributeType_NTSecDesc: return string_editor();
        case AttributeType_Numeric: return string_editor();
        case AttributeType_Printable: return string_editor();
        case AttributeType_DNString: return string_editor();

        // NOTE: putting these here as confirmed to be unsupported
        case AttributeType_ReplicaLink: return nullptr;
        case AttributeType_DNBinary: return nullptr;
    }

    return nullptr;
}
