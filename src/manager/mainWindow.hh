#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtkmm.h>
#include <giomm.h>
#include <glibmm.h>
#include <iostream>
#include "myarea.hh"
#include "proxy.hh"

class MainWindow
{
protected:
    Gtk::HeaderBar *headerBar = nullptr;
    Gtk::Button *left = nullptr;
    Gtk::Button *rotate = nullptr;
    Gtk::SearchEntry *searchEntry = nullptr;
    Gtk::AppChooserButton *appChooserButton = nullptr;
    Gtk::Box *box = nullptr;
    Gtk::Grid grid;
    Gtk::ScrolledWindow scrolledWindow;
    MyArea area;

    const Glib::RefPtr<Gtk::Builder> refBuilder;
    Glib::ustring mainWindowGlade;

public:
    MainWindow();
    virtual ~MainWindow();
    void load_scrolled_window(); 
    void load_single_image(Glib::ustring image);

	Gtk::ApplicationWindow *appWindow = nullptr;

    // signal handlers
    void on_my_custom_item_activated(const Glib::ustring& item_name);
    bool on_eventbox_button_press(GdkEventButton* button, Glib::ustring file/*Glib::RefPtr<Gdk::Pixbuf>& pimage*/);
	void on_back_button_clicked();    
	void on_rotate_button_clicked(Glib::RefPtr<Gdk::Pixbuf> m_Pimage); 
};

// destructor here
MainWindow::~MainWindow()
{
	std::cout << "MainWindow destructor " << std::endl;
}

// constructor here
MainWindow::MainWindow() : mainWindowGlade("src/manager/gui.glade")
{

    auto refBuilder = Gtk::Builder::create();

    try
    {
        refBuilder->add_from_file(mainWindowGlade);
    }
    catch(const Glib::FileError& ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
    }
    catch(const Glib::MarkupError& ex)
    {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
    }
    catch(const Gtk::BuilderError& ex)
    {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
    }

	refBuilder->get_widget("applicationWindow", appWindow);
    refBuilder->get_widget("headerBar", headerBar);
    refBuilder->get_widget("left", left);
    refBuilder->get_widget("rotate", rotate);
    refBuilder->get_widget("searchEntry", searchEntry);
    refBuilder->get_widget("appChooserButton", appChooserButton);
    refBuilder->get_widget("box", box);
    
    left->set_sensitive(false);
    rotate->set_sensitive(false);
    left->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_back_button_clicked));
    
    appWindow->set_titlebar(*headerBar);
    appWindow->set_default_size(1110, 700);
    appWindow->add(*box);

    Glib::RefPtr<Gio::Icon> cameraIcon = Gio::Icon::create("camera-photo");
    Glib::RefPtr<Gio::Icon> imageIcon = Gio::Icon::create("image-x-generic");
    Glib::RefPtr<Gio::Icon> icon = Gio::Icon::create("icon");

    appChooserButton->append_custom_item((const gchar *)"photosButton", (const gchar *)" Photos ", imageIcon);
    appChooserButton->append_custom_item((const gchar *)"peopleButton", (const gchar *)" People ", icon);
    appChooserButton->append_custom_item((const gchar *)"eventsButton", (const gchar *)" Events ", icon);
    appChooserButton->append_custom_item((const gchar *)"tagsButton", (const gchar *)" Tags ", icon);
    appChooserButton->append_custom_item((const gchar *)"cameraButton", (const gchar *)" Camera ", cameraIcon);
    appChooserButton->append_custom_item((const gchar *)"placesButton", (const gchar *)" Places ", icon);

    appChooserButton->signal_custom_item_activated().connect(sigc::mem_fun(*this, &MainWindow::on_my_custom_item_activated));

    appChooserButton->set_active_custom_item("photosButton");
    //show_all_children(true);
}

// member functions from here
void MainWindow::load_scrolled_window()
{
	ServerProxy sp;
	auto listOfFiles = sp.getPhotos();	
		    
    grid.set_column_spacing(10);
    grid.set_row_spacing(10);

    scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledWindow.remove_with_viewport();												/* right here */
    scrolledWindow.add(grid);
    
    if (!(box == nullptr)){
    	auto children = box->get_children();
    	for (auto child : children)
    		box->remove(*child);
    }
    box->pack_start(scrolledWindow);

    int row = 0;
	for (unsigned int i = 0; i < listOfFiles.size(); i++)
	{
		Glib::RefPtr<Gdk::Pixbuf> ptr_image;
		try
		{
			ptr_image = Gdk::Pixbuf::create_from_file(listOfFiles[i]);
		}
		catch (const Gio::ResourceError& ex)
		{
			std::cerr << "ResourceError: " << ex.what() << std::endl;
		}
		catch (const Gdk::PixbufError& ex)
		{
			std::cerr << "PixbufError: " << ex.what() << std::endl;
		}

		Gtk::Image* image = Gtk::manage(new Gtk::Image());
		Gtk::EventBox* eventBox = Gtk::manage(new Gtk::EventBox());

		eventBox->set_events(Gdk::BUTTON_PRESS_MASK);
		eventBox->signal_button_press_event().connect(
	                                               sigc::bind<Glib::ustring>(
                                                                sigc::mem_fun(*this,
                                                                 	&MainWindow::on_eventbox_button_press), listOfFiles[i]));
        
        eventBox->set_margin_top(10);
        eventBox->set_margin_bottom(10);
        eventBox->set_margin_left(10);
        eventBox->set_margin_right(10);
        eventBox->set_focus_on_click(true);

		image->set(ptr_image->scale_simple(350, 200, Gdk::INTERP_HYPER));
        eventBox->add(*image);

		if (i % 3 == 0)
			row++;

		grid.attach(*eventBox, i % 3, row, 1, 1);
		}
        std::cout << "Ok upto here " << std::endl;
        appWindow->show_all();
}

void MainWindow::load_single_image(Glib::ustring image)
{
	this->area.set_image(image);
	box->pack_start(area);
	appWindow->show_all();
}


void MainWindow::on_back_button_clicked()
{
	// TODO: review editing of titlebar
	headerBar->add(*appChooserButton);
	headerBar->remove(*left);
	headerBar->remove(*rotate);
	
	left->set_sensitive(false);
	rotate->set_sensitive(false);
	
	this->load_scrolled_window();
	headerBar->set_title("Photo Manager");
}


// signal handlers from here
void MainWindow::on_rotate_button_clicked(Glib::RefPtr<Gdk::Pixbuf> m_Pimage)
{
	m_Pimage = m_Pimage->rotate_simple((Gdk::PixbufRotation)90);
	std::cout << "Rotate button clicked " << std::endl;
}


void MainWindow::on_my_custom_item_activated(const Glib::ustring& item_name)
{
    if (item_name.compare("photosButton") == 0)
    {
	    // TODO: review editing of titlebar
    	headerBar->remove(*left);
    	headerBar->remove(*rotate);
		this->load_scrolled_window();
    }
    else if (item_name.compare("peopleButton"))
    {

    }
    else if (item_name.compare("eventsButton"))
    {

    }
    else if (item_name.compare("tagsButton"))
    {

    }
    else if (item_name.compare("cameraButton"))
    {

    }
    else if (item_name.compare("placesButton"))
    {

    }
}

bool MainWindow::on_eventbox_button_press(GdkEventButton* button, Glib::ustring image)
{
    if (button->type == GDK_DOUBLE_BUTTON_PRESS)
    {
    	// TODO: review editing of titlebar;
    	headerBar->add(*left);
    	headerBar->add(*rotate);
    	left->set_sensitive(true);
    	rotate->set_sensitive(true);
    	headerBar->remove(*appChooserButton);
    	
    	auto children = box->get_children();
    	for (auto child : children)
 			box->remove(*child);			
 			
    	std::cout << "removed scrolled window " << std::endl;
        this->load_single_image(image);
        return true;
    }
    else if (button->type == GDK_BUTTON_PRESS)
    {
        std::cout << "Single click handled " << std::endl;
        return true;
    }
}

#endif
