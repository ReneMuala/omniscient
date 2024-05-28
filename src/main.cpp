#include <libsumo/libtraci.h>
#include <iostream>
#include <thread>
#include "ApplicationLayer.h"
#include "World.hpp"
#include "Terminal.hpp"

using namespace libtraci;

void updateTerminalPosition(std::shared_ptr<Terminal> & terminal, libsumo::TraCIPosition & position){
    auto & vehiclePosition = terminal->applicationLayer->underlyingLayer->underlyingLayer->underlyingLayer->position;
    vehiclePosition.x = position.x;
    vehiclePosition.y = position.y;
}

void runApplicationOnTerminal(std::shared_ptr<Terminal> & terminal, int time){
    terminal->applicationLayer->send(NetworkLayer::broadcastAddress,
                                     ApplicationLayer::ApplicationLayerPacket("RESF-ROAD-STATE",
                                                                               "CHECK-STATUS;"+
                                                                               std::to_string(time)+";"+
                                                                               std::to_string(terminal->applicationLayer->underlyingLayer->address.binary)),
                                                                               time);
    terminal->applicationLayer->receive(time);
    int issuesOnThisLane = 0;
    auto packets = std::move(terminal->applicationLayer->receivedPackets);
    auto thisVehicleLaneId = Vehicle::getLaneID(terminal->id);
    auto thisVehicleImpatience = Vehicle::getImpatience(terminal->id);
    auto lanesIssues = std::map<std::string, int>();
    std::for_each(packets.begin(), packets.end(), [&](ApplicationLayer::ApplicationLayerPacket & packet){
        auto senderStr = std::string(packet.payload.begin() + (int)packet.payload.find_last_of(';')+1, packet.payload.end());
        auto senderInt = (int)strtol(senderStr.c_str(), nullptr, 10);
        auto sender = NetworkLayer::IPv4Address { .binary = senderInt };

        if(sender.binary == terminal->applicationLayer->underlyingLayer->address.binary ){
//            std::cout << "(LPBP)";
            return;
        }

//        auto packetTimeStr = std::string (packet.payload.begin() + (int)packet.payload.find_first_of(';')+1,packet.payload.begin() + (int)packet.payload.find_last_of(';'));
//        auto packetTimeInt = (int)strtol(packetTimeStr.c_str(), nullptr, 10);


        auto payload = std::string(packet.payload.begin(), packet.payload.begin() + packet.payload.find_first_of(';'));
//        std::cout << "RECEIVED PACKET: " << packet.header << " " << payload << " SENDER: " << NetworkLayer::convertIPv4AddressToString(sender) << " TIME: " << packetTimeInt << std::endl;

        if(payload == "CHECK-STATUS"){
            terminal->applicationLayer->send(sender,
                                             ApplicationLayer::ApplicationLayerPacket("RESF-ROAD-STATE",
                                                                                      "STATUS/"+std::to_string(thisVehicleImpatience)+"/"+thisVehicleLaneId+";"+
                                                                                      std::to_string(time)+";"+
                                                                                      std::to_string(terminal->applicationLayer->underlyingLayer->address.binary)),
                                                                                      time);
        } else if(std::string(payload.begin(), payload.begin() + (int)payload.find_first_of('/')) == "STATUS"){
            auto impatientBegin = payload.begin() + (int)payload.find_first_of('/')+1;
            auto impatientEnd = payload.begin() + (int)payload.find_last_of('/');
            auto impatientStr = std::string(impatientBegin, impatientEnd);
            auto impatient = strtod(impatientStr.c_str(), nullptr);

            auto laneBegin = impatientEnd +1;
            auto laneEnd = payload.end();
            auto laneId = std::string(laneBegin, laneEnd);

            if(laneId == thisVehicleLaneId && impatient > 0.1){
                lanesIssues[laneId]++;
                issuesOnThisLane++;
            }
        }
    });

    if(issuesOnThisLane){
        std::string nextLaneId;
        int nextLaneIndex = 0;
        for(auto & lane : Vehicle::getBestLanes(terminal->id)){
            if(nextLaneId.empty() or lanesIssues[lane.laneID] < lanesIssues[nextLaneId]){
                nextLaneId = lane.laneID;
                nextLaneIndex = (int)(std::find(Lane::getIDList().begin(), Lane::getIDList().end(),lane.laneID) - Lane::getIDList().begin());
            }
        }

        std::cout << "VEÍCULO "
        << terminal->id
        << " :: deve mudar a rota devido a "
        << issuesOnThisLane
        << " feedbacks(s) de outros motoristas na via: " << Vehicle::getLaneID(terminal->id) << std::endl;
        Vehicle::setAcceleration(terminal->id, 100.0, 10);
    }
}

int main() {
    NetworkLayer::DHCPProvider dhcp {
        NetworkLayer::IPv4Address {.octets {192,168,0,1}},
        NetworkLayer::IPv4Address {.octets {254,254,254,254}},
    };

    Simulation::start({"sumo-gui", "-n", "../res/osm.net.xml", "-r", "../res/osm.passenger.trips.xml", "-a", "../res/osm.poly.xml"});

    for(int i = 1 ; i <= 500 ; i++){
        Simulation::step();
        for(auto & id : Vehicle::getIDList()){
            auto position = Vehicle::getPosition(id);
            std::shared_ptr<Terminal> terminal;
            if(not World::isPresent(id)){
                auto phy = std::make_shared<PhysicLayer>(10);
                auto mac = std::make_shared<MacLayer>(phy);
                auto net  = std::make_shared<NetworkLayer>(dhcp.get().first, mac);
                auto app = std::make_shared<ApplicationLayer>("RESF-ROAD-STATE",net);
                terminal = std::make_shared<Terminal>(id,app);
                World::registerTerminal(terminal);
            } else {
                terminal = World::getById(id);
            }
            updateTerminalPosition(terminal, position);
            runApplicationOnTerminal(terminal, i);
//            std::cout << "VEHICLE " << id << " IS NOW AT: " << position.x << ", " << position.y << std::endl;
        }
    }
    Simulation::close();
    return 0;
}
