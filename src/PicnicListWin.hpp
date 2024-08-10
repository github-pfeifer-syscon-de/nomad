/*
 * Copyright (C) 2021 rpf
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

#pragma once

#include "ApplicationSupport.hpp"
#include "ImageIconList.hpp"
#include "FileTree.hpp"

#include <memory>

class PicnicApp;
class FileTreeView;
class KeyConfig;

class PicnicListWin : public Gtk::ApplicationWindow {
public:
    PicnicListWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Gtk::Application *picnicApp);
    virtual ~PicnicListWin() = default;

    void on_action_open();
    void on_action_view();

    static constexpr auto CONF_GROUP = "picnic";

    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog);
    Glib::RefPtr<Gio::File> getOpenDir();
protected:
    void on_activated(const Gtk::TreeModel::Path& path);
    void on_select_tree();
    void showDir(Glib::RefPtr<Gio::File>& fDir);
    void on_hide();
    void on_action_about();
private:
    Glib::RefPtr<ImageIconList> m_listStore;
    Glib::RefPtr<Gtk::IconView> m_iconView;
    Glib::RefPtr<FileTree> m_fileTree;
    Glib::RefPtr<FileTreeView> m_fileTreeView;
    std::shared_ptr<KeyConfig> m_config;
    ApplicationSupport m_appSupport;
    Glib::RefPtr<Gtk::Paned> m_paned;

    static constexpr auto CONF_PREFIX= "win";
    static constexpr auto CONF_PATH = "path";
    static constexpr auto DIV = "div";

};

