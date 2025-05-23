/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "tabs/general_ou_tab.h"
#include "tabs/ui_general_ou_tab.h"

#include "adldap.h"
#include "attribute_edits/country_edit.h"
#include "attribute_edits/general_name_edit.h"
#include "attribute_edits/string_edit.h"

GeneralOUTab::GeneralOUTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralOUTab();
    ui->setupUi(this);

    auto name_edit = new GeneralNameEdit(ui->name_label, this);
    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto street_edit = new StringEdit(ui->street_edit, ATTRIBUTE_STREET_OU, this);
    auto city_edit = new StringEdit(ui->city_edit, ATTRIBUTE_CITY, this);
    auto state_edit = new StringEdit(ui->state_edit, ATTRIBUTE_STATE, this);
    auto postal_code_edit = new StringEdit(ui->postal_code_edit, ATTRIBUTE_POSTAL_CODE, this);

    auto country_edit = new CountryEdit(ui->country_combo, this);

    edit_list->append({
        name_edit,
        description_edit,
        street_edit,
        city_edit,
        state_edit,
        postal_code_edit,
        country_edit,
    });
}

GeneralOUTab::~GeneralOUTab() {
    delete ui;
}
