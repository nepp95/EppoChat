#pragma once

#include <EppoCore/Core/Buffer.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

enum class PacketType : uint8_t
{
    ClientUpdate,
    Message
};

struct DataPacket
{
    PacketType Type;
    Eppo::Buffer Data;
};