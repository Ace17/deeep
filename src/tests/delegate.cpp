// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/delegate.h"
#include "tests.h"

unittest("Delegate: lambdas")
{
  Delegate<void(void)> callMe;

  bool wasCalled = false;
  callMe = [&] () { wasCalled = true; };
  callMe();
  assertTrue(wasCalled);
}

unittest("Delegate: return value")
{
  Delegate<int(int)> multiplyByThree;

  multiplyByThree = [] (int val) { return val * 3; };
  assertEquals(12, multiplyByThree(4));
}

unittest("Delegate: void, no args")
{
  bool invoked = false;
  auto f = [&] () { invoked = true; };
  Delegate<void()> dg = f;
  dg();
  assertTrue(invoked);
}

unittest("Delegate: return type + args")
{
  auto f = [] (int a, int b) { return a / b; };
  Delegate<int(int, int)> divide = f;
  assertEquals(32, divide(64, 2));
}

