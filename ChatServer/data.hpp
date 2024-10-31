#pragma once
#include <string>

struct UserInfo {
  UserInfo()
      : name(""),
        pwd(""),
        uid(0),
        email(""),
        nick(""),
        desc(""),
        sex(0),
        icon(""),
        back("") {}
  std::string name;
  std::string pwd;
  int uid;
  std::string email;
  std::string nick;
  std::string desc;
  int sex;
  std::string icon;
  std::string back;
};

struct ApplyInfo {
  ApplyInfo(int uid, std::string name, std::string desc, std::string icon,
            std::string nick, int sex, int status)
      : uid(uid),
        name(name),
        desc(desc),
        icon(icon),
        nick(nick),
        sex(sex),
        status(status) {}

  int uid;
  std::string name;
  std::string desc;
  std::string icon;
  std::string nick;
  int sex;
  int status;
};