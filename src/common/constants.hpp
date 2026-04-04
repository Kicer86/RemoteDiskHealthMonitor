#pragma once

#include <string>

#include "ProtocolVersion.h"


inline const std::string ZeroConfServiceName = "_RDHMonitor._tcp";
inline constexpr unsigned int RDHMPort = 1630;
inline const ProtocolVersion VersionOfProtocol = ProtocolVersion::VER_1;
inline const std::string ApplicationShortName = "RDHM";
