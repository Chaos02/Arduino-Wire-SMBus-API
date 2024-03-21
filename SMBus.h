/*

*/

#ifndef SMB_h
#define SMB_h

#include <Arduino.h>

#define BUFFER_LENGTH 32

#define WIRE_HAS_END 1

class SystemBus {
public:
    class iReceiveHandler {
        friend class SystemBus;
        friend class iHandler;
    protected:
        /// @brief A Function that *might* handle incoming data from the BUS
        /// @param buf Buffer array. First element contains the address of the device that sent the data.
        /// @param len length of the received Buffer.
        /// @return Shall return wether or not the data was accepted by this function
        virtual bool ReceiveHandler(char buf[], unsigned int len) = 0;
    };
    class iHandler {
        friend class SystemBus;
    public:
        /// Pointer to a member function to accept a buffer of the received message
        iReceiveHandler* Handler;
        /// generic pointer to the instance of the class, the function is member of. Must be casted.
        void* Instance;
        iHandler(iReceiveHandler* handler, void* instance);
    protected:
        iHandler();
    };
    enum ReturnCode {
        Success = 0,
        // Buffer overflow, data too long
        Overflow,
        // Address not acknowledged
        ANACK,
        // Data not acknowledged
        DNACK,
        // Other Error
        ERR,
        Timeout
    };

    void begin() const;
    void begin(byte address) const;
    unsigned int fetchWord(byte address, byte func) const;
    ReturnCode readBlock(byte address, byte func, byte* blockBuffer, byte* len) const;
    static void addReceiveHandler(iHandler Handler);
    void scan() const;
private:
    static iHandler Handlers[8];
    static byte HandlerCount;
    static void HandlerQueue(int length);
};

extern SystemBus SMBus;

#endif

