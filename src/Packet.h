//
// Created by descartes on 5/26/24.
//

#ifndef VANET_PACKET_H
#define VANET_PACKET_H

template<typename HT, typename PT>
struct Packet {
    HT header;
    PT payload;
    Packet(){}
    explicit Packet(HT header, PT payload): header(header), payload(payload){}
};

#endif //VANET_PACKET_H
