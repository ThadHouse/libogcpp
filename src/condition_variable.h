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

  template<class Rep, class Period>
  std::cv_status wait_for(std::unique_lock<gc::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time) {
    return wait_for(lock, std::chrono::steady_clock::now() + rel_time);
  }

  template<class Rep, class Period, class Predicate>
  bool wait_for(std::unique_lock<gc::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time, Predicate pred) {
    return wait_util(lock, std::chrono::steady_clock::now() + rel_time, std::move(pred));
  }

  template<class Clock, class Duration>
  std::cv_status wait_until(std::unique_lock<gc::mutex>& lock, const std::chrono::time_point<Clock, Duration>& timeout_time) {
    timespec time;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(timeout_time);
    auto dur = timeout_time - secs;
    time.tv_sec = secs.count();
    time.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    auto retVal = LWP_CondTimedWait(m_cond, lock.mutex()->native_handle(), &time);
    if (retVal == 0) {
      return std::cv_status::no_timeout;
    } else {
      return std::cv_status::timeout;
    }
  }

  template<class Clock, class Duration, class Pred>
  bool wait_until(std::unique_lock<gc::mutex>& lock, const std::chrono::time_point<Clock, Duration>& timeout_time, Pred pred) {
    while (!pred()) {
      if (wait_until(lock, timeout_time) == std::cv_status::timeout) {
        return pred();
      }
    }
    return true;
  }

 private:
  cond_t m_cond;
};
}
