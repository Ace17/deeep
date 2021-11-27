#pragma once

#include <memory>

template<class>
struct Delegate;

template<typename RetType, typename... Args>
struct Delegate<RetType(Args...)>
{
  // invokes the delegate
  RetType operator () (Args... args) const { return invokable->call(args...); }

  Delegate() = default;
  Delegate(Delegate &&) = default;

  Delegate(RetType(*f)(Args...))
  {
    invokable = std::make_unique<StaticInvokable>(f);
  }

  void operator = (Delegate<RetType(Args...)>&& other)
  {
    invokable.reset(other.invokable.get());
    other.invokable.release();
  }

  void operator = (RetType (* f)(Args...))
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
    virtual RetType call(Args... args) = 0;
  };

  std::unique_ptr<Invokable> invokable;

  // concrete invokable types
  struct StaticInvokable : Invokable
  {
    StaticInvokable(RetType(*f)(Args...)) : funcPtr(f) {}
    RetType call(Args... args) override { return (*funcPtr)(args...); }
    RetType (* funcPtr)(Args...) = nullptr;
  };

  template<typename Lambda>
  struct LambdaInvokable : Invokable
  {
    LambdaInvokable(Lambda f) : funcPtr(f) {}
    RetType call(Args... args) override { return funcPtr(args...); }
    Lambda funcPtr {};
  };
};

