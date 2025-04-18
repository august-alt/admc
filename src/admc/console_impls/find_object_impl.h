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

#ifndef FIND_OBJECT_IMPL_H
#define FIND_OBJECT_IMPL_H

/**
 * Impl for root of the find object tree which displays
 * results of a "Find" operation. Used in Find Widget.
 */

#include "console_widget/console_impl.h"

class FindObjectImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    FindObjectImpl(ConsoleWidget *console_arg);

    QString get_description(const QModelIndex &index) const override;

    QList<QString> column_labels() const override;
    QList<int> default_columns() const override;
};

QModelIndex get_find_object_root(ConsoleWidget *console);

#endif /* FIND_OBJECT_IMPL_H */
