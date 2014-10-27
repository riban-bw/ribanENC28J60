#pragma once
#include "ribanethernetprotocol.h"

class UDP
{
    public:
        UDP();
        virtual ~UDP();

        /** @brief  Adds or removes a UDP server
        *   @param  nPort Port to listen on
        *   @param  pHandleUdpPacket Pointer to packet handler function. NULL to stop listening
        */
        void Listen(uint16_t nPort, void (*pHandleUdpPacket)(uint16_t nPort, uint16_t nLen));

        /** @brief  Send a UDP dstagramp
        *   @param  pData Pointer to buffer holding UDP payload data
        *   @param  nLen Quantity of bytes in payload
        *   @param  pIp Pointer to 4-byte IP address of target
        *   @param  nPort UDP port of target
        *   @todo   Do we want to implement this?
        */
        void Send(byte* pData, uint16_t nLen, byte* pIp, uint16_t nPort);

        /** @brief  Start UDP send transaction
        *   @param  pIp Pointer to IP address of target
        *   @param  nPort UDP port of target
        */
        void BeginPacket(byte* pIp, uint16_t nPort);

        /** @brief  Start UDP send transaction
        *   @param  pHost Pointer to host name of target
        *   @param  nPort UDP port of target
        */
        void BeginPacket(const char* pHost, uint16_t nPort);

        /** @brief  Append data to UDP payload
        *   @param  pData Pointer to data
        *   @param  nLen Quantity of bytes to append
        */
        void Append(byte* pData, uint16_t nLen);

        /** @brief  Finsih UDP send transaction and send packet
        */
        void EndPacket();

        /** @brief  Gets the source IP address of the last datagram sent to server
        *   @return <i>byte*</i> Pointer to IP address
        */
        byte* GetRemoteIp() { return m_pRemoteIp; };

        /** @brief  Gets the UDP port number of the last datagram sent to server
        *   @return <i>uint16_t</i> Port number
        */
        uint16_t GetRemotePort() { return m_nRemotePort; };

    protected:
    private:
        byte* m_pRemoteIp; //!< Pointer to 4-byte IP address of remote host (UDP server)
        uint16_t m_nRemotePort; //!< UDP port number of remote host (UDP server)
};
