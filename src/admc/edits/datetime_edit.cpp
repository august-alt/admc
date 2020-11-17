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

#include "edits/datetime_edit.h"
#include "ad_config.h"
#include "utils.h"
#include "ad_interface.h"

#include <QFormLayout>
#include <QDateTimeEdit>
#include <QLabel>

DateTimeEdit::DateTimeEdit(const QString &attribute_arg, QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(parent)
{
    edit = new QDateTimeEdit();
    attribute = attribute_arg;

    QObject::connect(
        edit, &QDateTimeEdit::dateTimeChanged,
        [this]() {
            emit edited();
        });

    AttributeEdit::append_to_list(edits_out);
}

void DateTimeEdit::load(const AdObject &object) {
    original_value = object.get_datetime(attribute);

    edit->setDateTime(original_value);

    emit edited();
}

void DateTimeEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void DateTimeEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = ADCONFIG()->get_attribute_display_name(attribute, "") + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(label);

    layout->addRow(label, edit);
}

bool DateTimeEdit::verify() const {
    // TODO: datetime should fit within bounds of it's format, so greater than start of epoch for NTFS format?

    return true;
}

bool DateTimeEdit::changed() const {
    const QDateTime new_value = edit->dateTime();
    return (new_value != original_value);
}

bool DateTimeEdit::apply(const QString &dn) const {
    const QDateTime new_value = edit->dateTime();

    const bool success = AD()->attribute_replace_datetime(dn, attribute, new_value);

    return success;
}
