#pragma once

#include "concurrent_queue.hpp"
#include <future>

namespace cu
{

class TaskQueue
{
public:
  template <typename F>
  auto push( F && f )
  {
    auto task = std::packaged_task<typename std::result_of<F()>::type()>(
          std::forward<F>(f) );
    auto result = task.get_future();
    tasks.emplace( std::move(task) );
    return result;
  }

  void popAndExecute()
  {
    tasks.pop()();
  }

private:
  ConcurrentQueue<std::packaged_task<void()>> tasks;
};

} // namespace cu
