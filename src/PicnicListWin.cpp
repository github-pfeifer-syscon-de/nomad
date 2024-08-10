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

#include <iostream>
#include <thread>
#include <future>

#include <gtkmm.h>
#include <KeyConfig.hpp>

#include "PicnicListWin.hpp"
#include "FileTree.hpp"
#include "ImageView.hpp"
#include "FileTreeView.hpp"


PicnicListWin::PicnicListWin(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder,
	Gtk::Application *picnicApp)
: Gtk::ApplicationWindow(cobject)
, m_fileTreeView{nullptr}
, m_config{std::make_shared<KeyConfig>("picnic.conf")}
, m_appSupport{m_config}
{
    auto pix = Gdk::Pixbuf::create_from_resource(picnicApp->get_resource_base_path() + "/picnic.png");
    if (pix) {
        set_icon(pix);
    }
    m_appSupport.setApplication(picnicApp);
    m_appSupport.addWindow(this, CONF_PREFIX, 512, 512, CONF_GROUP);

	auto panedObj = builder->get_object("paned");
    m_paned = Glib::RefPtr<Gtk::Paned>::cast_dynamic(panedObj);
	if (m_config->hasKey(CONF_GROUP, DIV)) {
		m_paned->set_position(m_config->getInteger(CONF_GROUP, DIV));
	}

	FileTreeView *fileTreeView;
    builder->get_widget_derived("treeView", fileTreeView);
	m_fileTreeView = Glib::RefPtr<FileTreeView>(fileTreeView);
	m_fileTreeView->append_column("Name", FileTree::m_treeColumns.m_name);
	m_fileTree = FileTree::create(*(fileTreeView));
	m_fileTreeView->set_model(m_fileTree);
	m_fileTreeView->get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &PicnicListWin::on_select_tree));

	m_listStore = ImageIconList::create();
    auto displ = get_display();
    auto iconTheme = Gtk::IconTheme::get_for_screen(displ->get_default_screen());
	m_listStore->setIconTheme(iconTheme);
	m_listStore->set_sort_column(ImageIconList::m_imageListColumns.m_name, Gtk::SortType::SORT_ASCENDING);
	auto iconViewObj = builder->get_object("iconView");
    m_iconView = Glib::RefPtr<Gtk::IconView>::cast_dynamic(iconViewObj);
	m_iconView->set_model(m_listStore);

	m_iconView->set_column_spacing(2);		// less spacey but more packed
	m_iconView->property_item_padding() = 2;
	m_iconView->property_margin() = 2;
	//Glib::PropertyProxy_ReadOnly<Glib::RefPtr<Gtk::CellArea>> cellArea = m_iconView->property_cell_area(); // renderer but r/o
	//Glib::RefPtr<Gtk::CellAreaBox> cellAreaBox = Glib::RefPtr<Gtk::CellAreaBox>::cast_dynamic(cellArea.get_value());
	//if (cellAreaBox) {
	//	cellAreaBox->set_spacing(0);	// no visual change
	//}
	//else {
	//	std::cout << "cellAreaBox " << (cellAreaBox ? "valid" : "invalid") << std::endl;
	//}
	m_iconView->set_text_column(ImageIconList::m_imageListColumns.m_shortName);
	m_iconView->set_tooltip_column(0);	// use full text as tooltip
	m_iconView->set_pixbuf_column(ImageIconList::m_imageListColumns.m_icon);
	m_iconView->signal_item_activated().connect(
		sigc::mem_fun(*this, &PicnicListWin::on_activated));
    add_action("open", sigc::mem_fun(*this, &PicnicListWin::on_action_open));
    add_action("view", sigc::mem_fun(*this, &PicnicListWin::on_action_view));
    add_action("about", sigc::mem_fun(*this, &PicnicListWin::on_action_about));

    //show_all_children();

	Glib::RefPtr<Gio::File> fDir = getOpenDir();
	showDir(fDir);
}


void
PicnicListWin::showDir(Glib::RefPtr<Gio::File>& fDir)
{
	Gtk::TreePath path = m_fileTree->load(fDir);
	m_fileTreeView->expand_to_path(path);
	m_fileTreeView->get_selection()->select(path);
}

void
PicnicListWin::on_select_tree()
{
	auto iter = m_fileTreeView->get_selection()->get_selected();
	auto node = iter->get_value(FileTree::m_treeColumns.m_node);
	Glib::RefPtr<Gio::File> fDir = node->getFile();
	m_listStore->load(fDir);
}

void
PicnicListWin::on_hide()
{
	m_fileTree->unref();
	m_config->setInteger(CONF_GROUP, DIV, m_paned->get_position());
    m_appSupport.removeWindow(this, CONF_PREFIX, CONF_GROUP);
}

Glib::RefPtr<Gio::File>
PicnicListWin::getOpenDir()
{
    if (m_config->hasKey(PicnicListWin::CONF_GROUP, CONF_PATH)) {
        Glib::ustring pics = m_config->getString(PicnicListWin::CONF_GROUP, CONF_PATH);
        Glib::RefPtr<Gio::File> fPics = Gio::File::create_for_path(pics);
        if (fPics
         && fPics->query_exists()) {
            return fPics;
        }
    }
    Glib::ustring pics = Glib::get_home_dir();        // just start from somewhere
    Glib::RefPtr<Gio::File> fPics = Gio::File::create_for_path(pics);
    m_config->setString(PicnicListWin::CONF_GROUP, CONF_PATH, pics);
    m_appSupport.saveConfig();
    return fPics;
}

void
PicnicListWin::on_activated(const Gtk::TreeModel::Path& path)
{
	auto iter = m_listStore->get_iter(path);
	auto row = *iter;
	Glib::RefPtr<Gio::File> selectedFile = row.get_value(ImageIconList::m_imageListColumns.m_file);
    int32_t front = 0;
	std::vector<Glib::RefPtr<Gio::File>> files = m_listStore->getFiles();
    std::vector<Glib::RefPtr<Gio::File>> picts;
	auto cancellable = Gio::Cancellable::create();
	for (Glib::RefPtr<Gio::File> f : files) {
		auto info = f->query_info(cancellable, "standard::*");
		auto mime = info->get_content_type();	// use also extended type
		if (ImageView<Gtk::Window,GtkWindow>::is_mime_gdk_readable(mime)) {
			if (selectedFile && selectedFile == f) {
				front = picts.size();
			}
			picts.push_back(f);
		}
	}
	if (!picts.empty()) {
		ImageView<Gtk::Window,GtkWindow>::showView(front, picts, m_appSupport);
	}
	else {
		m_appSupport.showError("No displayable images", Gtk::MessageType::MESSAGE_INFO);
	}
	// use the follwing if no pic file selected ?
	// Glib::RefPtr<AppInfo> query_default_handler(const Glib::RefPtr<Cancellable>& cancellable);


}

void
PicnicListWin::on_action_view()
{
	auto selected = m_iconView->get_selected_items();
	Gtk::TreeModel::Path path;
	if (!selected.empty()) {
		path = selected.front();
	}
	else {
		// use first seems most reasonable
		path = m_listStore->get_path(m_listStore->children().begin());
	}

	on_activated(path);
}

void
PicnicListWin::on_action_open()
{
    Glib::RefPtr<Gio::File> openDir = getOpenDir();
    auto dialog = new Gtk::FileChooserDialog("Please choose a directory",
          Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog->set_modal(true);
    dialog->set_current_folder(openDir->get_path());
    dialog->signal_response().connect(sigc::bind(
        sigc::mem_fun(this, &PicnicListWin::on_file_dialog_response), dialog));

    m_appSupport.addDialogYesNo(*dialog);

  //Show the dialog and wait for a user response:
  dialog->show();
}

void
PicnicListWin::on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog)
{
    switch (response_id)
    {
    case Gtk::ResponseType::RESPONSE_OK:
        auto filename = dialog->get_file()->get_path();
        auto fPics = Gio::File::create_for_path(filename);
        if (fPics->query_exists()) {
            m_listStore->load(fPics);
            m_config->setString(PicnicListWin::CONF_GROUP, CONF_PATH, filename);
            m_appSupport.saveConfig();
        }
        break;
    //case Gtk::ResponseType::RESPONSE_CANCEL:
    //    break;
    }
    delete dialog;
}



void
PicnicListWin::on_action_about()
{
    auto refBuilder = Gtk::Builder::create();
    try {
		auto appl = m_appSupport.getApplication();
        refBuilder->add_from_resource(
            appl->get_resource_base_path() + "/abt-dlg.ui");
        auto object = refBuilder->get_object("abt-dlg");
        auto abtdlg = Glib::RefPtr<Gtk::AboutDialog>::cast_dynamic(object);
        if (abtdlg) {
            Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(
                    appl->get_resource_base_path() +"/picnic.png");
            abtdlg->set_logo(pix);
            abtdlg->set_transient_for(*this);
            abtdlg->run();
            abtdlg->hide();
        } else
            std::cerr << "PicnicListWin::on_action_about(): No \"abt-dlg\" object in abt-dlg.ui"
                << std::endl;
    } catch (const Glib::Error& ex) {
        std::cerr << "PicnicListWin::on_action_about(): " << ex.what() << std::endl;
    }

}

