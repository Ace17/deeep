#include "entity_factory.h"
#include <map>
#include <functional>

using namespace std;

static const map<string, CreationFunc> registry = getRegistry();

unique_ptr<Entity> createEntity(string formula)
{
  EntityArgs args;
  auto name = formula;

  auto i_func = registry.find(name);

  if(i_func == registry.end())
    throw runtime_error("unknown entity type: '" + name + "'");

  return (*i_func).second(args);
}

