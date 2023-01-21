#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <boost/json.hpp>

#include "model.h"

namespace json_loader { 

namespace json = boost::json;

void ParseFile(const std::filesystem::path& json_path, boost::json::value& file_json);
void CreateGame(boost::json::value& file_json, model::Game& game);
void ReadRoadsIntoMap(const json::value& maps, model::Map& map);
void ReadBuildingsIntoMap(const json::value& maps, model::Map& map_for_add);
void ReadOfficesIntoMap(const json::value& maps, model::Map& map_for_add);
model::Game LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader