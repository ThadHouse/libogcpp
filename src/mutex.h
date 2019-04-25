#pragma once

#include <mutex>
#include <ogc/mutex.h>

namespace gc {
class recursive_mutex {
 public:
  using native_handle_type = mutex_t;

  recursive_mutex() noexcept {
    LWP_MutexInit(&m_mutex, false);
  }
  ~recursive_mutex() noexcept {
    if (m_mutex != 0) {
      LWP_MutexDestroy(m_mutex);
    }
  }
  recursive_mutex(const recursive_mutex&) = delete;
  recursive_mutex& operator=(const recursive_mutex&) = delete;
  recursive_mutex(recursive_mutex&& other) = delete;
  recursive_mutex& operator=(recursive_mutex&& other) = delete;

  void lock() { LWP_MutexLock(m_mutex); }
  void unlock() { LWP_MutexUnlock(m_mutex); }
  bool try_lock() { return !LWP_MutexTryLock(m_mutex); }

  native_handle_type native_handle() { return m_mutex; }

 private:
  mutex_t m_mutex;
};

class condition_variable;

class mutex {
 public:
  friend class condition_variable;
  using native_handle_type = mutex_t;

  mutex() noexcept {
    LWP_MutexInit(&m_mutex, false);
  }
  ~mutex() {
    if (m_mutex != 0) {
      LWP_MutexDestroy(m_mutex);
    }
  }
  mutex(const mutex&) = delete;
  mutex& operator=(const mutex&) = delete;
  mutex(mutex&& other) {
    m_mutex = other.m_mutex;
    other.m_mutex = 0;
  }
  mutex& operator=(mutex&& other) {
    if (m_mutex != 0) {
      LWP_MutexDestroy(m_mutex);
    }
    m_mutex = other.m_mutex;
    other.m_mutex = 0;
    return *this;
  }

  void lock() { LWP_MutexLock(m_mutex); }
  void unlock() { LWP_MutexUnlock(m_mutex); }
  bool try_lock() { return LWP_MutexTryLock(m_mutex); }

  native_handle_type native_handle() { return m_mutex; }

 private:
  mutex_t m_mutex;
};
} // gc
