/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "entity_factory.h"
#include <map>
#include <stdexcept>

using namespace std;

static const map<string, CreationFunc> registry = getRegistry();

static
vector<string> parseCall(string content)
{
  content += '\0';
  auto stream = content.c_str();

  auto head = [&] ()
    {
      return *stream;
    };

  auto accept = [&] (char what)
    {
      if(!*stream)
        return false;

      if(head() != what)
        return false;

      stream++;
      return true;
    };

  auto expect = [&] (char what)
    {
      if(!accept(what))
        throw runtime_error(string("Expected '") + what + "'");
    };

  auto parseString = [&] ()
    {
      string r;

      while(!accept('"'))
      {
        char c = head();
        accept(c);
        r += c;
      }

      return r;
    };

  auto parseIdentifier = [&] ()
    {
      string r;

      while(isalnum(head()) || head() == '_' || head() == '-')
      {
        char c = head();
        accept(c);
        r += c;
      }

      return r;
    };

  auto parseArgument = [&] ()
    {
      if(accept('"'))
        return parseString();
      else
        return parseIdentifier();
    };

  vector<string> r;
  r.push_back(parseIdentifier());

  if(accept('('))
  {
    bool first = true;

    while(!accept(')'))
    {
      if(!first)
        expect(',');

      r.push_back(parseArgument());
      first = false;
    }
  }

  return r;
}

unique_ptr<Entity> createEntity(string formula)
{
  auto words = parseCall(formula);
  auto name = words[0];
  words.erase(words.begin());
  EntityArgs args = words;

  auto i_func = registry.find(name);

  if(i_func == registry.end())
    throw runtime_error("unknown entity type: '" + name + "'");

  return (*i_func).second(args);
}

