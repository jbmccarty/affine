#include <gdk--/gc.h>
#include <gtk--/box.h>
#include <gtk--/checkbutton.h>
#include <gtk--/drawingarea.h>
#include <gtk--/entry.h>
#include <gtk--/frame.h>
#include <gtk--/label.h>
#include <gtk--/main.h>
#include <gtk--/menu.h>
#include <gtk--/menubar.h>
#include <gtk--/notebook.h>
#include <gtk--/radiobutton.h>
#include <gtk--/spinbutton.h>
#include <gtk--/style.h>
#include <gtk--/window.h>
#include <gdk/gdkx.h>
#include <iostream> //FIXME debug
#include "grapher.h"
//#include "plot.h"

Grapher::Grapher ()
{
  //set up left pane
//  Gtk::DrawingArea* plotArea = new Gtk::DrawingArea;
//  plotArea->size(350, 350);
  //connect mouse signals:
  //left-drag to zoom in, right-drag to zoom out, display coordinates of
  //crosshair cursor in bottom

  //set up main window
  Gtk::VBox* mainBox = new Gtk::VBox;

  //set up menu
  Gtk::MenuBar* menuBar = new Gtk::MenuBar;
  using Gtk::Menu_Helpers::MenuElem;

  //file menu
  Gtk::Menu* menu = new Gtk::Menu;
  menu->items().push_back(MenuElem("_New")); //connect callback
  menu->items().push_back(MenuElem("_Open")); //connect callback
  menu->items().push_back(MenuElem("_Save")); //connect callback
  menu->items().back()->set_sensitive(false);
  menu->items().push_back(MenuElem("_Close")); //connect callback
  menu->items().back()->set_sensitive(false);
  menu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
  menu->items().push_back(MenuElem("_Quit")); //connect callback
  menuBar->items().push_back(MenuElem("_File", "<alt>f", *manage(menu)));

  //view menu
  menu = new Gtk::Menu;
  menuBar->items().push_back(MenuElem("_View", "<alt>v", *manage(menu)));
  menuBar->items().back()->set_sensitive(false);
  
  //add menu to window
  mainBox->pack_start(*manage(menuBar));
  
  //set up function tab
  Gtk::VBox* funcTab = new Gtk::VBox;
  funcTab->set_spacing(5);
  funcTab->set_border_width(5);
  
  //function entry
  Gtk::HBox* hbox = new Gtk::HBox;
  hbox->set_spacing(5);
  Gtk::Label* label = new Gtk::Label("f(x, y) =");
  hbox->pack_start(*manage(label), false);
  Gtk::Entry* entry = new Gtk::Entry; //attach signals
  hbox->pack_start(*manage(entry));
  funcTab->pack_start(*manage(hbox), false);

  //domain settings
  Gtk::VBox* domBox = new Gtk::VBox;
  domBox->set_border_width(5);
  domBox->set_spacing(5);
  
  //x domain
  hbox = new Gtk::HBox;
  label = new Gtk::Label("x: [");
  hbox->pack_start(*manage(label), false);
  Gtk::SpinButton* spBut = new Gtk::SpinButton; //attach callbacks
  hbox->pack_start(*manage(spBut));
  label = new Gtk::Label("..");
  hbox->pack_start(*manage(label), false);
  spBut = new Gtk::SpinButton; //attach callbacks
  hbox->pack_start(*manage(spBut));
  label = new Gtk::Label("]");
  hbox->pack_start(*manage(label), false);
  domBox->pack_start(*manage(hbox), false);

  //y domain
  hbox = new Gtk::HBox;
  label = new Gtk::Label("y: [");
  hbox->pack_start(*manage(label), false);
  spBut = new Gtk::SpinButton; //attach callbacks
  hbox->pack_start(*manage(spBut));
  label = new Gtk::Label("..");
  hbox->pack_start(*manage(label), false);
  spBut = new Gtk::SpinButton; //attach callbacks
  hbox->pack_start(*manage(spBut));
  label = new Gtk::Label("]");
  hbox->pack_start(*manage(label), false);
  domBox->pack_start(*manage(hbox), false);

  //aspect ratio
  hbox = new Gtk::HBox;
  Gtk::CheckButton* ckBut =
    new Gtk::CheckButton("Preserve aspect ratio"); //connect callbacks
  hbox->pack_start(*manage(ckBut), false);
  spBut = new Gtk::SpinButton; //attach callbacks and save pointer to attach
  //it to checkbutton
  hbox->pack_start(*manage(spBut));
  label = new Gtk::Label(":1");
  hbox->pack_start(*manage(label), false);
  domBox->pack_start(*manage(hbox), false);

  //add domain frame to function tab
  Gtk::Frame* domFrame = new Gtk::Frame("Domain");
  domFrame->add(*manage(domBox));
  funcTab->pack_start(*manage(domFrame), false);

  //plot label
  hbox = new Gtk::HBox;
  hbox->set_spacing(5);
  label = new Gtk::Label("Label");
  hbox->pack_start(*manage(label), false);
  entry = new Gtk::Entry; //attach callbacks
  hbox->pack_start(*manage(entry));
  funcTab->pack_start(*manage(hbox), false);
  
  //set up display options tab
  Gtk::VBox* dispTab = new Gtk::VBox;
  dispTab->set_spacing(5);
  dispTab->set_border_width(5);

  //cell dimension frame
  Gtk::VBox* dimBox = new Gtk::VBox;
  dimBox->set_spacing(5);
  dimBox->set_border_width(5);

  //auto cell size
  Gtk::RadioButton_Helpers::Group g;
  Gtk::RadioButton* rdBut = new Gtk::RadioButton(g, "Auto"); //add callback
  rdBut->set_active(true);
  dimBox->pack_start(*manage(rdBut), false);
  
  //fixed cell size
  hbox = new Gtk::HBox;
  rdBut = new Gtk::RadioButton(g, "Fixed"); //add callback
  hbox->pack_start(*manage(rdBut), false);
  spBut = new Gtk::SpinButton;
  spBut->set_sensitive(false);
  hbox->pack_start(*manage(spBut), false);
  //...
  dimBox->pack_start(*manage(hbox), false);

  //add dimension frame to dispoption tab
  Gtk::Frame* dimFrame = new Gtk::Frame("Maximum Cell Dimension");
  dimFrame->add(*manage(dimBox));
  dispTab->pack_start(*manage(dimFrame), false);
  
  //set up option tabs
  Gtk::Notebook* optionTabs = new Gtk::Notebook;
  optionTabs->add(*manage(funcTab));
  optionTabs->pages().back()->set_tab_text("Grids and Axes");
  optionTabs->add(*manage(dispTab));
  optionTabs->pages().back()->set_tab_text("Line Segments");
//  optionTabs->set_sensitive(false);
  optionTabs->set_border_width(5);
  mainBox->pack_start(*manage(optionTabs));

  //plot button
  Gtk::Button* lbBut = new Gtk::Button("Plot"); //connect callbacks
  lbBut->set_sensitive(false);
  mainBox->pack_start(*manage(lbBut));
  
  //put widgets in window
  add(*manage(mainBox));

  //display app
  set_policy(false, false, true);
  show_all();

//  std::cout << GDK_WINDOW_XWINDOW(GTK_WIDGET(plotArea->gtkobj())->window) << "\n";
  //GDK_WINDOW_XWINDOW(GTK_WIDGET(plotArea->gtkobj())->window)
  //gets the XID of the window to pass to ghostscript
  //problem: gs doesn't notice window resize events
}

Grapher::~Grapher ()
{}

int Grapher::delete_event_impl (GdkEventAny*)
{
  Gtk::Main::quit();
}
