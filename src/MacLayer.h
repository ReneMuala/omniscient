//
// Created by descartes on 5/26/24.
//

#ifndef VANET_MACLAYER_H
#define VANET_MACLAYER_H
#include "Packet.h"
#include <string>
#include <list>
#include <memory>
#include "PhysicLayer.h"
#include <algorithm>
#include <utility>

class MacLayer {
    static int all_ids;
public:
    struct MacLayerPacket : Packet<int, std::string> {
        explicit MacLayerPacket(int header, std::string payload): Packet<int, std::string>(header, std::move(payload)){}
        static MacLayerPacket fromPhysicLayerPacket(const PhysicLayer::PhysicLayerPacket & packet){
            return MacLayer::MacLayerPacket {
                    (int)strtol(std::string(packet.payload.begin(), packet.payload.begin() + packet.payload.find_first_of(";")).c_str(),
                                nullptr, 10),
                {packet.payload.begin() + packet.payload.find_first_of(";")+1, packet.payload.end()}
            };
        }

        static PhysicLayer::PhysicLayerPacket toPhysicLayerPacket(const MacLayer::MacLayerPacket & packet, const PhysicLayer & layer){
            return PhysicLayer::PhysicLayerPacket {
                layer.id,
                std::to_string(packet.header) + ";" + packet.payload
            };
        }
    };

    std::list<MacLayerPacket> receivedPackets;
public:
    std::shared_ptr<PhysicLayer> underlyingLayer;
    int id;

    explicit MacLayer(std::shared_ptr<PhysicLayer> phyLayer): id(all_ids++), underlyingLayer(std::move(phyLayer)){}

    void send(const MacLayerPacket & packet){
        underlyingLayer->send(MacLayer::MacLayerPacket::toPhysicLayerPacket(packet, *underlyingLayer));
    }

    void receive(){
        auto phyPackets = std::move(underlyingLayer->receivedPackets);
        std::for_each(phyPackets.rbegin(), phyPackets.rend(), [&](PhysicLayer::PhysicLayerPacket & packet){
            receivedPackets.push_front(MacLayerPacket::fromPhysicLayerPacket(std::ref(packet)));
        });
    }
};

#endif //VANET_MACLAYER_H
