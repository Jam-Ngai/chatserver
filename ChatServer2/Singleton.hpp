#pragma once
#include <memory>
#include <mutex>

template <typename T>
class Singleton {
 public:
  static std::shared_ptr<T>& GetInstance() {
    static std::once_flag flag;
    std::call_once(flag, [&]() { instance_ = std::shared_ptr<T>(new T); });
    return instance_;
  }
  ~Singleton() {};

 protected:
  Singleton() = default;
  Singleton(const Singleton<T>&) = delete;
  Singleton& operator=(const Singleton<T>&) = delete;

 private:
  static std::shared_ptr<T> instance_;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;