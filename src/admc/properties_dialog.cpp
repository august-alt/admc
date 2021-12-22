/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "properties_dialog.h"
#include "ui_properties_dialog.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "globals.h"
#include "properties_warning_dialog.h"
#include "settings.h"
#include "status.h"
#include "tab_widget.h"
#include "tabs/account_tab.h"
#include "tabs/address_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/delegation_tab.h"
#include "tabs/general_computer_tab.h"
#include "tabs/general_group_tab.h"
#include "tabs/general_other_tab.h"
#include "tabs/general_ou_tab.h"
#include "tabs/general_policy_tab.h"
#include "tabs/general_user_tab.h"
#include "tabs/general_computer_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/managed_by_tab.h"
#include "tabs/membership_tab.h"
#include "tabs/object_tab.h"
#include "tabs/organization_tab.h"
#include "tabs/os_tab.h"
#include "tabs/profile_tab.h"
#include "tabs/security_tab.h"
#include "tabs/telephones_tab.h"
#include "tabs/laps_tab.h"
#include "utils.h"

#include <QAbstractItemView>
#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QPushButton>

QHash<QString, PropertiesDialog *> PropertiesDialog::instances;

PropertiesDialog *PropertiesDialog::open_for_target(AdInterface &ad, const QString &target, bool *dialog_is_new) {
    if (target.isEmpty()) {
        return nullptr;
    }

    show_busy_indicator();

    const bool dialog_already_open_for_this_target = PropertiesDialog::instances.contains(target);

    PropertiesDialog *dialog;

    if (dialog_already_open_for_this_target) {
        // Focus already open dialog
        dialog = PropertiesDialog::instances[target];
        dialog->raise();
        dialog->setFocus();
    } else {
        // Make new dialog for this target
        dialog = new PropertiesDialog(ad, target);
        dialog->open();
    }

    hide_busy_indicator();

    if (dialog_is_new != nullptr) {
        *dialog_is_new = !dialog_already_open_for_this_target;
    }

    return dialog;
}

void PropertiesDialog::open_when_view_item_activated(QAbstractItemView *view, const int dn_role) {
    connect(
        view, &QAbstractItemView::doubleClicked,
        view,
        [view, dn_role](const QModelIndex &index) {
            AdInterface ad;
            if (ad_failed(ad, view)) {
                return;
            }

            const QString dn = index.data(dn_role).toString();

            open_for_target(ad, dn);
        });
}

PropertiesDialog::PropertiesDialog(AdInterface &ad, const QString &target_arg)
: QDialog() {
    ui = new Ui::PropertiesDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    target = target_arg;
    is_modified = false;

    PropertiesDialog::instances[target] = this;

    apply_button = ui->button_box->button(QDialogButtonBox::Apply);
    reset_button = ui->button_box->button(QDialogButtonBox::Reset);
    auto cancel_button = ui->button_box->button(QDialogButtonBox::Cancel);

    const QString title = [&]() {
        const QString target_name = dn_get_name(target_arg);

        if (!target_name.isEmpty()) {
            return QString(tr("%1 Properties")).arg(target_name);
        } else {
            return tr("Properties");
        }
    }();
    setWindowTitle(title);

    const AdObject object = ad.search_object(target);

    //
    // Create tabs
    //
    QWidget *general_tab = [&]() -> QWidget * {
        if (object.is_class(CLASS_USER)) {
            return new GeneralUserTab(&edit_list, this);
        } else if (object.is_class(CLASS_GROUP)) {
            return new GeneralGroupTab(&edit_list, this);
        } else if (object.is_class(CLASS_OU)) {
            return new GeneralOUTab(&edit_list, this);
        } else if (object.is_class(CLASS_COMPUTER)) {
            return new GeneralComputerTab(&edit_list, this);
        } else if (object.is_class(CLASS_GP_CONTAINER)) {
            return new GeneralPolicyTab(&edit_list, this);
        } else {
            return new GeneralOtherTab(&edit_list, this);
        }
    }();

    ui->tab_widget->add_tab(general_tab, tr("General"));

    const bool advanced_view_ON = settings_get_variant(SETTING_advanced_features).toBool();

    if (advanced_view_ON) {
        auto object_tab = new ObjectTab(&edit_list, this);
        attributes_tab = new AttributesTab(&edit_list, this);

        ui->tab_widget->add_tab(object_tab, tr("Object"));
        ui->tab_widget->add_tab(attributes_tab, tr("Attributes"));
    } else {
        attributes_tab = nullptr;
    }

    if (object.is_class(CLASS_USER)) {
        auto account_tab = new AccountTab(ad, &edit_list, this);
        auto address_tab = new AddressTab(&edit_list, this);
        auto organization_tab = new OrganizationTab(&edit_list, this);
        auto telephones_tab = new TelephonesTab(&edit_list, this);
        auto profile_tab = new ProfileTab(&edit_list, this);

        ui->tab_widget->add_tab(account_tab, tr("Account"));
        ui->tab_widget->add_tab(address_tab, tr("Address"));
        ui->tab_widget->add_tab(organization_tab, tr("Organization"));
        ui->tab_widget->add_tab(telephones_tab, tr("Telephones"));
        ui->tab_widget->add_tab(profile_tab, tr("Profile"));
    }
    if (object.is_class(CLASS_GROUP)) {
        auto members_tab = new MembershipTab(&edit_list, MembershipTabType_Members, this);
        ui->tab_widget->add_tab(members_tab, tr("Members"));
    }
    if (object.is_class(CLASS_USER) || object.is_class(CLASS_COMPUTER)) {
        auto member_of_tab = new MembershipTab(&edit_list, MembershipTabType_MemberOf, this);
        ui->tab_widget->add_tab(member_of_tab, tr("Member of"));
    }

    if (object.is_class(CLASS_OU) || object.is_class(CLASS_COMPUTER)) {
        auto managed_by_tab = new ManagedByTab(&edit_list, this);
        ui->tab_widget->add_tab(managed_by_tab, tr("Managed by"));
    }

    if (object.is_class(CLASS_OU) || object.is_class(CLASS_DOMAIN)) {
        auto group_policy_tab = new GroupPolicyTab(&edit_list, this);
        ui->tab_widget->add_tab(group_policy_tab, tr("Group policy"));
    }

    if (object.is_class(CLASS_COMPUTER)) {
        auto os_tab = new OSTab(&edit_list, this);
        auto delegation_tab = new DelegationTab(&edit_list, this);

        ui->tab_widget->add_tab(os_tab, tr("Operating System"));
        ui->tab_widget->add_tab(delegation_tab, tr("Delegation"));

        const bool laps_enabled = [&]() {
            const QList<QString> attribute_list = object.attributes();
            const bool out = (attribute_list.contains(ATTRIBUTE_LAPS_PASSWORD) && attribute_list.contains(ATTRIBUTE_LAPS_EXPIRATION));

            return out;
        }();

        if (laps_enabled) {
            auto laps_tab = new LAPSTab(&edit_list, this);
            ui->tab_widget->add_tab(laps_tab, tr("LAPS"));
        }
    }

    const bool need_security_tab = object.attributes().contains(ATTRIBUTE_SECURITY_DESCRIPTOR);
    if (need_security_tab && advanced_view_ON) {
        auto security_tab = new SecurityTab(&edit_list, this);
        ui->tab_widget->add_tab(security_tab, tr("Security"));
    }

    for (AttributeEdit *edit : edit_list) {
        connect(
            edit, &AttributeEdit::edited,
            [this, edit]() {
                const bool already_added = apply_list.contains(edit);

                if (!already_added) {
                    apply_list.append(edit);
                }
            });
    }

    reset_internal(ad, object);

    settings_setup_dialog_geometry(SETTING_properties_dialog_geometry, this);

    connect(
        apply_button, &QPushButton::clicked,
        this, &PropertiesDialog::apply);
    connect(
        reset_button, &QPushButton::clicked,
        this, &PropertiesDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &PropertiesDialog::reject);
    connect(
        ui->tab_widget, &TabWidget::current_changed,
        this, &PropertiesDialog::on_current_tab_changed);
}

PropertiesDialog::~PropertiesDialog() {
    delete ui;
}

void PropertiesDialog::on_current_tab_changed(QWidget *prev_tab, QWidget *new_tab) {
    const bool switching_to_or_from_attributes = (prev_tab == attributes_tab || new_tab == attributes_tab);
    const bool need_to_open_dialog = (switching_to_or_from_attributes && is_modified);
    if (!need_to_open_dialog) {
        return;
    }

    const PropertiesWarningType warning_type = [&]() {
        if (new_tab == attributes_tab) {
            return PropertiesWarningType_SwitchToAttributes;
        } else {
            return PropertiesWarningType_SwitchFromAttributes;
        }
    }();

    auto dialog = new PropertiesWarningDialog(warning_type, this);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this]() {
            AdInterface ad;
            if (ad_failed(ad, this)) {
                return;
            }

            apply_internal(ad);

            // NOTE: have to reset for attributes tab and other tabs
            // to load updates
            const AdObject object = ad.search_object(target);
            reset_internal(ad, object);
        });

    connect(
        dialog, &QDialog::rejected,
        this, &PropertiesDialog::reset);
}

void PropertiesDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const bool success = apply_internal(ad);

    if (success) {
        QDialog::accept();
    }
}

void PropertiesDialog::done(int r) {
    PropertiesDialog::instances.remove(target);

    QDialog::done(r);
}

void PropertiesDialog::apply() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    const bool apply_success = apply_internal(ad);

    ad.clear_messages();

    if (apply_success) {
        const AdObject object = ad.search_object(target);
        reset_internal(ad, object);
    }
}

void PropertiesDialog::reset() {
    AdInterface ad;
    if (ad_connected(ad, this)) {
        const AdObject object = ad.search_object(target);
        reset_internal(ad, object);
    }
}

bool PropertiesDialog::apply_internal(AdInterface &ad) {
    // NOTE: only verify and apply edits in the "apply
    // list", aka the edits that were edited
    const bool edits_verify_success = AttributeEdit::verify(apply_list, ad, target);

    if (!edits_verify_success) {
        return false;
    }

    show_busy_indicator();

    bool total_apply_success = true;

    const bool edits_apply_success = AttributeEdit::apply(apply_list, ad, target);
    if (!edits_apply_success) {
        total_apply_success = false;
    }

    g_status->display_ad_messages(ad, this);

    if (total_apply_success) {
        apply_button->setEnabled(false);
        reset_button->setEnabled(false);
        is_modified = false;
    }

    hide_busy_indicator();

    emit applied();

    return total_apply_success;
}

void PropertiesDialog::reset_internal(AdInterface &ad, const AdObject &object) {
    apply_list.clear();

    AttributeEdit::load(edit_list, ad, object);

    apply_button->setEnabled(false);
    reset_button->setEnabled(false);
    is_modified = false;

    g_status->display_ad_messages(ad, this);
}

void PropertiesDialog::on_edited() {
    apply_button->setEnabled(true);
    reset_button->setEnabled(true);
    is_modified = true;
}
