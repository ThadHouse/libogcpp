#include "mutex.h"
#include "condition_variable.h"
#include "thread.h"

void func() {
  gc::mutex mutex;
  gc::condition_variable cond;
  std::unique_lock<gc::mutex> lock(mutex);

  cond.wait(lock);

  gc::thread thr([](){

  });
}
