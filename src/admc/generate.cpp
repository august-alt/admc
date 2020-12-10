
#include "generate.h"

#include "ad_interface.h"
#include "ad_utils.h"
#include "ad_config.h"
#include "status.h"

#include <algorithm>
#include <QDebug>
#include <QtMath>
#include <QCoreApplication>

void generate() {
    const QString parent = "OU=generated users," + AD()->domain_head();

    const int max_id =
    [parent]() {
        const QList<QString> search_attributes = QList<QString>();
        const QString filter = QString();
        const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, parent);
        QList<QString> users = search_results.keys();

        int max = 0;

        for (const QString user : users) {
            QString user_rdn = user.split(",")[0];
            user_rdn.remove("CN=user");
            const int this_id = user_rdn.toInt();

            if (this_id > max) {
                max = this_id;
            }
        }

        return max;
    }();

    int id = max_id + 1;

    AD()->start_batch();
    const int i_max = 5000;
    int process_events_timer = 0;
    for (int i = 1; i <= i_max; i++) {
        const QString name = QString("user%1").arg(id);
        id++;

        const QString dn = QString("CN=%1,%2").arg(name, parent);

        AD()->object_add(dn, CLASS_USER);


        const QList<QString> attributes = {
            ATTRIBUTE_DESCRIPTION,
            ATTRIBUTE_STREET,
            ATTRIBUTE_WWW_HOMEPAGE,
            ATTRIBUTE_COMPANY,
            ATTRIBUTE_TITLE,
        };
        for (const auto attribute : attributes) {
            AD()->attribute_replace_string(dn, attribute, "test value");
        }
        const int progress_percentage = (int)(((float) i / i_max) * 100);

        STATUS()->message(QString("Generate progress = %1 / %2 = %3 %").arg(i).arg(i_max).arg(progress_percentage), StatusType_Success);

        // Process events periodically to update UI and prevent app handing
        process_events_timer++;
        if (process_events_timer > 20) {
            process_events_timer = 0;
            QCoreApplication::processEvents();
        }
    }
    AD()->end_batch();
}
