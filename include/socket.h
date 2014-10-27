/*  Create one socket per network connection.
    Each socket describes a single connection. This may be UDP listening, UDP send, TCP listening, TCP connect, RAW listening, RAW sending.
    Only one packet may be populated for transmission. Recieve data handlers may send data so all transmission calls should be complete before calling nic.process.
*/
#pragma once
#include "Arduino.h"

//Define Domain names
static const byte AF_INET   = 0;
static const byte AF_INET6  = 1;
static const byte AF_PACKET = 2;

static const byte PROTO_RAW = 0;
static const byte PROTO_UDP = 1;
static const byte PROTO_TCP = 2;

static const byte SOCK_IDLE         = 0;
static const byte SOCK_WAIT         = 1;
static const byte SOCK_TBA          = 2; //!@todo Define connection states
static const byte SOCK_CONNECTED    = 3;

class Address;
class ribanEthernet;

class Socket
{
    public:
        /** @brief  Create a socket
        *   @param  nDomain The network domain: AF_INET | AF_INET6 | AF_PACKET
        *   @param  pInterface Pointer to the network interface to use
        *   @param  nProtocol Protocol to implement within socket communication: PROTO_RAW | PROTO_UDP | PROTO_TCP
        *   @todo   Consider order of arguments
        */
        Socket(byte nDomain, ribanEthernet* pInterface, byte nProtocol);

        virtual ~Socket();

        /** @brief  Starts a transmission transaction
        *   @param  pAddress Pointer to the target address. Null to use socket connection. Default is NULL
        *   @param  nPort Target port number. Zero to use socket connection. Default is zero.
        */
        void TxBegin(Address* pAddress = NULL, uint16_t nPort = 0);

        /** @brief  Appends data to a transmission transaction
        *   @param  pData Pointer to data to append
        *   @param  nSize Quantity of bytes to append
        */
        void TxAppend(byte* pData, uint16_t nSize);

        /** @brief  Completes transmit transaction and sends data
        */
        void TxEnd();

        /** @brief  Sends a packet of data
        *   @param  pData Pointer to data
        *   @param  nSize Quantity of bytes to send
        *   @param  pAddress Pointer to an address to send to (optional - only for connectionless protocols, e.g. UDP)
        *   @param  nPort Port to send to (optional - only for connectionless protocols, e.g. UDP)
        */
        void Send(byte* pData, uint16_t nSize, Address* pAddress = NULL, uint16_t nPort = 0);

        /** @brief  Binds a socket to a port and triggers an event when packets are recieved on that port. Automatically accepts connections.
        *   @param  nPort Port number to listen on
        *   @todo   Add event handler pointer to param list
        */
        void Listen(uint16_t nPort);

        /** @brief  Sets a time (in seconds) to disconnect connection
        *   @brief  nTimeout Quantity of seconds of idle (no data flow) before connection cleared. Set to zero to disable disconnection on idle
        */
        void SetIdleDisconnectTimeout(uint16_t nTimeout);

        /** @brief  Attempts to make a connection to a remote host. If the socket is configured for connectionless protocol.
        *   @brief  address Address of remote host
        *   @brief  nPort Port number to connect to
        *   @note   Only valid for session type protocols, e.g. TCP
        *   @todo   Should Connect block until connected / failed and return success?
        */
        void Connect(const Address& address, uint16_t nPort);

        /** @brief  Check if socket connected
        *   @return <i>bool</i> True if socket connected
        */
        bool IsConnected();


    protected:

    private:
        byte m_nProtocol; //!< Socket type PROTO_RAW | PROTO_UDP | PROTO_TCP
        byte m_nStatus; //!< Connection status SOCK_CLOSED | SOCK_INIT | SOCK_LISTEN | SOCK_ESTABLISHED | SOCK_CLOSE_WAIT
        uint16_t nPortLocal; //!< Local host port number
        uint16_t nPortRemote; //!< Remote host port number
        Address* m_pIpRemote; //!< Pointer to remote host IP address
        ribanEthernet* m_pInterface; //!< Pointer to the network interface
};

