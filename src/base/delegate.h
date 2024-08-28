// Copyright (C) 2023 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// lightweight functor
#pragma once

template<class>
struct Delegate;

template<typename RetType, typename... Args>
struct Delegate<RetType(Args...)>
{
  // invokes the delegate
  RetType operator () (Args... args) const { return invokable->call(args...); }

  Delegate() = default;
  Delegate(Delegate &&) = default;

  Delegate(RetType (*f)(Args...))
  {
    reset(new StaticInvokable(f));
  }

  void operator = (Delegate<RetType(Args...)>&& other)
  {
    reset(other.invokable);
    other.invokable = nullptr;
  }

  void operator = (RetType (* f)(Args...))
  {
    reset(new StaticInvokable(f));
  }

  template<typename Lambda>
  Delegate(const Lambda& func)
  {
    reset(new LambdaInvokable<Lambda>(func));
  }

  template<typename Lambda>
  void operator = (const Lambda& func)
  {
    reset(new LambdaInvokable<Lambda>(func));
  }

  operator bool () const
  {
    return invokable;
  }

private:
  struct Invokable
  {
    virtual ~Invokable() = default;
    virtual RetType call(Args... args) = 0;
  };

  Invokable* invokable = nullptr;

  void reset(Invokable* i)
  {
    delete invokable;
    invokable = i;
  }

  // concrete invokable types
  struct StaticInvokable : Invokable
  {
    StaticInvokable(RetType (*f)(Args...)) : funcPtr(f)
    {
    }
    RetType call(Args... args) override { return (*funcPtr)(args...); }
    RetType (* funcPtr)(Args...) = nullptr;
  };

  template<typename Lambda>
  struct LambdaInvokable : Invokable
  {
    LambdaInvokable(Lambda f) : funcPtr(f)
    {
    }
    RetType call(Args... args) override { return funcPtr(args...); }
    Lambda funcPtr {};
  };
};

