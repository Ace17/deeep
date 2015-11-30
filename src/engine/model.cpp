#include "model.h"
#include "geom.h"
#include "json.h"
#include "util.h"
#include <sstream>

static
string asString(json::Value* pVal)
{
  return json::cast<json::String>(pVal)->value;
}

static
int asInt(json::Value* pVal)
{
  auto const s = asString(pVal);
  int i;
  stringstream ss(s);
  ss >> i;
  return i;
}

// -----------------------------------------------------------------------------
// Single-sheet

static
Action loadSheetAction(json::Value* val, string sheetPath, Dimension2i cell)
{
  Action r;

  auto action = json::cast<json::Object>(val);
  action->getMember<json::String>("name");
  auto frames = action->getMember<json::Array>("frames");

  for(auto& frame : frames->elements)
  {
    auto const idx = asInt(frame.get());

    auto const col = idx % 16;
    auto const row = idx / 16;
    r.addTexture(sheetPath, Rect2i(col * cell.width, row * cell.height, cell.width, cell.height));
  }

  return r;
}

// -----------------------------------------------------------------------------

Model loadModel(string jsonPath)
{
  Model r;
  auto obj = json::load(jsonPath);
  auto dir = dirName(jsonPath);

  auto type = obj->getMember<json::String>("type")->value;
  auto actions = obj->getMember<json::Array>("actions");

  if(type == "sheet")
  {
    auto sheet = obj->getMember<json::String>("sheet")->value;
    auto width = asInt(obj->getMember<json::String>("width"));
    auto height = asInt(obj->getMember<json::String>("height"));

    auto cell = Dimension2i(width, height);

    for(auto& action : actions->elements)
      r.actions.push_back(loadSheetAction(action.get(), dir + "/" + sheet, cell));
  }
  else
    throw runtime_error("Unknown model type: '" + type + "'");

  return r;
}

int loadTexture(string path, Rect2i rect = Rect2i(0, 0, 0, 0));

void Action::addTexture(string path, Rect2i rect)
{
  textures.push_back(loadTexture(path, rect));
}

