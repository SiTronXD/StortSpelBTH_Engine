#include "Configurator.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <cassert>
#include <fstream>
#include <regex>

vengine_helper::config::configHolder vengine_helper::config::conf; //TODO: Do not use a global...

namespace vengine_helper::config
{
void readConfIntoMap(std::ifstream &conf_file,
               std::map<std::string, std::string> &configs) {
  std::string line;

  const std::regex conf_line(R"(^([a-zA-Z_-]+):([a-zA-Z_0-9\\/\.]+);)");

  std::smatch conf_match;
  while (std::getline(conf_file, line)) {
    if (std::regex_match(line, conf_match, conf_line)) {
      // std::cout << line << std::endl;
      if (conf_match[1].matched && conf_match[2].matched) {
        // std::cout << "RULE:"  <<  conf_match[1].str() << std::endl; // rule
        // matched std::cout << "VALUE:" <<  conf_match[2].str() << std::endl;
        // // value matched
        configs.insert({conf_match[1].str(), conf_match[2].str()});
      } else {
        
        assert( false &&  "config.cfg contains a invalid rule!");
      }
    }
  }
  
}
void storeConfInMemory(std::map<std::string, std::string> &configs) {
  for (auto &rule : conf.rules) {
    auto map_rule = configs.find(std::string(rule.first));
    auto *rule_val = &rule.second;
    if (configs.end() != map_rule) {
      switch (rule_val->type) {
      case Type::int_t:
        rule_val->value =
            std::any_cast<int>(std::stoi(std::string(map_rule->second)));
        break;
      case Type::float_t:
        rule_val->value =
            std::any_cast<float>(std::stof(std::string(map_rule->second)));
        break;
      case Type::douible_t:
        rule_val->value =
            std::any_cast<double>(std::stod(std::string(map_rule->second)));
        break;
      case Type::string_t:
        rule_val->value = std::any_cast<std::string>(map_rule->second);
        break;
      case Type::bool_t:
        auto temp_bool = std::any_cast<std::string>(map_rule->second);
        std::transform(temp_bool.begin(),temp_bool.end(), temp_bool.begin(), ::toupper);
        rule_val->value = temp_bool == "TRUE" || temp_bool == "1";
        break;
      }
    } else {
      std::cout << "invalid rule!" << std::endl;
    }
  }
}
void verifyStoredConfigs(std::map<std::string, std::string> &configs) {
  if (configs.size() != conf.rules.size()) {
    for (auto &rule_config : configs) {
      if (!conf.rules.contains(std::string(rule_config.first))) {
        std::cout << "config.cfg invalid option: " << rule_config.first
                  << std::endl;
      }
    }
    for (auto &rule : conf.rules) {
      if (!configs.contains(std::string(rule.first))) {
        std::cout << "config.cfg missing option: " << rule.first << std::endl;
      }
    }
  }
}
void loadConfIntoMemory() 
{
    using namespace vengine_helper::config;
    
  std::ifstream conf_file(std::string(DEF<std::string>(P_ASSETS)) + "config.cfg", std::ios::in);
  if (conf_file.is_open()) {
    std::map<std::string, std::string> configs;
    readConfIntoMap(conf_file, configs);
    conf_file.close();

    storeConfInMemory(configs);

    verifyStoredConfigs(configs);
  }
}

    float camera_fov()
    {
        
        //return get_float(std::string("camera_fov"));
        return get_rule_val<float>(std::string("camera_fov"));
        
    }
    
    
    
    
    template<typename T>
    T get_rule_val(std::string&& ruleName)
    {
        std::cout << std::any_cast<T>(conf.rules.find(ruleName)->second.value) << std::endl;        
        return std::any_cast<T>(conf.rules.find(ruleName)->second.value);
    }


} // namespace vengine_helper::config
