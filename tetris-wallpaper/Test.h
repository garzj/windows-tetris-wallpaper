#pragma once

#include "Singleton.h"
#include <iostream>

class Test : public Singleton<Test> {
  Test() {
    std::cout << "test2\n";
  }
};
