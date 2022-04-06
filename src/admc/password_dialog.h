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

#ifndef PASSWORD_DIALOG_H
#define PASSWORD_DIALOG_H

#include <QDialog>

class AttributeEdit;
class AdInterface;
class QLineEdit;

namespace Ui {
class PasswordDialog;
}

// Accepts input of new password and changes password when done
class PasswordDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::PasswordDialog *ui;

    PasswordDialog(AdInterface &ad, const QString &dn, QWidget *parent);
    ~PasswordDialog();

public slots:
    void accept();

private:
    QString target;
    QList<AttributeEdit *> edits;
    AttributeEdit *pass_expired_edit;
    QList<QLineEdit *> required_list;

    void on_edited();
};

#endif /* PASSWORD_DIALOG_H */
