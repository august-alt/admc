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

#include "attributes_tab_dialog_string.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// TODO: figure out what can and can't be renamed and disable renaming for exceptions (computers can't for example)

AttributesTabDialogString::AttributesTabDialogString(const QString attribute, const QList<QByteArray> values)
: AttributesTabDialog()
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(300, 300);

    const auto title_text = QString(tr("Edit %1")).arg(attribute);
    const auto title_label = new QLabel(title_text);

    edit = new QLineEdit();

    const QByteArray value = values.value(0, QByteArray());
    original_value = QString::fromUtf8(value);
    edit->setText(original_value);

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addWidget(edit);
    top_layout->addWidget(button_box);

    // TODO: verify before saving?
    connect(
        button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &AttributesTabDialogString::on_cancel);
}

void AttributesTabDialogString::on_cancel() {
    // Reset to original display value
    edit->setText(original_value);
}

QList<QByteArray> AttributesTabDialogString::get_new_values() const {
    const QString new_value_string = edit->text();

    if (new_value_string.isEmpty()) {
        return {};
    } else {
        const QByteArray new_value = new_value_string.toUtf8();
        return {new_value};
    }
}
