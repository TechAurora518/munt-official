// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#pragma once

#include <cstdint>
#include <string>
#include <utility>

struct PeerRecord final {
    int64_t id;
    std::string ip;
    std::string hostname;
    std::string addrLocal;
    std::string addrBind;
    int64_t start_height;
    int64_t synced_height;
    int64_t common_height;
    int64_t time_connected;
    int64_t time_offset;
    int64_t latency;
    int64_t last_send;
    int64_t last_receive;
    int64_t send_bytes;
    int64_t receive_bytes;
    std::string userAgent;
    int64_t protocol;
    int64_t services;
    bool inbound;
    bool whitelisted;
    bool addnode;
    bool relay_txes;
    int64_t banscore;

    PeerRecord(int64_t id_,
               std::string ip_,
               std::string hostname_,
               std::string addrLocal_,
               std::string addrBind_,
               int64_t start_height_,
               int64_t synced_height_,
               int64_t common_height_,
               int64_t time_connected_,
               int64_t time_offset_,
               int64_t latency_,
               int64_t last_send_,
               int64_t last_receive_,
               int64_t send_bytes_,
               int64_t receive_bytes_,
               std::string userAgent_,
               int64_t protocol_,
               int64_t services_,
               bool inbound_,
               bool whitelisted_,
               bool addnode_,
               bool relay_txes_,
               int64_t banscore_)
    : id(std::move(id_))
    , ip(std::move(ip_))
    , hostname(std::move(hostname_))
    , addrLocal(std::move(addrLocal_))
    , addrBind(std::move(addrBind_))
    , start_height(std::move(start_height_))
    , synced_height(std::move(synced_height_))
    , common_height(std::move(common_height_))
    , time_connected(std::move(time_connected_))
    , time_offset(std::move(time_offset_))
    , latency(std::move(latency_))
    , last_send(std::move(last_send_))
    , last_receive(std::move(last_receive_))
    , send_bytes(std::move(send_bytes_))
    , receive_bytes(std::move(receive_bytes_))
    , userAgent(std::move(userAgent_))
    , protocol(std::move(protocol_))
    , services(std::move(services_))
    , inbound(std::move(inbound_))
    , whitelisted(std::move(whitelisted_))
    , addnode(std::move(addnode_))
    , relay_txes(std::move(relay_txes_))
    , banscore(std::move(banscore_))
    {}
};
