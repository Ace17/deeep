#pragma once

#include "base/delegate.h"
#include "game.h"
#include <list>

struct HandleWithDeleter : Handle
{
  HandleWithDeleter(Delegate<void(void)> deleter_) : deleter(std::move(deleter_))
  {
  }

  ~HandleWithDeleter()
  {
    deleter();
  }

  Delegate<void(void)> deleter;
};

struct Variable : IVariable
{
  ~Variable()
  {
    assert(observers.empty());
  }

  int get() override
  {
    return value;
  }

  void set(int newValue) override
  {
    value = newValue;

    for(auto& observer : observers)
      observer(newValue);
  }

  std::unique_ptr<Handle> observe(Observer&& observer) override
  {
    auto it = observers.insert(observers.begin(), std::move(observer));

    auto removeObserver = [ = ] () { observers.erase(it); };

    return std::make_unique<HandleWithDeleter>(removeObserver);
  }

private:
  std::list<Observer> observers;
  int value;
};

