//
// Created by descartes on 5/26/24.
//

#ifndef VANET_APPLICATIONLAYER_H
#define VANET_APPLICATIONLAYER_H
#include "Packet.h"
#include "NetworkLayer.h"

class ApplicationLayer {
public:
    struct ApplicationLayerPacket : Packet<std::string, std::string> {
        explicit ApplicationLayerPacket(std::string header, std::string payload): Packet<std::string, std::string>(header, std::move(payload)){}

        static ApplicationLayerPacket fromNetworkLayerPacket(NetworkLayer::NetworkLayerPacket & packet){
            return ApplicationLayer::ApplicationLayerPacket {
                    std::string(packet.payload.begin(), packet.payload.begin() + packet.payload.find_first_of(';')),
                    std::string(packet.payload.begin() + packet.payload.find_first_of(';')+1, packet.payload.end()),
            };
        }

        static NetworkLayer::NetworkLayerPacket toNetworkLayerPacket(ApplicationLayer::ApplicationLayerPacket & packet,NetworkLayer::IPv4Address & address, int time){
            return NetworkLayer::NetworkLayerPacket {
                    address,
                    packet.header + ";" + packet.payload,
                    time,
            };
        }
    };

    std::string id;
    std::list<ApplicationLayerPacket> receivedPackets;
    std::shared_ptr<NetworkLayer> underlyingLayer;

    explicit ApplicationLayer(std::string id, std::shared_ptr<NetworkLayer> netLayer): id(id), underlyingLayer(netLayer) {}
    
    void send(NetworkLayer::IPv4Address address, ApplicationLayerPacket packet, int time){
        underlyingLayer->send(ApplicationLayerPacket::toNetworkLayerPacket(packet, address, time));
    }

    void receive(int time){
        underlyingLayer->receive(time);
        auto netPackets = std::move(underlyingLayer->receivedPackets);
        std::for_each(netPackets.rbegin(), netPackets.rend(), [&](NetworkLayer::NetworkLayerPacket & packet){
            const auto & new_packet = ApplicationLayerPacket::fromNetworkLayerPacket(packet);
            if(new_packet.header == id){
                receivedPackets.push_front(new_packet);
            }
        });
    }
};

#endif //VANET_APPLICATIONLAYER_H
