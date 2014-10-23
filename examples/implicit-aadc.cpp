#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <affine/interval.h>
#include "pipestream.h"

void recurse (const Interval& x, const Interval& y, std::string operation,
    int depth, std::iostream& pipe, std::ostream& outf);

int main (int argc, char** argv)
{
  using std::cin;
  using std::cout;

  std::ostream* outs;
  std::ofstream* outf = 0;
  if(argc > 1) {
    outf = new std::ofstream(argv[1]);
    if (!outf->is_open()) {
      delete outf;
      std::cerr << "couldn't open " << argv[1] << "\n";
      return 1;
    }
    else
      outs = outf;
  }
  else
    outs = &cout;
  
  std::string operation;
  Interval xrange, yrange;
  int depth;

  cout << "%operation: ";
  getline(cin, operation);
  
  cout << "%x range, y range, depth: ";
  cin >> xrange >> yrange >> depth; //FIXME: check for input errors

  char* args[] = {"./aadc", "-q", 0};
  pipestream p(args[0], args, unix::environ);
  
  args[0] = "/bin/date";
  args[1] = "-R";
  pipestream date(args[0], args, unix::environ);
  std::string datestring;
  std::getline(date, datestring);

  char scratch;
  if (p.readsome(&scratch, 1)) {
    std::cerr << "Whoops: " << scratch;
    std::string error;
    while (std::getline(p, error))
      std::cerr << error << "\n"; //print error
    return 1;
  }
  p << "16 k\n"; //set precision
  p << "terse\n"; // set terse output

  double width, height;
  double ratio = (yrange.hi() - yrange.lo())
    / (xrange.hi() - xrange.lo());
  if (ratio > 1.0) {
    height = 504;
    width = height / ratio;
  }
  else {
    width = 504;
    height = width * ratio;
  }
    *outs << "%!PS-Adobe-3.0 EPSF-3.0\n"
    << "%%BoundingBox: 0 0 " << static_cast<int>(width + 10.5)
        << " " << static_cast<int>(height + 36.0) << "\n"
    << "%%Creator: Implicit Grapher 0.2\n"
    << "%%CreationDate: " << datestring << "\n"
    << "gsave\n"
    << "/Times-Roman findfont 12 scalefont setfont\n"
    << "5 " << height + 22 << " moveto (Depth: " << depth << "; x range: "
    << xrange << "; y range: " << yrange << ") show\n"
    << "5 " << height + 10 << " moveto (Operation: " << operation << ") show\n"
    << "0 setlinewidth 5 5 translate\n"
    
    << "/a {newpath  3 index 3 index moveto  1 index 3 index lineto\n"
    << "    1 index 1 index lineto  3 index 1 index lineto  closepath\n"
    << "    4 {pop} repeat} bind def\n"
    //proc to make a box path: input llx lly urx ury

    // draw enclosing box, and clip to it
    << "0 0 " << width << " " << height << " a gsave stroke grestore clip\n"

    << width / (xrange.hi() - xrange.lo()) << " " //adjust scale
        << height / (yrange.hi() - yrange.lo()) << " scale\n"

    << -xrange.lo() << " " << -yrange.lo() << " translate\n" //adjust origin

    << "/b {1 0 0 setrgbcolor a fill} bind def\n" //domain error
    << "/c {0 1 0 setrgbcolor a fill} bind def\n" //contains 0
    << "/d {0 setgray a stroke} bind def\n" //recursion box
    << "%/d {4 {pop} repeat} bind def %uncomment to hide boxes\n";

  recurse(xrange, yrange, operation, depth, p, *outs);
  *outs << "grestore\n"
    << "showpage\n"
    << "%%EOF\n";
  if (outf)
    delete outf;
  return 0;
}

void recurse (const Interval& x, const Interval& y, std::string operation,
    int depth, std::iostream& pipe, std::ostream& outf)
{
  pipe << x << " $x " << y << " $y\n"
    << operation << " p\n"
    << "#x #y c\n";

  char c = 0;
  pipe >> c;
  pipe.putback(c);
  bool domerr = false;
  Interval result;

  if (c != '[') { //input or domain error
    std::string scratch;
    std::getline(pipe, scratch);
    if (std::strstr(scratch.c_str(), "domain error"))
      domerr = true;
    else {
      std::cerr << "bad user input: " << scratch << "\n";
      throw;
    }
  }
  else if (!(pipe >> result)) { //probably overflow
    pipe.clear();
    std::string scratch;
    std::getline(pipe, scratch); //eat bad output
    domerr = true;
  }

  if (!depth) //don't recurse any further
    if (domerr)
      outf << x.lo() << " " << y.lo() << " " << x.hi() << " " << y.hi()
        << " b\n"; //domain error
    else if ((result.lo() <= 0) && (0 <= result.hi()))
      outf << x.lo() << " " << y.lo() << " " << x.hi() << " " << y.hi()
        << " c\n"; //may contain 0
    else {
//      outf << x.lo() << " " << y.lo() << " " << x.hi() << " " << y.hi()
//        << " d\n"; //uninteresting; no zeros or discontinuities
      return;
    }
  else {
    if ((0 < result.lo()) || (result.hi() < 0)) {
//      outf << x.lo() << " " << y.lo() << " " << x.hi() << " " << y.hi()
//        << " d\n"; //uninteresting; no zeros or discontinuities
      return;
    }
    else {
      double xcenter = (x.lo() + x.hi()) / 2;
      double ycenter = (y.lo() + y.hi()) / 2;
      Interval left(x.lo(), xcenter), right(xcenter, x.hi()),
          bottom(y.lo(), ycenter), top(ycenter, y.hi());
      recurse(left, bottom, operation, depth - 1, pipe, outf);
      recurse(right, bottom, operation, depth - 1, pipe, outf);
      recurse(right, top, operation, depth - 1, pipe, outf);
      recurse(left, top, operation, depth - 1, pipe, outf);
    }
  }
}
