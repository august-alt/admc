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

#ifndef FILTER_WIDGET_H
#define FILTER_WIDGET_H

/**
 * Allows user to enter a filter, which is then passed on to
 * a parent widget. Has tabs for different ways to enter a
 * filter: normal and advanced tab.
 */

#include <QWidget>

class FilterWidgetTab;

namespace Ui {
class FilterWidget;
}

class FilterWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::FilterWidget *ui;

    FilterWidget(QWidget *parent = nullptr);
    ~FilterWidget();

    void set_classes(const QList<QString> &class_list, const QList<QString> &selected_list);

    QString get_filter() const;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

    void clear();

    void enable_filtering_all_classes();
};

class FilterWidgetTab : public QWidget {
    Q_OBJECT

public:
    virtual QString get_filter() const = 0;
    virtual void clear() = 0;
};

#endif /* FILTER_WIDGET_H */
