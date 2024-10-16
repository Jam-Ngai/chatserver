#include "ConfigManager.hpp"
// Section
SectionInfo::SectionInfo(const SectionInfo& src) {
  section_datas_ = src.section_datas_;
}

SectionInfo& SectionInfo::operator=(const SectionInfo& src) {
  if (&src == this) this->section_datas_ = src.section_datas_;
  return *this;
}

std::string SectionInfo::operator[](const std::string& key) {
  if (section_datas_.find(key) == section_datas_.end()) {
    return "";
  }
  return section_datas_[key];
}

// ConfigManager
ConfigManager::ConfigManager() {
  std::filesystem::path current_path = std::filesystem::current_path();
  std::filesystem::path config_path = current_path / ".config";
  std::cout << "Config path: " << config_path << std::endl;
  boost::property_tree::ptree pt;
  boost::property_tree::read_ini(config_path.string(), pt);

  // 遍历INI文件中的所有section
  for (const auto& section_pair : pt) {
    const std::string& section_name = section_pair.first;
    const boost::property_tree::ptree& section_tree = section_pair.second;

    // 对于每个section，遍历其所有的key-value对
    std::unordered_map<std::string, std::string> section_config;
    for (const auto& key_value_pair : section_tree) {
      const std::string& key = key_value_pair.first;
      const std::string& value = key_value_pair.second.get_value<std::string>();
      section_config[key] = value;
    }
    SectionInfo sectionInfo;
    sectionInfo.section_datas_ = section_config;
    // 将section的key-value对保存到config_map中
    config_map_[section_name] = sectionInfo;
  }

  // 输出所有的section和key-value对
  for (const auto& section_entry : config_map_) {
    const std::string& section_name = section_entry.first;
    SectionInfo section_config = section_entry.second;
    std::cout << "[" << section_name << "]" << std::endl;
    for (const auto& key_value_pair : section_config.section_datas_) {
      std::cout << key_value_pair.first << "=" << key_value_pair.second
                << std::endl;
    }
  }
}

ConfigManager::ConfigManager(const ConfigManager& src) {
  this->config_map_ = src.config_map_;
}

SectionInfo ConfigManager::operator[](const std::string& section) {
  if (config_map_.find(section) == config_map_.end()) {
    return SectionInfo();
  }
  return config_map_[section];
}

ConfigManager& ConfigManager::operator=(const ConfigManager& src) {
  if (&src == this) this->config_map_ = src.config_map_;
  return *this;
}