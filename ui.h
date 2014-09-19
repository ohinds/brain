#pragma once

#include <sstream>
#include <string>

class Ui {

 public:

  Ui(bool use_curses);

  ~Ui();

  void print(const std::string &str);

  char getInput();

 private:

  bool use_curses;
};

template <typename T>
Ui& operator<<(Ui &ui, T &t) {
  std::stringstream s;
  s << t;
  ui.print(s.str());

  return ui;
}
