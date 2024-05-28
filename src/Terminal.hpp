//
// Created by descartes on 5/27/24.
//

#ifndef VANET_TERMINAL_HPP
#define VANET_TERMINAL_HPP

#include <libsumo/libtraci.h>
#include <iostream>
#include <thread>
#include "ApplicationLayer.h"

class Terminal {
public:
    std::shared_ptr<ApplicationLayer> applicationLayer;
    std::string id;
    explicit Terminal(std::string id, std::shared_ptr<ApplicationLayer> applicationLayer): id(id), applicationLayer(applicationLayer){}
};

#endif //VANET_TERMINAL_HPP
