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
#include "tabs/attributes_tab_p.h"

#include "adldap.h"
#include "editors/attribute_editor.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_Type,
    AttributesColumn_COUNT,
};

QHash<AttributeFilter, bool> get_filter_state_from_settings();

QString attribute_type_display_string(const AttributeType type);

AttributesTab::AttributesTab() {
    filter_dialog = new AttributesFilterDialog(this);

    model = new QStandardItemModel(0, AttributesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {AttributesColumn_Name, tr("Name")},
            {AttributesColumn_Value, tr("Value")},
            {AttributesColumn_Type, tr("Type")},
        });

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    proxy = new AttributesTabProxy(this);

    proxy->setSourceModel(model);
    view->setModel(proxy);

    g_settings->setup_header_state(view->header(), VariantSetting_AttributesHeader);

    auto edit_button = new QPushButton(tr("Edit"));
    edit_button->setObjectName("edit_button");
    auto filter_button = new QPushButton(tr("Filter"));
    filter_button->setObjectName("filter_button");
    auto buttons = new QHBoxLayout();
    buttons->addWidget(edit_button);
    buttons->addStretch();
    buttons->addWidget(filter_button);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addLayout(buttons);

    enable_widget_on_selection(edit_button, view);

    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &AttributesTab::edit_attribute);
    connect(
        edit_button, &QAbstractButton::clicked,
        this, &AttributesTab::edit_attribute);
    connect(
        filter_button, &QAbstractButton::clicked,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        proxy, &AttributesTabProxy::update_filter_state);
    proxy->update_filter_state();
}

void AttributesTab::edit_attribute() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selecteds = selection_model->selectedRows();

    if (selecteds.isEmpty()) {
        return;
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

    const QString attribute = row[AttributesColumn_Name]->text();
    const QList<QByteArray> values = current[attribute];

    AttributeEditor *editor = AttributeEditor::make(attribute, values, this);
    if (editor != nullptr) {
        connect(
            editor, &QDialog::accepted,
            [this, editor, attribute, row]() {
                const QList<QByteArray> new_values = editor->get_new_values();

                current[attribute] = new_values;
                load_row(row, attribute, new_values);

                emit edited();
            });

        editor->open();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("No editor is available for this attribute type."));
    }
}

void AttributesTab::load(AdInterface &ad, const AdObject &object) {
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

    view->sortByColumn(AttributesColumn_Name, Qt::AscendingOrder);
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

void AttributesTab::update_proxy() {

}

AttributesFilterDialog::AttributesFilterDialog(QWidget *parent)
: QDialog(parent) {
    // Load filter state from settings
    const QHash<AttributeFilter, bool> filter_state = get_filter_state_from_settings();

    auto add_check = [&](const QString text, const AttributeFilter filter) {
        auto check = new QCheckBox();
        check->setText(text);
        check->setChecked(filter_state[filter]);
        check->setObjectName(QString::number(filter));

        checks.insert(filter, check);
    };

    add_check(tr("Unset"), AttributeFilter_Unset);
    add_check(tr("Read-only"), AttributeFilter_ReadOnly);
    add_check(tr("Mandatory"), AttributeFilter_Mandatory);
    add_check(tr("Optional"), AttributeFilter_Optional);
    add_check(tr("System-only"), AttributeFilter_SystemOnly);
    add_check(tr("Constructed"), AttributeFilter_Constructed);
    add_check(tr("Backlink"), AttributeFilter_Backlink);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    // Group checkboxes into frames
    auto make_frame = []() -> QFrame * {
        auto frame = new QFrame();
        frame->setFrameShape(QFrame::Box);
        frame->setFrameShadow(QFrame::Raised);
        frame->setLayout(new QVBoxLayout());

        return frame;
    };

    QFrame *first_frame = make_frame();
    first_frame->layout()->addWidget(checks[AttributeFilter_Unset]);
    first_frame->layout()->addWidget(checks[AttributeFilter_ReadOnly]);

    QFrame *second_frame = make_frame();
    second_frame->layout()->addWidget(checks[AttributeFilter_Mandatory]);
    second_frame->layout()->addWidget(checks[AttributeFilter_Optional]);

    QFrame *third_frame = make_frame();
    third_frame->layout()->addWidget(new QLabel(tr("Read-only attributes:")));
    third_frame->layout()->addWidget(checks[AttributeFilter_SystemOnly]);
    third_frame->layout()->addWidget(checks[AttributeFilter_Constructed]);
    third_frame->layout()->addWidget(checks[AttributeFilter_Backlink]);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(first_frame);
    layout->addWidget(second_frame);
    layout->addWidget(third_frame);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    connect(
        checks[AttributeFilter_ReadOnly], &QCheckBox::stateChanged,
        this, &AttributesFilterDialog::on_read_only_check);
    on_read_only_check();
}

void AttributesFilterDialog::accept() {
    // Save filters to settings
    const QVariant filters_variant = [=]() {
        QList<QVariant> filters_list;

        for (int filter_i = 0; filter_i < AttributeFilter_COUNT; filter_i++) {
            const AttributeFilter filter = (AttributeFilter) filter_i;
            const bool filter_is_set = checks[filter]->isChecked();

            filters_list.append(QVariant(filter_is_set));
        }

        return QVariant(filters_list);
    }();
    g_settings->set_variant(VariantSetting_AttributesTabFilter, filters_variant);

    const QHash<AttributeFilter, bool> filter_state = [&]() {
        QHash<AttributeFilter, bool> out;

        for (const AttributeFilter filter : checks.keys()) {
            const QCheckBox *check = checks[filter];
            out[filter] = check->isChecked();
        }

        return out;
    }();

    QDialog::accept();
}

// Enable readonly subtype checks when readonly is enabled
void AttributesFilterDialog::on_read_only_check() {
    const bool read_only_enabled = checks[AttributeFilter_ReadOnly]->isChecked();

    checks[AttributeFilter_SystemOnly]->setEnabled(read_only_enabled);
    checks[AttributeFilter_Constructed]->setEnabled(read_only_enabled);
    checks[AttributeFilter_Backlink]->setEnabled(read_only_enabled);
}

AttributesTabProxy::AttributesTabProxy(QObject *parent)
: QSortFilterProxyModel(parent) {

}

void AttributesTabProxy::update_filter_state() {
    filter_state = get_filter_state_from_settings();

    invalidate();
}

void AttributesTabProxy::load(const AdObject &object) {
    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    mandatory_attributes = g_adconfig->get_mandatory_attributes(object_classes).toSet();
    optional_attributes = g_adconfig->get_optional_attributes(object_classes).toSet();

    set_attributes = object.attributes().toSet();
}

bool AttributesTabProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto source = sourceModel();
    const QString attribute = source->index(source_row, AttributesColumn_Name, source_parent).data().toString();

    const bool system_only = g_adconfig->get_attribute_is_system_only(attribute);
    const bool unset = !set_attributes.contains(attribute);
    const bool mandatory = mandatory_attributes.contains(attribute);
    const bool optional = optional_attributes.contains(attribute);

    if (!filter_state[AttributeFilter_Unset] && unset) {
        return false;
    }

    if (!filter_state[AttributeFilter_Mandatory] && mandatory) {
        return false;
    }

    if (!filter_state[AttributeFilter_Optional] && optional) {
        return false;
    }

    if (filter_state[AttributeFilter_ReadOnly] && system_only) {
        const bool constructed = g_adconfig->get_attribute_is_constructed(attribute);
        const bool backlink = g_adconfig->get_attribute_is_backlink(attribute);

        if (!filter_state[AttributeFilter_SystemOnly] && !constructed && !backlink) {
            return false;
        }

        if (!filter_state[AttributeFilter_Constructed] && constructed) {
            return false;
        }

        if (!filter_state[AttributeFilter_Backlink] && backlink) {
            return false;
        }
    }

    if (!filter_state[AttributeFilter_ReadOnly] && system_only) {
        return false;
    }

    return true;
}

QString attribute_type_display_string(const AttributeType type) {
    switch (type) {
        case AttributeType_Boolean: return AttributesTab::tr("Boolean");
        case AttributeType_Enumeration: return AttributesTab::tr("Enumeration");
        case AttributeType_Integer: return AttributesTab::tr("Integer");
        case AttributeType_LargeInteger: return AttributesTab::tr("Large Integer");
        case AttributeType_StringCase: return AttributesTab::tr("String Case");
        case AttributeType_IA5: return AttributesTab::tr("IA5");
        case AttributeType_NTSecDesc: return AttributesTab::tr("NT Security Descriptor");
        case AttributeType_Numeric: return AttributesTab::tr("Numeric");
        case AttributeType_ObjectIdentifier: return AttributesTab::tr("Object Identifier");
        case AttributeType_Octet: return AttributesTab::tr("Octet");
        case AttributeType_ReplicaLink: return AttributesTab::tr("Replica Link");
        case AttributeType_Printable: return AttributesTab::tr("Printable");
        case AttributeType_Sid: return AttributesTab::tr("SID");
        case AttributeType_Teletex: return AttributesTab::tr("Teletex");
        case AttributeType_Unicode: return AttributesTab::tr("Unicode");
        case AttributeType_UTCTime: return AttributesTab::tr("UTC Time");
        case AttributeType_GeneralizedTime: return AttributesTab::tr("Generalized Time");
        case AttributeType_DNString: return AttributesTab::tr("DN String");
        case AttributeType_DNBinary: return AttributesTab::tr("DN Binary");
        case AttributeType_DSDN: return AttributesTab::tr("Distinguished Name");
    }
    return QString();
}

QHash<AttributeFilter, bool> get_filter_state_from_settings() {
    QHash<AttributeFilter, bool> out;

    const QVariant filters_variant = g_settings->get_variant(VariantSetting_AttributesTabFilter);
    const QList<QVariant> filters_list = filters_variant.toList();
    for (int filter_i = 0; filter_i < AttributeFilter_COUNT; filter_i++) {
        const AttributeFilter filter = (AttributeFilter) filter_i;

        const bool filter_is_set = [&]() {
            if (filter_i < filters_list.size()) {
                return filters_list[filter_i].toBool();
            } else {
                return true;
            }
        }();

        out[filter] = filter_is_set;
    }

    return out;
}
