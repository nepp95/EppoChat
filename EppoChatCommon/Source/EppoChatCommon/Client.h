#pragma once

#include <EppoCore/Core/BufferReader.h>
#include <EppoCore/Core/BufferWriter.h>

#include <steam/steamnetworkingtypes.h>

#include <string>

using ClientID = HSteamNetConnection;

struct ClientInfo
{
    std::string Username;

    static auto Serialize(Eppo::BufferWriter* serializer, const ClientInfo& object) -> bool
    {
        serializer->WriteString(object.Username);
        return true;
    }

    static auto Deserialize(Eppo::BufferReader* serializer, ClientInfo& object) -> bool
    {
        serializer->ReadString(object.Username);
        return true;
    }
};
