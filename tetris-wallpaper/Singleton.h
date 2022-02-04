#pragma once

template<typename T> class Singleton {
protected:
  Singleton() {}

public:
  Singleton(const Singleton&) = delete;

  Singleton& operator=(const Singleton&) = delete;

  static T& Instance()
  {
    static T instance;
    return instance;
  }
};
