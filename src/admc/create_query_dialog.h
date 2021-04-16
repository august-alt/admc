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

#ifndef CREATE_QUERY_DIALOG_H
#define CREATE_QUERY_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QModelIndex>

class QLineEdit;
class FilterWidget;
class SearchBaseWidget;
class ConsoleWidget;

class CreateQueryDialog : public QDialog {
Q_OBJECT

public:
    CreateQueryDialog(ConsoleWidget *console_arg);

private:
    QLineEdit *name_edit;
    QLineEdit *description_edit;
    FilterWidget *filter_widget;
    SearchBaseWidget *search_base_widget;
    ConsoleWidget *console;

    void accept() override;
};

#endif /* CREATE_QUERY_DIALOG_H */