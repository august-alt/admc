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

#ifndef ATTRIBUTES_TAB_DIALOG_H
#define ATTRIBUTES_TAB_DIALOG_H

#include <QString>
#include <QDialog>
#include <QList>
#include <QByteArray>

// Gets input from user and then returns it through
// get_new_values(). Whatever widget opened this
// dialog should be applying changes.

class EditDialog : public QDialog {
Q_OBJECT

public:
    static EditDialog *make(const QString attribute, const QList<QByteArray> values);

    virtual QList<QByteArray> get_new_values() const = 0;
};

#endif /* ATTRIBUTES_TAB_DIALOG_H */