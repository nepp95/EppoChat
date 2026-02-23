#pragma once

#include <EppoCore/Core/BufferReader.h>
#include <EppoCore/Core/BufferWriter.h>

#include <chrono>

// With utf-8 strings, we use 64 bit uints to signify the string length
// Both buffer reader and writer account for this

constexpr uint32_t MAX_PAYLOAD_SIZE = 1024 * 1024;

enum class PacketType : uint16_t
{
    Unknown = 0,

    // Client -> Server
    // UTF-8 string -> Username
    //
    // Server -> Client
    // 8-bit integer (bool) -> Client accepted yes/no
    ConnectionRequest,

    // Server -> Client
    // 32-bit integer -> Client map size
    // Key/value map of type clientid/clientinfo
    ClientList,

    // Client -> Server
    // 64-bit signed int -> Message time since epoch in millisecond precision
    // UTF-8 string -> Message
    //
    // Server -> Client
    // 32-bit unsigned int -> Message count
    // Below is either a single item or a map, depending on message count
    // 32-bit unsigned int -> ClientID
    // 64-bit signed int -> Message time since epoch in millisecond precision
    // UTF-8 string -> Message
    Message,
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
