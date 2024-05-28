//
// Created by descartes on 5/27/24.
//

#ifndef VANET_WORLD_HPP
#define VANET_WORLD_HPP

#include "ApplicationLayer.h"
#include "Terminal.hpp"
#include <thread>
#include <iostream>
#include <libsumo/libtraci.h>

class World {
    static std::list<std::shared_ptr<Terminal>> terminals;
public:
    static std::shared_ptr<Terminal> getById(std::string & terminalId){
        return *std::find_if(terminals.begin(), terminals.end(), [&](std::shared_ptr<Terminal> & terminal){
           return terminalId == terminal->id;
        });
    }

    static bool isPresent(std::string & terminalId){
        return std::any_of(terminals.begin(), terminals.end(), [&](std::shared_ptr<Terminal> & terminal)->bool {
            return  terminalId == terminal->id;
        });
    }

    static void registerTerminal(std::shared_ptr<Terminal> terminal){
        auto & address =  terminal->applicationLayer->underlyingLayer->address;
        std::cout << "NEW TERMINAL: " << terminal->id << ", " << NetworkLayer::convertIPv4AddressToString(address)<< std::endl;
        terminals.push_front(std::move(terminal));
    }
};

#endif //VANET_WORLD_HPP
