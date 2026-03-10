#pragma once

#include <EppoCore/Core/BufferReader.h>
#include <EppoCore/Core/BufferWriter.h>

#include <chrono>

constexpr uint32_t MAX_PAYLOAD_SIZE = 1024 * 1024;

enum class PacketType : uint16_t
{
    Unknown = 0,

    // Server -> Client
    // 32-bit integer -> Client map size
    // Key/value map of type clientid/clientinfo
    ClientList,

    // Client -> Server
    // 8-bit integer (bool) -> Client is typing yes/no
    //
    // Server -> Client
    // 32-bit unsigned int -> Client id
    // 8-bit integer (bool) -> Client is typing yes/no
    ClientTyping,

    // Server -> Client
    // 32-bit unsigned int -> Client id
    // UTF-8 string -> Username
    ClientConnected,

    // Server -> Client
    // 32-bit unsigned int -> Client id
    ClientQuit,

    // Client -> Server
    // UTF-8 string -> Username
    //
    // Server -> Client
    // 8-bit integer (bool) -> Client accepted yes/no
    // UTF-8 string -> Error message
    ConnectionRequest,

    // Client -> Server
    // MessageData Object
    //
    // Server -> Client
    // MessageData Object
    Message,

    // Server -> Client
    // UTF-8 string -> Error message
    Error,
};

struct MessageData
{
    ClientID UserID;
    int64_t Timestamp;
    std::string Username;
    std::string Message;

    static auto Serialize(Eppo::BufferWriter* serializer, const MessageData& object) -> bool
    {
        serializer->WriteRaw<uint32_t>(object.UserID);
        serializer->WriteRaw<int64_t>(object.Timestamp);
        serializer->WriteString(object.Username);
        serializer->WriteString(object.Message);

        return true;
    }

    static auto Deserialize(Eppo::BufferReader* serializer, MessageData& object) -> bool
    {
        using namespace std::chrono;

        serializer->ReadRaw<uint32_t>(object.UserID);
        serializer->ReadRaw<int64_t>(object.Timestamp);
        serializer->ReadString(object.Username);
        serializer->ReadString(object.Message);

        return true;
    }
};
