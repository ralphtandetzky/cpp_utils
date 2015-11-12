#pragma once

#include "task_queue.hpp"

#include <thread>

namespace cu
{

class TaskQueueThread
{
private:
  TaskQueue queue;
  bool done = false;
  std::thread worker;

public:
  TaskQueueThread()
  {
    worker = std::thread( [this]()
    {
      while (!done)
      {
        queue.popAndExecute();
      }
    });
  }

  template <typename F>
  auto operator()( F && f )
  {
    return queue.push( std::forward<F>(f) );
  }

  ~TaskQueueThread()
  {
    (*this)([this](){ done = true; });
    worker.join();
  }
};

} // namespace cu
