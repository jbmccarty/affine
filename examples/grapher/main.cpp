#include <gtk--/main.h>
#include "grapher.h"

int main (int argc, char** argv)
{
  Gtk::Main m(&argc, &argv);
  Grapher g;
  m.run();
  return 0;
}
