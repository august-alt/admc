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

#ifndef GENERAL_OU_TAB_H
#define GENERAL_OU_TAB_H

#include <QWidget>

class AdObject;
class AttributeEdit;

namespace Ui {
class GeneralOUTab;
}

class GeneralOUTab final : public QWidget {
    Q_OBJECT

public:
    Ui::GeneralOUTab *ui;

    GeneralOUTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~GeneralOUTab();
};

#endif /* GENERAL_OU_TAB_H */