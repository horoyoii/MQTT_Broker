
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



private:




};
