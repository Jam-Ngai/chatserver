#pragma once
#include "utilities.hpp"


struct SectionInfo {
  SectionInfo() {};
  SectionInfo(const SectionInfo& src);
  SectionInfo& operator=(const SectionInfo& src);
  std::string operator[](const std::string& key);

  std::unordered_map<std::string, std::string> section_datas_;
};

class ConfigManager {
 public:
  ConfigManager();
  ConfigManager(const ConfigManager& src);
  SectionInfo operator[](const std::string& section);
  ConfigManager& operator=(const ConfigManager& src);

 private:
  // 存储section和key-value对的map
  std::unordered_map<std::string, SectionInfo> config_map_;
};