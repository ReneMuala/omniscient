//
// Created by descartes on 5/26/24.
//

#ifndef VANET_NETWORKLAYER_H
#define VANET_NETWORKLAYER_H
#include <array>
#include "Packet.h"
#include <string>
#include <optional>
#include "MacLayer.h"
#include <utility>
#include <iostream>

class NetworkLayer {
public:
     union IPv4Address {
         int binary;
         std::array<unsigned char, 4> octets;
     };

     static std::string convertIPv4AddressToString(IPv4Address address){
         return std::to_string(address.octets[0]) + '.'
                   + std::to_string(address.octets[1]) + '.'
                   + std::to_string(address.octets[2]) + '.'
                   + std::to_string(address.octets[3]);
     }

     class DHCPProvider {
         std::optional<IPv4Address> current;
         IPv4Address firstAddress;
         IPv4Address lastAddress;
         bool expired;

     public:

         explicit DHCPProvider(IPv4Address firstAddress, IPv4Address lastAddress): firstAddress(firstAddress), lastAddress(lastAddress), expired(false){}

         std::pair<IPv4Address, bool> get(){
             if(expired){
                 return {current.value(), false };
             }

             if(not current){
                 current = firstAddress;
             } else {
                 for(int i = 3; i >= 0; i--){
                     if(current->octets[i] < 254){
                         current->octets[i]++;
                         for(int j = i+1; j < 4;j++) {
                             current->octets[j] = 0;
                         } break;
                     }
                 }
             }
             if(current->octets[3] == 0) current->octets[3]++;

             if(current->binary == lastAddress.binary){
                 expired = true;
             }

             return {current.value(), not expired };
         }
     };

    static constexpr IPv4Address broadcastAddress = { .octets = std::array<unsigned char, 4>{255,255,255,255} };

    struct NetworkLayerPacket : Packet<IPv4Address, std::string> {
        int time;
        explicit NetworkLayerPacket(IPv4Address header, std::string payload, int _time): Packet<IPv4Address, std::string>(header, payload), time(_time){}

        static NetworkLayerPacket fromMacLayerPacket(const MacLayer::MacLayerPacket & packet){
            auto addressBegin = packet.payload.begin();
            auto addressEnd = packet.payload.begin() + (int)packet.payload.find_first_of(";");
            auto addressStr = std::string(addressBegin, addressEnd);

            auto ttlBegin = addressEnd+1;
            auto ttlEnd = packet.payload.begin() + (int)packet.payload.find_first_of(';', (int)packet.payload.find_first_of(";")+1);
            auto ttlStr = std::string(ttlBegin,  ttlEnd);
            return NetworkLayerPacket {
                    IPv4Address {
                        .binary = (int)strtol(addressStr.c_str(),
                                              nullptr, 10)},
                    { ttlEnd + 1, packet.payload.end() },
                    (int)strtol(ttlStr.c_str(),
                                nullptr, 10)
            };
        }

        static MacLayer::MacLayerPacket toMacLayerPacket(const NetworkLayer::NetworkLayerPacket & packet, const MacLayer & layer){
            return MacLayer::MacLayerPacket {
                    layer.id,
                    std::to_string(packet.header.binary) + ";" + std::to_string(packet.time) + ";" + packet.payload
            };
        }
    };
public:
    IPv4Address address;
    std::list<NetworkLayerPacket> receivedPackets;
    std::shared_ptr<MacLayer> underlyingLayer;

    NetworkLayer(IPv4Address address, std::shared_ptr<MacLayer> macLayer): address(address),underlyingLayer(macLayer){}

    void send(NetworkLayerPacket packet){
        underlyingLayer->send(NetworkLayerPacket::toMacLayerPacket(packet, *underlyingLayer));
    }

    void receive(int time){
        underlyingLayer->receive();
        auto macPackets = std::move(underlyingLayer->receivedPackets);
        std::for_each(macPackets.rbegin(), macPackets.rend(), [&](MacLayer::MacLayerPacket & packet){
            auto new_packet = NetworkLayerPacket::fromMacLayerPacket(packet);
            if(new_packet.header.binary == address.binary or new_packet.header.binary == broadcastAddress.binary){
                receivedPackets.push_front(new_packet);
            }
            if (new_packet.time == time && (new_packet.header.binary != address.binary or new_packet.header.binary == broadcastAddress.binary)){
                send(new_packet);
            }
        });
    }
};

#endif //VANET_NETWORKLAYER_H
