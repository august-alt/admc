/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#ifndef ADMC_TEST_EDIT_QUERY_ITEM_WIDGET_H
#define ADMC_TEST_EDIT_QUERY_ITEM_WIDGET_H

#include "admc_test.h"

class EditQueryItemWidget;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;
class QTextEdit;

class ADMCTestEditQueryItemWidget : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void save_and_load();

private:
    EditQueryItemWidget *widget;
    QLineEdit *name_edit;
    QLineEdit *description_edit;
    QCheckBox *scope_checkbox;
    QComboBox *base_combo;
    QPushButton *edit_filter_button;
    QTextEdit *filter_display;
};

#endif /* ADMC_TEST_EDIT_QUERY_ITEM_WIDGET_H */
