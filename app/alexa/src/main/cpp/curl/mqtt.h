#ifndef HEADER_CURL_MQTT_H
#define HEADER_CURL_MQTT_H

#ifndef CURL_ENABLE_MQTT
extern const struct Curl_handler Curl_handler_mqtt;
#endif
enum mqttstate {
    MQTT_FIRST,
    MQTT_REMAINING_LENGTH,
    MQTT_CONNACK,
    MQTT_SUBACK,
    MQTT_SUBACK_COMING,
    MQTT_PUBWAIT,
    MQTT_PUB_REMAIN,
    MQTT_NOSTATE
};
struct mqtt_conn {
    enum mqttstate state;
    enum mqttstate nextstate;
    unsigned int packetid;
};
struct MQTT {
    char *sendleftovers;
    size_t nsend;
    size_t npacket;
    unsigned char firstbyte;
    size_t remaining_length;
};
#endif