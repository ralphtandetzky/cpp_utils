#pragma once

#include "concurrent_queue.hpp"
#include <future>
#include <type_traits>

namespace cu
{

class TaskQueue
{
public:
  template <typename F>
  auto push( F && f )
  {
    auto task = std::packaged_task<std::result_of_t<F()>()>(
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
