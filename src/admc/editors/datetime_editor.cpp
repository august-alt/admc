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

#include "editors/datetime_editor.h"
#include "editors/ui_datetime_editor.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"

DateTimeEditor::DateTimeEditor(QWidget *parent)
: AttributeEditor(parent) {
    ui = new Ui::DateTimeEditor();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    AttributeEditor::set_attribute_label(ui->attribute_label);

    settings_setup_dialog_geometry(SETTING_datetime_editor_geometry, this);
}

DateTimeEditor::~DateTimeEditor() {
    delete ui;
}

void DateTimeEditor::set_read_only(const bool read_only) {
    AttributeEditor::set_read_only(read_only);

    ui->edit->setReadOnly(read_only);
}

void DateTimeEditor::set_value_list(const QList<QByteArray> &values) {
    const QByteArray value = values.value(0, QByteArray());
    const QString value_string = QString(value);
    const QDateTime value_datetime = datetime_string_to_qdatetime(get_attribute(), value_string, g_adconfig);
    ui->edit->setDateTime(value_datetime);
}

QList<QByteArray> DateTimeEditor::get_value_list() const {
    return QList<QByteArray>();
}
