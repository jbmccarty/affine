#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <affine/affine.h>
#include "evaluator.h"

namespace globals {
  const Affine e_l(0.0, 1.0), e_t(0.0, 1.0);
  //basis vectors for length and thickness
  evaluator::var_map vars; //variables used by expression
  Affine *x, *y; //pointers into vars
}

namespace params {
  Interval x(-10.0, 10.0);
  Interval y(-10.0, 10.0);
  Real aspect_ratio = 1.0;
  Real max_thickness = 0.5;
  bool constant_thickness = true;
  bool gradient_subdivision = true;
  bool show_subdivision = false;
  bool show_tangent = false;
  Real tangent_length;
  std::ostream* output = &std::cout;
  std::string operation;
  evaluator::node expression;

  void set_params (int argc, char** argv);
}

void error (const std::string& err)
{
  std::cerr << "implicit-affine: " << err << "\n";
  std::exit(1);
}

void print_usage (char* argv0)
{
  std::cout << "Usage: " << argv0 << " [options] [--] FUNCTION\n\n"
    << "Options:\n"
    << "  -x XRANGE       Set x range (default " << params::x << ")\n"
    << "  -y YRANGE       Set y range (default " << params::y << ")\n"
    << "  -a RATIO        Set y:x aspect ratio (default "
      << params::aspect_ratio << ")\n"
    << "  -t THICKNESS    Set maximum line thickness in points (default "
      << params::max_thickness << ")\n"
    << "  -c              Draw each line using its calculated thickness\n"
    << "  -Xg             Don't use gradient-based subdivision (implies -c)\n"
    << "  -s              Show subdivision boundaries\n"
    << "  -d              Draw tangent lines (experimental)\n"
    << "  -o FILE         Output to FILE instead of stdout\n\n";
  std::exit(1);
}

void recurse (Real x0, Real y0, Real xinc, Real yinc, Real thickness);

int main (int argc, char** argv)
{
  using namespace params;
  set_params(argc, argv);

  time_t date = std::time(0);
  char date_string[100];                        // with ctime/strftime
  std::strftime(date_string, 100, "%a,  %d  %b %Y %H:%M:%S %z",
      std::localtime(&date)); // XXX: %z is a GNU extension

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
    << "\ngsave\n"
    << "/Times-Roman findfont 12 scalefont setfont\n"
    << "5 " << height + 22 << " moveto (Thickness: " << max_thickness
    << " points; x range: " << x << "; y range: " << y << ") show\n"
    << "5 " << height + 10 << " moveto (Operation: " << operation << ") show\n"
    << "0 setlinewidth 5 5 translate\n"
    
    // draw enclosing box, and clip to it
    << "newpath 0 0 moveto " << width << " 0 lineto " << width << " "
    << height << " lineto 0 " << height
    << " lineto closepath gsave stroke grestore clip\n"

    //adjust scale
    << width / abs_width << " " << height / abs_height << " scale\n"

    // adjust origin
    << -x.lo() << " " << -y.lo() << " translate\n";

  //set thickness of drawn lines
  params::max_thickness *= abs_width / width;
  if (constant_thickness)
    *output << "/w " << max_thickness << " def\n";

  //proc to draw a line given xinc, yinc, x0, y0, and thickness
  *output << "/l {setlinewidth newpath moveto rlineto stroke} bind def\n"
    << "/p {"; //p is for proxy
  if (constant_thickness)
    *output << "w ";
  *output << "l} bind def\n"

    << "/z {0 1 0 setrgbcolor p} bind def\n" //contains zero
    << "/d {1 0 0 setrgbcolor p} bind def\n" //domain error
    << "/o {0 0 1 setrgbcolor p} bind def\n" //overflow

    //proc to draw subdivision borders given xinc, yinc, x0, y0, thickness
    << (!show_subdivision ? "" :
       "/s {0 setlinewidth 0 setgray\n" // set params
       "    4 index dup mul 4 index dup mul add sqrt div\n" // thickness/length
       "    dup 4 index mul exch 5 index mul neg\n" // perpendicular vector
       "    4 2 roll\n" // move to corner
       "    exch 3 index 0.5 mul add exch 2 index 0.5 mul add newpath moveto\n" 
       "    3 index 3 index rlineto\n" // line 1
       "    exch neg exch neg rlineto\n" // line 2
       "    exch neg exch neg rlineto\n" // line 3
       "    closepath stroke\n" // line 4
       "  } bind def\n")

    //proc to draw tangent lines
    << "/t {0 0.5 0 setrgbcolor 0 l} bind def\n";

  recurse(x.lo(), 0.5 * (y.lo() + y.hi()), abs_width, 0.0, abs_height);
  *output << "grestore\n"
    << "showpage\n"
    << "%%EOF\n";
  if (output != &std::cout)
    delete output;
  return 0;
}

#define make_sstream(input) (is.clear(), is.str(input), is)

void params::set_params (int argc, char** argv)
{
  std::istringstream is;
  int i;
  for (i = 1; i < argc; ++i) {
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
    } else if (!std::strcmp(argv[i], "-c")) {
      constant_thickness = false;
    } else if (!std::strcmp(argv[i], "-Xg")) {
      constant_thickness = gradient_subdivision = false;
    } else if (!std::strcmp(argv[i], "-s")) {
      show_subdivision = true;
    } else if (!std::strcmp(argv[i], "-d")) {
      show_tangent = true;
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

  if (argc <= i)
    print_usage(argv[0]);
  operation = argv[i++];
  for (; i < argc; ++i)
    operation += std::string(" ") + argv[i];
  
  
  globals::x = &globals::vars["x"]; //allocate space for variables
  globals::y = &globals::vars["y"];

  expression = evaluator::make_evaluator(operation.c_str(), globals::vars);

  for (std::string::iterator i = operation.begin(); i != operation.end(); ++i)
    if (*i == '\\' || *i == '(' || *i == ')') {
      //postscript uses parentheses as string delimiters
      i = operation.insert(i, '\\');
      ++i;
    }
}

void recurse (Real x0, Real y0, Real xinc, Real yinc, Real thickness)
{
  using params::output;
  Real length = std::sqrt(xinc * xinc + yinc * yinc);
  Real x1 = x0, y1 = y0, xinc1 = xinc, yinc1 = yinc, thickness1 = thickness,
    length1 = length;
  Real a, b;
  bool domerr = false;
  bool overflow = false;
  bool contains_zero = false;

  if (params::show_subdivision)
    *output << xinc << ' ' << yinc << ' ' << x0 << ' ' << y0 << ' '
      << thickness << " s\n";

  {
    Affine& x = *globals::x;
    Affine& y = *globals::y;
    Affine& z = *params::expression->value;
    x = x0 + 0.5 * xinc * (1.0 + globals::e_l)
      + 0.5 * thickness * yinc / length * globals::e_t;
    y = y0 + 0.5 * yinc * (1.0 + globals::e_l)
      - 0.5 * thickness * xinc / length * globals::e_t;
    params::expression->reset();
    try { params::expression->safe_evaluate(); }
    catch (Affine::DomainError) {
      domerr = true;
    }

    if (!std::isfinite(z.lo()) || !std::isfinite(z.hi())) {
      domerr = false;
      overflow = true;
    } else {
      if (!domerr && ((z.lo() <= 0.0) && (z.hi() >= 0.0)))
        contains_zero = true;

      if ((params::gradient_subdivision || params::show_tangent)
          && contains_zero) {
        z.collapse();
        solve_linear(z, a, x, b, y);

        //solve for diagonal line instead of clamping x and y
        if (a != 0.0) {
          Interval newx = intersect(Interval(x), Interval(x - z / a));
          if (xinc != 0.0) {
            x1 = newx.lo();
            xinc1 = width(newx);
            length1 = xinc1;
          } else {
            x1 = 0.5 * (newx.hi() + newx.lo());
            thickness1 = width(newx);
          }
        }
 
        if (b != 0.0) {
          Interval newy = intersect(Interval(y), Interval(y - z / b));
          if (xinc != 0.0) {
            y1 = 0.5 * (newy.hi() + newy.lo());
            thickness1 = width(newy);
          } else {
            y1 = newy.lo();
            yinc1 = width(newy);
            length1 = yinc1;
          }
        }
      }
    }
  }

  if (length1 < thickness1) { //thickness should be < length for good slope
    Real newxinc = thickness1 * yinc1 / length1;
    Real newyinc = -thickness1 * xinc1 / length1;
    x1 = x1 + 0.5 * xinc1 - 0.5 * newxinc;
    y1 = y1 + 0.5 * yinc1 - 0.5 * newyinc;
    xinc1 = newxinc;
    yinc1 = newyinc;
    Real temp = thickness1;
    thickness1 = length1;
    length1 = temp;
  }

  if (domerr || overflow || contains_zero) { // it's an area of interest
    if (thickness1 <= params::max_thickness) { // and small enough to draw
      *output << xinc1 << " " << yinc1 << " " << x1 << " " << y1 << " ";
      if (!params::constant_thickness)
        *output << thickness1 << " ";

      if (contains_zero) {
        *output << "z\n";
        if (params::show_tangent) {
          Real m = 1/std::sqrt(a*a + b*b);
          *output << b*m << " " << -a*m << " " << x1 << " " << y1 << " t\n";
        }
      } else if (domerr) //domain error
        *output << "d\n";
      else //overflow
        *output << "o\n";
    } else { //recurse
      Real newxinc = 0.5 * xinc1;
      Real newyinc = 0.5 * yinc1;
      recurse(x1, y1, newxinc, newyinc, thickness1);
      recurse(x1 + newxinc, y1 + newyinc, newxinc, newyinc, thickness1);
    }
  }
}
