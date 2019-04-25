#pragma once

#include "mutex.h"
#include <ogc/cond.h>
#include <condition_variable>

namespace gc {
class condition_variable {
 public:
  using native_handle_type = cond_t;

  condition_variable() noexcept {
    LWP_CondInit(&m_cond);
  }
  ~condition_variable() noexcept {
    if (m_cond != 0) {
      LWP_CondDestroy(m_cond);
    }
  }

  condition_variable(const condition_variable&) = delete;
  condition_variable& operator=(const condition_variable&) = delete;
  condition_variable(condition_variable&&) = delete;
  condition_variable& operator=(condition_variable&&) = delete;

  void notify_one() noexcept {
    LWP_CondSignal(m_cond);
  }

  void notify_all() noexcept {
    LWP_CondBroadcast(m_cond);
  }

  void wait(std::unique_lock<gc::mutex>& lock) {
    LWP_CondWait(m_cond, lock.mutex()->m_mutex);
  }

  template<typename Predicate>
  void wait(std::unique_lock<gc::mutex>& lock, Predicate pred) {
    while (!pred()) {
      wait(lock);
    }
  }

 private:
  cond_t m_cond;
};
}
