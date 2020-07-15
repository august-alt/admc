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

#include "config.h"
#include "admc.h"
#include "main_window.h"
#include "ad_interface.h"
#include "settings.h"

#include <QCommandLineParser>
#include <QStringList>

int main(int argc, char **argv) {
    const auto admc = new ADMC(argc, argv);
    admc->setApplicationDisplayName(ADMC_APPLICATION_DISPLAY_NAME);
    admc->setApplicationName(ADMC_APPLICATION_NAME);
    admc->setApplicationVersion(ADMC_VERSION);
    admc->setOrganizationName(ADMC_ORGANIZATION);
    admc->setOrganizationDomain(ADMC_ORGANIZATION_DOMAIN);
    // NOTE: must load settings after setting app/org names so that
    // settings file path is correct
    admc->settings()->load_settings();

    QCommandLineParser cli_parser;
    cli_parser.setApplicationDescription(QCoreApplication::applicationName());
    cli_parser.addHelpOption();
    cli_parser.addVersionOption();

    cli_parser.addOption({{"H", "host"}, "Host to use for login", "host"});
    cli_parser.addOption({{"D", "domain"}, "Domain to use for login", "domain"});

    const QStringList arg_list = qApp->arguments();
    cli_parser.process(arg_list);

    QStringList positional_args = cli_parser.positionalArguments();
    if (positional_args.size() > 0) {
        // CLI
        const bool defined_login_values = cli_parser.isSet("host") && cli_parser.isSet("domain");
        if (!defined_login_values) {
            printf("Error: must define host and domain options, see help for options.");

            return 1;
        }

        const QString host = cli_parser.value("host");
        const QString domain = cli_parser.value("domain");
        
        AD()->login(host, domain);
        AD()->command(positional_args);

        return 0;
    } else {
        // GUI
        MainWindow main_window;
        main_window.show();

        const int retval = admc->exec();

        return retval;
    }
}
