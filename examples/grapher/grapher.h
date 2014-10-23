#ifndef _GRAPHER_H
#define _GRAPHER_H
#include <deque>
#include <gtk--/window.h>

class Plot;

class Grapher : public Gtk::Window
{
public:
  Grapher ();
  ~Grapher ();

protected:
  virtual int delete_event_impl (GdkEventAny*);

private:
  std::deque<Plot*> plots;
  Plot* curPlot;
};

#endif //_GRAPHER_H
