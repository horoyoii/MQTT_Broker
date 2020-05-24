
enum class MessageType{
    CONNECT = 1,
    CONNACK,
    PUBLISH,
    PUBACK,
    PUBREC,
    PUBREL,
    PUBCOMP,
    SUBSCRIBE,
    SUBACK,
    UNSUBSCRIBE,
    UNSUBACK,
    PINGREQ,
    PINGRESP,
    DISCONNECT

};

class Message{
public:
    static MessageType getMessageType(char Header_first_byte){
        unsigned char type = ((unsigned char)Header_first_byte)>>4;
        
        return static_cast<MessageType>(type);
    }   

    static int get_QoS_level(char *buf){
        unsigned char off = 3;
        unsigned char QoS = (unsigned char)(buf[0]>>1) & off;

        return static_cast<int>(QoS);
    }
    
    static bool hasRetainFlag(char *buf){
        unsigned char off = 1;
        return (unsigned char)(buf[0]) & off; 
    }


private:




};
