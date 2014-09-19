#include "ui.h"

#include <iostream>

#include <ncurses.h>

using std::cout;
using std::endl;
using std::string;

Ui::Ui(bool use_curses)
  : use_curses(use_curses) {

  if (use_curses) {
    initscr();
    noecho();
    nodelay(stdscr, TRUE);
  }
}

Ui::~Ui() {
  if (use_curses) {
    endwin();
  }
}

void Ui::print(const string &str) {
  if (use_curses) {
    printw(str.c_str());
    refresh();
  }
  else {
    cout << str << endl;
  }
}

char Ui::getInput() {
  if (use_curses) {
    return getch();
  }
  else {
    return -1;
  }
}
