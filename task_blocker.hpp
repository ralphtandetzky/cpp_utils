#pragma once

#include "monitor.hpp"

#include <chrono>
#include <condition_variable>
#include <deque>

namespace cu
{

class TaskBlocker
{
public:
  TaskBlocker( std::chrono::steady_clock::duration maxLatency )
    : maxLatency_( std::move( maxLatency ) )
  {}

  void push()
  {
    data_( cu::PassUniqueLockTag{},
           [&]( Data & data, std::unique_lock<std::mutex> & lock )
    {
      data.conditionVariable.wait( lock, [&]()
      {
        return data.pushTimes.empty() ||
               data.pushTimes.front() >= std::chrono::steady_clock::now() - maxLatency_;
      } );
      data.pushTimes.push_back( std::chrono::steady_clock::now() );
    } );
  }

  void pop()
  {
    data_( []( Data & data )
    {
      data.pushTimes.pop_front();
      data.conditionVariable.notify_all();
    } );
  }

private:
  struct Data
  {
    std::deque<std::chrono::steady_clock::time_point> pushTimes;
    std::condition_variable conditionVariable;
  };

  cu::Monitor<Data> data_;
  const std::chrono::steady_clock::duration maxLatency_;
};


class TaskBlockerItem
{
public:
  explicit TaskBlockerItem( TaskBlocker & taskBlocker )
    : taskBlocker_(taskBlocker)
  {
    taskBlocker.push();
  }

  ~TaskBlockerItem()
  {
    taskBlocker_.pop();
  }

private:
  TaskBlocker & taskBlocker_;
};


class SharedTaskBlockerItem
{
public:
  explicit SharedTaskBlockerItem( TaskBlocker & taskBlocker )
    : itemPtr_( std::make_shared<TaskBlockerItem>( taskBlocker ) )
  {}

private:
  std::shared_ptr<const void> itemPtr_;
};

} // namespace cu
