#ifndef SerialCommunication_h
#define SerialCommunication_h

typedef uint8_t (*ReceptionOpe)(char);

class SerialCommunication
{
public:
  void init(void);
  void setReceptOpe(ReceptionOpe* pRcvOpeSet);
  void sendByte(uint8_t byte);
  void sendBuffer(const char* buffer, size_t len);

private:
};

#endif  // SerialCommunication_h
