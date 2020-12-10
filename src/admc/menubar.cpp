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

#include "menubar.h"
#include "ad_interface.h"
#include "settings.h"
#include "confirmation_dialog.h"
#include "find_dialog.h"
#include "toggle_widgets_dialog.h"
#include "status.h"

#include "generate.h"

#include <QMenu>
#include <QLocale>
#include <QMessageBox>
#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QTimer>

MenuBar::MenuBar()
: QMenuBar() {
    QMenu *action_menu = addMenu(tr("Action"));

    auto connect_action = action_menu->addAction(tr("Connect"),
        [this]() {
            STATUS()->start_error_log();
            AD()->connect();
            STATUS()->end_error_log(this);
        });

    action_menu->addAction(tr("Generate objects"),
        []() {
            generate();
        });

    action_menu->addAction(tr("Find"),
        [this]() {
            auto find_dialog = new FindDialog(this);
            find_dialog->open();
        });

    action_menu->addAction(tr("Refresh"),
        []() {
            AD()->refresh();
        });

    filter_contents_action = action_menu->addAction(tr("Filter contents"));

    auto quit_action = action_menu->addAction(tr("Quit"),
        []() {
            QApplication::quit();
        });

    auto add_bool_setting_action = 
    [](QMenu *menu, QString display_text, BoolSetting type) {
        QAction *action = menu->addAction(display_text);
        SETTINGS()->connect_action_to_bool_setting(action, type);
    };

    QMenu *view_menu = addMenu(tr("View"));
    add_bool_setting_action(view_menu, tr("Advanced view"), BoolSetting_AdvancedView);
    add_bool_setting_action(view_menu, tr("Dock Details dialog"), BoolSetting_DetailsIsDocked);
    auto toggle_widgets_action = view_menu->addAction(tr("Toggle widgets"));

    QMenu *preferences_menu = addMenu(tr("Preferences"));
    add_bool_setting_action(preferences_menu, tr("Confirm actions"), BoolSetting_ConfirmActions);
    add_bool_setting_action(preferences_menu, tr("Dev mode"), BoolSetting_DevMode);
    add_bool_setting_action(preferences_menu, tr("Show non-container objects in Containers tree"), BoolSetting_ShowNonContainersInContainersTree);
    add_bool_setting_action(preferences_menu, tr("Put last name before first name when creating users"), BoolSetting_LastNameBeforeFirstName);

    QMenu *language_menu = preferences_menu->addMenu(tr("Language"));
    auto language_group = new QActionGroup(language_menu);

    auto add_language_action =
    [this, language_menu, language_group] (QLocale::Language language) {
        QLocale locale(language);
        const QString language_name = locale.nativeLanguageName();

        const auto action = new QAction(language_name, language_group);
        action->setCheckable(true);
        language_group->addAction(action);
        language_menu->addAction(action);

        const QLocale saved_locale = SETTINGS()->get_variant(VariantSetting_Locale).toLocale();
        const QLocale::Language saved_language = saved_locale.language();
        if (language == saved_language) {
            action->setChecked(true);
        }

        connect(
            action, &QAction::toggled,
            [this, locale](bool checked) {
                if (checked) {
                    SETTINGS()->set_variant(VariantSetting_Locale, locale);

                    QMessageBox::information(this, tr("Info"), tr("App needs to be restarted for the language option to take effect."));
                }
            });
    };

    add_language_action(QLocale::English);
    add_language_action(QLocale::Russian);

    menus = {
        action_menu,
        view_menu,
        preferences_menu,
    };

    // Offline, only "connect" and "exit" are enabled
    enable_actions(false);
    connect_action->setEnabled(true);
    quit_action->setEnabled(true);

    // Once connected, everything is enabled and connect is removed
    connect(
        AD(), &AdInterface::connected,
        [this, action_menu, connect_action]() {
            enable_actions(true);
            action_menu->removeAction(connect_action);
        });

    connect(
        toggle_widgets_action, &QAction::triggered,
        [this]() {
            auto dialog = new ToggleWidgetsDialog(this);
            dialog->open();
        });
}

void MenuBar::enable_actions(const bool enabled) {
    for (auto menu : menus) {
        const QList<QAction *> actions = menu->actions();

        for (auto action : actions) {
            action->setEnabled(enabled);
        }
    }
}
