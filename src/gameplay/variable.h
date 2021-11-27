#pragma once

#include "base/delegate.h"
#include "game.h"
#include <list>

struct HandleWithDeleter : Handle
{
  HandleWithDeleter(Delegate<void(void)> deleter_) : deleter(std::move(deleter_)) {}

  ~HandleWithDeleter() { deleter(); }

  Delegate<void(void)> deleter;
};

struct Variable : IVariable
{
  ~Variable()
  {
    assert(observers.empty());
  }

  virtual int get() override
  {
    return value;
  }

  virtual void set(int newValue) override
  {
    value = newValue;

    for(auto& observer : observers)
      observer(newValue);
  }

  virtual unique_ptr<Handle> observe(Observer&& observer) override
  {
    auto it = observers.insert(observers.begin(), std::move(observer));

    auto removeObserver = [ = ] () { observers.erase(it); };

    return make_unique<HandleWithDeleter>(removeObserver);
  }

private:
  list<Observer> observers;
  int value;
};

