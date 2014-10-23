#ifndef _PIPESTREAM_H
#define _PIPESTREAM_H
//Stupid streambuf and iostream types for bidirectional IPC.
//streambuf partly based on _The C++ Standard Library: A Tutorial and_
//_Reference_, pp. 671-81.

#include <streambuf>
#include <istream>
#include <cstdio>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>

namespace unix {
  using ::close;
  using ::dup2;
  using ::execve;
  using ::fork;
  using ::pid_t;
  using ::pipe;
  using ::poll;
  using ::pollfd;
  using ::read;
  using std::signal;
  using ::write;
  extern "C" char** environ;
}

class fdbuf : public std::streambuf {
public:
  fdbuf (int in, int out); //read from in, write to out
  virtual std::streamsize showmanyc ();
protected:
  virtual int_type overflow (int_type);
  virtual std::streamsize xsputn (const char*, std::streamsize);
  virtual int_type underflow ();

  int fdin, fdout;
  const static int size = 84;
  char inbuffer[size];
};

class pipestream : public std::iostream {
public:
  pipestream (const char *, char *const [], char *const []);
  virtual ~pipestream ();
protected:
  fdbuf* makebuf (const char*, char *const[], char *const []);
  int read, write;
  fdbuf* buf;
};

inline fdbuf::fdbuf (int in, int out) : fdin(in), fdout(out)
{
  setg(inbuffer + 4, inbuffer + 4, inbuffer + 4);
}

inline std::streamsize fdbuf::showmanyc ()
{
  ptrdiff_t diff = egptr() - gptr();
  if (!diff) {
    unix::pollfd p = { fdin, POLLIN };
    if (unix::poll(&p, 1, 0) == 1) {
      underflow();
      diff = egptr() - gptr();
    }
  }
  return diff;
}

inline std::streambuf::int_type fdbuf::overflow (std::streambuf::int_type c)
{
  if (c != EOF) {
    char z = c;
    if (unix::write(fdout, &z, 1) != 1)
      return EOF;
  }
  return c;
}

inline std::streamsize fdbuf::xsputn (const char* s, std::streamsize num)
{
  return unix::write(fdout, s, num);
}

inline std::streambuf::int_type fdbuf::underflow ()
{
  if (gptr() < egptr())
    return *gptr();
  int numPutback = gptr() - eback();
  if (numPutback > 4)
    numPutback = 4;
  std::memcpy(inbuffer + 4 - numPutback, gptr() - numPutback, numPutback);

  int num = unix::read(fdin, inbuffer + 4, size - 4);
  if (num <= 0)
    return EOF; //should check for -EPIPE

  setg(inbuffer + 4 - numPutback, inbuffer + 4, inbuffer + 4 + num);
  return *gptr();
}

inline pipestream::pipestream (const char *cmd, char *const argv[],
    char *const envp[]) : std::iostream(makebuf(cmd, argv, envp))
{}

inline pipestream::~pipestream ()
{
  delete buf;
  unix::close(read);
  unix::close(write);
}

inline fdbuf* pipestream::makebuf (const char *cmd, char *const argv[],
    char *const envp[])
{
  int infd[2], outfd[2]; //stdin, stdout of child
  unix::pipe(infd);
  unix::pipe(outfd);
  unix::pid_t forkval = unix::fork();
  if (forkval == -1)
    return 0; //error
  else if (forkval) { //parent, this is the pid, as if we care
    unix::close(infd[0]);
    unix::close(outfd[1]);
    write = infd[1];
    read = outfd[0];
    unix::signal(SIGCHLD, SIG_IGN);
    //SIGPIPE should signal that child is done or dead
    return buf = new fdbuf(read, write);
  }
  else
  {
    unix::dup2(infd[0], STDIN_FILENO); //set stdin to read from parent
    unix::dup2(outfd[1], STDOUT_FILENO); //set stdout to write to parent
    unix::dup2(outfd[1], STDERR_FILENO); //set stderr to write to parent
    unix::close(infd[1]);
    unix::close(outfd[0]);
    unix::execve(cmd, argv, envp);
    return 0; //to make gcc happy
  }
}

#endif //_PIPESTREAM_H
