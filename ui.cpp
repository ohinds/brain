#include "ui.h"

#include <ncurses.h>

using std::string;

Ui::Ui() {
  initscr();
  noecho();
  nodelay(stdscr, TRUE);
}

Ui::~Ui() {
  endwin();
}

void Ui::print(const string &str) {
  printw(str.c_str());
  refresh();
}

char Ui::getInput() {
  return getch();
}
