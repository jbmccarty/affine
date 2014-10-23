#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <cmath>
#include <affine/affine.h>

typedef void (*Operation) (Interval& result, const Interval& x, const Interval& y);

inline void my_op (Interval& result, const Interval& x, const Interval& y)
{ //Define to preferred operation
  result = sin(x + 2.0 * sin(y)) - cos(y + 3.0 * cos(x));
}
char operation[] = "sin\\(x + 2sin\\(y\\)\\) = cos\\(y + 3cos\\(x\\)\\)";

namespace params {
  Interval x(-10.0, 10.0);
  Interval y(-10.0, 10.0);
  Real aspect_ratio = 1.0;
  Real max_thickness = 0.5;
  bool show_subdivision = false;
  std::ostream* output = &std::cout;

  void set_params (int argc, char** argv);
}

void error (const std::string& err)
{
  std::cerr << "implicit-interval: " << err << "\n";
  std::exit(1);
}

void print_usage ()
{
  std::cout << "Usage: implicit-interval [options] [--] FUNCTION\n\n"
    << "Options:\n"
    << "  -x XRANGE       Set x range (default " << params::x << ")\n"
    << "  -y YRANGE       Set y range (default " << params::y << ")\n"
    << "  -a RATIO        Set y:x aspect ratio (default "
      << params::aspect_ratio << ")\n"
    << "  -t THICKNESS    Set maximum line thickness in points (default "
      << params::max_thickness << ")\n"
    << "  -s              Show subdivision boundaries\n"
    << "  -o FILE         Output to FILE instead of stdout\n\n";
  std::exit(1);
}

void recurse (const Interval& x, const Interval& y);

int main (int argc, char** argv)
{
  using namespace params;
  set_params(argc, argv);

  FILE* date_prog = popen("/bin/date -R", "r"); // this should be replaced
  char date_string[100];                        // with ctime/strftime
  fgets(date_string, 100, date_prog);
  fclose(date_prog);

  Real abs_width = width(x);
  Real abs_height = width(y);
  Real adj_height = aspect_ratio * abs_height;
  Real width = 504, height = 504; //in points
  if (abs_width > adj_height)
    height = adj_height * width / abs_width;
  else
    width = abs_width * height / adj_height;

  *output << "%!PS-Adobe-3.0 EPSF-3.0\n"
    << "%%BoundingBox: 0 0 " << static_cast<int>(width + 10.5)
        << " " << static_cast<int>(height + 36.0) << "\n"
    << "%%Creator: Implicit Grapher 0.2\n"
    << "%%CreationDate: " << date_string
    << "gsave\n"
    << "/Times-Roman findfont 12 scalefont setfont\n"
    << "5 " << height + 22 << " moveto (Thickness: " << max_thickness
    << " points; x range: " << x << "; y range: " << y << ") show\n"
    << "5 " << height + 10 << " moveto (Operation: " << operation << ") show\n"
    << "0 setlinewidth 5 5 translate\n"
    
    << "/a {newpath 0 0 moveto 1 0 rlineto 0 1 rlineto -1 0 rlineto closepath}\n"
      << "    bind def\n"
      //proc to draw a line
    << "/b {0 1 0 setrgbcolor a fill} bind def\n" //contains zero
    << "/c {1 0 0 setrgbcolor a fill} bind def\n" //domain error
    << "/d {0 0 1 setrgbcolor a fill} bind def\n" //overflow
    << "/e {gsave 0.5 1 scale} bind def\n" //left recursion box
    << "/f {1 0 translate} bind def\n" //right
    << "/g {gsave 1 0.5 scale} bind def\n" //bottom
    << "/h {0 1 translate} bind def\n" //top
    << "/i {grestore} bind def\n" //return from recursion

    << width << " " << height << " scale\n" //adjust scale
    << "a stroke a clip\n"; //draw enclosing box and clip to it
   

  params::max_thickness *= abs_width / width;
  recurse(x, y);
  *output << "i\n"
    << "showpage\n"
    << "%%EOF\n";
  if (output != &std::cout)
    delete output;
  return 0;
}

#define make_sstream(input) (is.clear(), is.str(input), is)

void params::set_params (int argc, char** argv)
{
  if (argc <= 1)
    print_usage();
  std::istringstream is;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "-x")) {
      ++i;
      if ((argc <= i) || !(make_sstream(argv[i]) >> x))
        error("option -x requires an interval argument");
    } else if (!std::strcmp(argv[i], "-y")) {
      ++i;
      if ((argc <= i) || !(make_sstream(argv[i]) >> y))
        error("option -y requires an interval argument");
    } else if (!std::strcmp(argv[i], "-a")) {
      ++i;
      if ((argc <= i) || !(make_sstream(argv[i]) >> aspect_ratio))
        error("option -a requires a numeric argument");
    } else if (!std::strcmp(argv[i], "-t")) {
      ++i;
      if ((argc <= i) || !(make_sstream(argv[i]) >> max_thickness))
        error("option -t requires a numeric argument");
    } else if (!std::strcmp(argv[i], "-s")) {
      show_subdivision = true;
    } else if (!std::strcmp(argv[i], "-o")) {
      ++i;
      std::string filename;
      if ((argc <= i) || !(make_sstream(argv[i]) >> filename))
        error("option -o requires a filename argument");
      std::ofstream* file = new std::ofstream(argv[i]);
      if (!file->is_open()) {
        delete file;
        error("couldn't open file for writing: " + filename);
      }
      output = file;
    } else if (!std::strcmp(argv[i], "--")) {
      ++i;
      break;
    } else
      break;
  }
  //should set function here
}

void recurse (const Interval& x, const Interval& y)
{
  using params::output;
  bool domerr = false;
  bool overflow = false;
  bool contains_zero = false;

  Real xwidth = width(x), ywidth = width(y);

  if (params::show_subdivision)
    *output << "0 setgray a stroke\n";

  {
    Interval z;
    try { my_op(z, x, y); }
    catch (Interval::DomainError) {
      domerr = true;
    }

    if (!std::isfinite(z.lo()) || !std::isfinite(z.hi())) {
      domerr = false;
      overflow = true;
    } else {
      if (!domerr && ((z.lo() <= 0.0) && (z.hi() >= 0.0)))
        contains_zero = true;
    }
  }

  if (!domerr && !overflow && !contains_zero) {
  } else if (std::min(xwidth, ywidth) <= params::max_thickness) {
    //don't recurse any further
    if (contains_zero)
      *output << "b\n";
    else if (domerr) //domain error
      *output << "c\n";
    else //overflow
      *output << "d\n";
  } else { //recurse
    if (xwidth >= ywidth) {
      Real center = x.lo() + 0.5 * xwidth;
      *output << "e\n";
      recurse(Interval(x.lo(), center), y);
      *output << "f\n";
      recurse(Interval(center, x.hi()), y);
    } else {
      Real center = y.lo() + 0.5 * ywidth;
      *output << "g\n";
      recurse(x, Interval(y.lo(), center));
      *output << "h\n";
      recurse(x, Interval(center, y.hi()));
    }
    *output << "i\n";
  }
}
