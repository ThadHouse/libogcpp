#include "ogcpp/mutex.h"
#include "ogcpp/condition_variable.h"
#include "ogcpp/thread.h"

void func() {
  gc::mutex mutex;
  gc::condition_variable cond;
  std::unique_lock<gc::mutex> lock(mutex);

  cond.wait(lock);

  gc::thread thr([](){

  });
}
