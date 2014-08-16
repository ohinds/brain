#include "terminal_input.h"

#include <iostream>
#include <unistd.h>

using std::cin;
using std::mutex;
using std::thread;

TerminalInput::TerminalInput()
  : input_thread(NULL)
  , should_stop(false)
{
  cfmakeraw(&tios);
}

TerminalInput::~TerminalInput() {
  if (!should_stop) {
    stop();
  }
}

bool TerminalInput::start() {
  input_thread = new thread(&TerminalInput::run, this);
  return input_thread != NULL;
}

void TerminalInput::run() {
  while (!should_stop) {
    char c;

    // TODO this blocks and means another char must be hit after 'q'
    // to actually quit.
    cin >> c;

    input_mutex.lock();
    chars.push_back(c);
    input_mutex.unlock();
  }
}

char TerminalInput::getNextChar() {
  char ret = 0;

  input_mutex.lock();

  if (!chars.empty()) {
    ret = chars.front();
    chars.pop_front();
  }

  input_mutex.unlock();

  return ret;
}

bool TerminalInput::stop() {
  should_stop = true;
  input_thread->join();
  delete input_thread;
  input_thread = NULL;

  return true;
}
