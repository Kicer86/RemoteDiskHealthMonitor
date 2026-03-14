
#ifndef DMESGPARSER_H
#define DMESGPARSER_H

#include <map>
#include <memory>
#include <set>
#include <string>

#include <agent/Disk.h>


class IPartitionsManager;


namespace DmesgParser
{
    std::map<Disk, std::set<std::string>> parse(const std::string &, const IPartitionsManager &);
}

#endif // DMESGPARSER_H
