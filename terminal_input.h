#pragma once

#include <deque>
#include <mutex>
#include <termios.h>
#include <thread>

class TerminalInput {
 public:
  TerminalInput();

  ~TerminalInput();

  bool start();

  void run();

  char getNextChar();

  bool stop();

 private:

  termios tios;
  std::deque<char> chars;
  std::thread *input_thread;
  std::mutex input_mutex;
  bool should_stop;
};
