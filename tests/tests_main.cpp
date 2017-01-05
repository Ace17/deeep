/**
 * @brief Unit test framework
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "tests.h"

int main(int argc, char* argv[])
{
  char const* filter = "";

  if(argc == 2)
    filter = argv[1];

  RunTests(filter);
  return 0;
}

