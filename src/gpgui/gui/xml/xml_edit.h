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

#ifndef XML_EDIT_H
#define XML_EDIT_H

#include <QObject>
#include <QDomDocument>
#include <QString>

class QGridLayout;
class QWidget;

class XmlEdit : public QObject {
Q_OBJECT
public:
    virtual void add_to_layout(QGridLayout *layout) = 0;
    virtual void load(const QDomDocument &doc) = 0;
    virtual bool changed() const = 0;
    virtual bool verify_input(QWidget *parent) = 0;
    virtual void apply(QDomDocument *doc) = 0;

signals:
    void edited();
};

#define DECL_XML_EDIT_VIRTUALS()\
void add_to_layout(QGridLayout *layout);\
void load(const QDomDocument &doc);\
bool changed() const;\
bool verify_input(QWidget *parent);\
void apply(QDomDocument *doc);

QDomElement get_element_by_tag_name(const QDomDocument &doc, const QString &tag_name);

#endif /* XML_EDIT_H */
