
#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

#include <QList>
#include <QString>
#include <QMap>

// Interface functions to convert from raw char** active directory returns to Qt containers
// Also can load fake data if program is run with "fake" option

enum NewEntryType {
    User,
};

extern bool FAKE_AD; 

bool ad_interface_login();
QList<QString> load_children(QString &dn);
QMap<QString, QList<QString>> load_attributes(QString &dn);
bool set_attribute(QString &dn, QString &attribute, QString &value);
QString get_users_container_dn();
QString get_new_user_dn(QString &username);
bool create_entry(QString &name, NewEntryType type);

#endif /* AD_INTERFACE_H */
