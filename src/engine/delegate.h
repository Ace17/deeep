#pragma once

#include <memory>

template<class>
struct Delegate;

template<typename... Args>
struct Delegate<void(Args...)>
{
  // invokes the delegate
  void operator () (Args... args) { invokable->call(args...); }

  Delegate() = default;

  Delegate(void(*f)(Args...))
  {
    invokable = std::make_unique<StaticInvokable>(f);
  }

  void operator = (Delegate<void(Args...)>&& other)
  {
    invokable.reset(other.invokable.get());
    other.invokable.release();
  }

  void operator = (void (* f)(Args...))
  {
    invokable = std::make_unique<StaticInvokable>(f);
  }

  template<typename Lambda>
  Delegate(const Lambda& func)
  {
    invokable = std::make_unique<LambdaInvokable<Lambda>>(func);
  }

  template<typename Lambda>
  void operator = (const Lambda& func)
  {
    invokable = std::make_unique<LambdaInvokable<Lambda>>(func);
  }

  operator bool () const { return invokable.ptr; }

private:
  struct Invokable
  {
    virtual ~Invokable() = default;
    virtual void call(Args... args) = 0;
  };

  std::unique_ptr<Invokable> invokable;

  // concrete invokable types
  struct StaticInvokable : Invokable
  {
    StaticInvokable(void(*f)(Args...)) : funcPtr(f) {}
    void call(Args... args) override { (*funcPtr)(args...); }
    void (* funcPtr)(Args...) = nullptr;
  };

  template<typename Lambda>
  struct LambdaInvokable : Invokable
  {
    LambdaInvokable(Lambda f) : funcPtr(f) {}
    void call(Args... args) override { funcPtr(args...); }
    Lambda funcPtr {};
  };
};

