//
// Created by descartes on 5/26/24.
//

#ifndef VANET_PHYSICLAYER_H
#define VANET_PHYSICLAYER_H
#include <list>
#include <memory>
#include "Packet.h"
#include <string>
#include <cmath>

class PhysicLayer {
public:
    struct PhysicLayerPacket : Packet<int, std::string> {
        explicit PhysicLayerPacket(int header, std::string payload): Packet<int, std::string>(header, payload){}
    };
private:

    static std::list<std::reference_wrapper<PhysicLayer>> all;

    double range;

    std::reference_wrapper<PhysicLayer> ref;
public:
    struct Position {
        double x,y;
    } position;
    int id;
    std::list<PhysicLayerPacket> receivedPackets;

    bool isInRange(PhysicLayer & other){
        double distance = sqrt(pow(this->position.x - other.position.x,2) + pow(this->position.y - other.position.y,2));
        return distance < this->range or distance < other.range;
    }

    void send(PhysicLayerPacket packet){
        for (auto &other: all){
            if(isInRange(other)){
                other.get().receivedPackets.push_front(packet);
            }
        }
    }

    PhysicLayer(double range): range(range), ref(std::ref(*this)){
        all.push_back(ref);
    }

    ~PhysicLayer(){
        all.remove_if([this](std::reference_wrapper<PhysicLayer> & element)->bool{
            return element.get().id == id;
        });
    }
};

#endif //VANET_PHYSICLAYER_H
