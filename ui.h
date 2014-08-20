#pragma once

#include <sstream>
#include <string>

class Ui {

 public:

  Ui();

  ~Ui();

  void print(const std::string &str);

  char getInput();

 private:

};

template <typename T>
Ui& operator<<(Ui &ui, T &t) {
  std::stringstream s;
  s << t;
  ui.print(s.str());

  return ui;
}
