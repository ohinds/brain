#include <iostream>
#include <string>

#include "brain.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[]) {
  Brain brain(argv[1]);
  brain.init();
  brain.run();
  return 0;
}
