/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2020 rpf
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
#include <exception>
#include <ImageView.hpp>

#include <config.h>
#include "NomadApp.hpp"
#include "NomadWin.hpp"
#include "EditMode.hpp"

#undef NOMAD_DEBUG

NomadApp::NomadApp(int argc, char **argv)
: Gtk::Application(argc, argv, "de.pfeifer_syscon.nomad", Gio::ApplicationFlags::APPLICATION_HANDLES_OPEN)
, m_appSupport{"nomad.conf"}
, m_exec{argv[0]}
{
    m_appSupport.setApplication(this);
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::NomadApp" << std::endl;
    #endif
}


NomadWin*
NomadApp::createImageWindow(std::shared_ptr<Mode> mode)
{
    NomadWin* imageView{nullptr};
    auto builder = Gtk::Builder::create();
    try {
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::createImageWindow appl " << get_resource_base_path() << std::endl;
        #endif
        builder->add_from_resource(get_resource_base_path() + "/imageview.ui");
        builder->get_widget_derived("ImageView", imageView, mode, m_appSupport);
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::createImageWindow created" << std::endl;
        #endif
    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load imageview.ui: " << ex.what() << std::endl;
    }
    return imageView;
}

NomadWin*
NomadApp::getOrCreateImageWindow(std::shared_ptr<Mode> mode)
{
    // The application has been asked to open some files,
    // so let's open a new view for each one.
    NomadWin* appwindow = nullptr;
    auto windows = get_windows();
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::getOrCreateImageWindow wins " << windows.size() << std::endl;
    #endif
    if (windows.size() > 0) {
        appwindow = dynamic_cast<NomadWin*>(windows[0]);
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::getOrCreateImageWindow appwindow " << appwindow << std::endl;
        #endif
    }
    if (!appwindow) {
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::getOrCreateImageWindow create " << std::endl;
        #endif
        m_nomadAppWindow = createImageWindow(mode);
        add_window(*m_nomadAppWindow);
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::getOrCreateImageWindow showing " << m_nomadAppWindow << std::endl;
        #endif
        //m_nomadAppWindow->show_all(); this crashes (if used w/o parameter, but with parameter no windows is shown) !!!
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::getOrCreateImageWindow shown " << std::endl;
        #endif
    }
    else {
        #ifdef NOMAD_DEBUG
        std::cout << "NomadApp::getOrCreateImageWindow adding " << std::endl;
        #endif
        // need to add to mode
        m_nomadAppWindow = appwindow;
        if (m_nomadAppWindow->getMode()->join(mode)) {
            m_nomadAppWindow->refresh();
        }
        else {
            std::cout << "Unable to join mode" << std::endl;
        }
    }
    return m_nomadAppWindow;
}

void
NomadApp::on_activate()
{
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::on_activate" << std::endl;
    #endif
    // either on_activate is called (no args)
    auto mode = std::make_shared<EditMode>();
    NomadWin* imageView = getOrCreateImageWindow(mode); // on instance shoud be sufficent
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::on_activate created " << imageView << std::endl;
    #endif
    mode->setNomadWin(imageView);
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::on_activate show_all " << imageView << std::endl;
    #endif
    imageView->show_all();
}

void
NomadApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint)
{
    // or on_open is called (some args)
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::on_open files " << files.size() << std::endl;
    #endif
    try {
        if (!files.empty()) {
            std::vector<Glib::RefPtr<Gio::File>>  picts;
            for (auto file : files) {
                picts.push_back(file);
            }
            const int32_t front = 0;
            #ifdef NOMAD_DEBUG
            std::cout << "NomadApp::on_open picts " << picts.size() << std::endl;
            #endif
            // this opens a single dialog with paging ...
            auto mode = std::make_shared<PagingMode>(front, picts);
            auto win = getOrCreateImageWindow(mode);
            #ifdef NOMAD_DEBUG
            std::cout << "NomadApp::on_open show_all " << std::endl;
            #endif
            win->show_all();
        }
    }
    catch (const Glib::Error& ex) {
        std::cerr << "NomadApp::on_open() glib: " << ex.what() << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "NomadApp::on_open() except: " << ex.what() << std::endl;
    }
}

void
NomadApp::on_action_quit()
{
    m_nomadAppWindow->hide();

    // and we shoud delete appWindow if we were not going exit anyway
    // Not really necessary, when Gtk::Widget::hide() is called, unless
    // Gio::Application::hold() has been called without a corresponding call
    // to Gio::Application::release().
    quit();
}

void
NomadApp::on_action_about() {
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(get_resource_base_path() + "/abt-dlg.ui");
        auto dlgObj = builder->get_object("abt-dlg");
        auto dialog = Glib::RefPtr<Gtk::AboutDialog>::cast_dynamic(dlgObj);
        auto icon = Gdk::Pixbuf::create_from_resource(get_resource_base_path() + "/nomad.png");
        dialog->set_logo(icon);
        dialog->set_transient_for(*m_nomadAppWindow);
        dialog->show_all();
        dialog->run();
        dialog->hide();
    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load about-dialog: " << ex.what() << std::endl;
    }
}

void
NomadApp::on_action_help() {
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(get_resource_base_path() + "/help-dlg.ui");
        auto dlgObj = builder->get_object("help-dlg");
        auto dialog = Glib::RefPtr<Gtk::Dialog>::cast_dynamic(dlgObj);
        auto textObj = builder->get_object("text");
        auto text = Glib::RefPtr<Gtk::TextView>::cast_dynamic(textObj);

        std::string fullPath = g_canonicalize_filename(m_exec.c_str(), Glib::get_current_dir().c_str());
        Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(fullPath);
        auto dist_dir = f->get_parent()->get_parent();
        // this file identifies the development dir, beside executable
        auto readme = Glib::build_filename(dist_dir->get_path(), "README");
        if (!Glib::file_test(readme, Glib::FileTest::FILE_TEST_IS_REGULAR)) {
            // alternative search in distribution
            Glib::RefPtr<Gio::File> packageData = Gio::File::create_for_path(PACKAGE_DATA_DIR); // see Makefile.am
            std::string base_name = packageData->get_basename();
            readme = Glib::build_filename(packageData->get_parent()->get_path(), "doc", base_name, "README");
        }
        if (Glib::file_test(readme, Glib::FileTest::FILE_TEST_IS_REGULAR)) {
            Glib::ustring readmeText = Glib::file_get_contents(readme);
            text->get_buffer()->set_text(readmeText);
        } else {
            text->get_buffer()->set_text(Glib::ustring::sprintf("The README file %s coud not be located.\n", readme));
        }

        dialog->set_transient_for(*m_nomadAppWindow);
        dialog->show_all();
        dialog->run();
        dialog->hide();
    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load menubar: " << ex.what() << std::endl;
    }
}

Glib::ustring
NomadApp::get_exec_path()
{
    return m_exec;
}

void
NomadApp::on_startup()
{
    #ifdef NOMAD_DEBUG
    std::cout << "NomadApp::on_startup" << std::endl;
    #endif
    Gtk::Application::on_startup();

    add_action("quit", sigc::mem_fun(*this, &NomadApp::on_action_quit));
    set_accel_for_action("app.quit", "<Ctrl>Q");
    add_action("about", sigc::mem_fun(*this, &NomadApp::on_action_about));
    add_action("help", sigc::mem_fun(*this, &NomadApp::on_action_help));

    m_builder = Gtk::Builder::create();
    try {
        m_builder->add_from_resource(get_resource_base_path() + "/app-menu.ui");
        auto menuObj = m_builder->get_object("menubar");
        auto menuBar = Glib::RefPtr<Gio::Menu>::cast_dynamic(menuObj);
        if (menuBar) {
            set_menubar(menuBar);
        }
        else {
            std::cerr << "Cound not find/cast menubar!" << std::endl;
        }
    }
    catch (const Glib::FileError& ex) {
        std::cerr << "Unable to load menubar: " << ex.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");      // make locale dependent
    NomadApp app(argc, argv);

    return app.run();
}