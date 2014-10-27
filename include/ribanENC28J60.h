/**     ribanENC28J60 - Extensible Ethernet interface
*       Copyright (c) 2014, Brian Walton. All rights reserved. GLPL.
*       Source availble at https://github.com/riban-bw/ribanENC28J60.git
*       Allows use of different network interface controllers (not yet!)
*       Allows different protocols to be added
*
*       Proposed protocols:
*           Raw Ethernet Type II (done)
*           IPV4 (done) (and IPV6)
*               ARP (done)
*               ICMP (echo request and response done)
*               DHCP
*               DNS
*               UDP
*                  (S)NTP
*                   SNMP
*               TCP
*                   HTTP
*                   TELNET
*                   SMTP
*                   FTP
*
*       Uses instance of a network interface chip driver (m_nic). Each NIC driver must implemnent public functions:
*           Initialize
*           packetReceive
*           TxGetStatus
*           TxClearError
*           TxBegin
*           TxAppend
*           TxWrite
*           TxEnd
*       Currently implemented NICs:
*           ENC28J60
*/

#pragma once
#include <Arduino.h>
#include "enc28j60.h"
#include "ipv4.h"
#include "socket.h"
#include "address.h"

#define IP4

//Define EtherTypes
const static unsigned int ETHTYPE_IPV4          = 0x0800;
const static unsigned int ETHTYPE_ARP           = 0x0806;
const static unsigned int ETHTYPE_IEEE801_10    = 0x8100;
const static unsigned int ETHTYPE_IPV6          = 0x86DD;


/** @brief  This class provides an Ethernet interface
*/
class ribanENC28J60
{
    friend class Socket;
    public:
        /** @brief  Construct an interface
        *   @param  addressMAC The MAC address for the network interface
        *   @param  nChipSelectPin Arduino pin number used as chip select. Default = 10.
        *   @todo   Should constructor and begin() be seperate?
        */
        ribanENC28J60(Address addressMAC, byte nChipSelectPin = 10);

        /** @brief  Destructs interface and tidies up
        */
        virtual ~ribanENC28J60();

        /** @brief  Process recieved data and send any pending data
        *   @note   Processes default protocols then iterates through sockets then drops unprocessed packets
        */
        uint16_t Process();

        /** @brief  Set the handler function for transmission errors
        *   @param  TxErrorHandler Pointer to error handler function
        *   @note   Error handler function should be declared: void HandleTxError();
        */
        void SetTxErrorHandler(void (*HandleTxError)());

        /** @brief  Get the local hardware (MAC) address
        *   @return <i>Address*</i> Pointer to the MAC address
        */
        Address* GetMac() { return m_pLocalMac; };

        /** @brief  Starts a transmission transaction
        *   @param  pDestination Pointer to the 6 byte destination MAC address
        */
        void TxBegin(byte* pDestination);

        /** @brief  Ends a transmission transaction
        *   @param  nLen Quantity of bytes in child payload, including any grandchildren
        */
        void TxEnd(uint16_t nLen);

    protected:
        /** @brief  Initialise class
        *   @note   Use to initialise derived classes
        */
        virtual void Init() {};

        /** @brief  Adds a socket to the event handler
        *   @param  pSocket Pointer to the socket
        */
        void AddSocket(Socket* pSocket);

        /** @brief  Removes socket from event handler
        *   @param  pSocket Pointer to the socket
        */
        void RemoveSocket(Socket* pSocket);

        Address* m_pLocalMac; //!< Pointer to local host hardware MAC address
        Address* m_pRemoteMac; //!< Pointer to remote host hardware MAC address

    private:

        /** @brief  Process derived classes
        *   @param  nType EtherType of the packet
        *   @param  nLen Quantity of data bytes in buffer
        *   @return <i>unsigned int<i> Quantity of bytes consumed by this protocol. Return 0 if protocol does not handle data in buffer.
        *   @note   Must consume all expected data, e.g. return value should include any padding.
        *   @note   If packet must be sent after processing, populate buffer with data and set m_nSendPacketLen with size of Ethernet payload
        */
        virtual uint16_t DoProcess(uint16_t nType, uint16_t nLen) { return 0; };

        byte m_nChipSelectPin; //!< Index of pin used to select NIC
        //!@todo Do we need to handle Tx errors and if so, does this actually work?
        void (*m_pHandleTxError)(); //!< Pointer to function to handle Tx error

        ENC28J60 m_nic; //!< ENC28J60 network interface object
        #ifdef IP4
        IPV4* m_pIpv4; //!< Pointer to IPV4 protocol hander
        #endif // IP4
        #ifdef IP6
        static IP6* m_pIpv6; //!< Pointer to IPV6 protocol hander
        #endif // IP6
};
