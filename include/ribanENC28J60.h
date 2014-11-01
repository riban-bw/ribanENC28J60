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
#include "constants.h"

#define IP4

/** @brief  This class provides an Ethernet interface with minimal IP protocol
*   @note   Use #define IP6 to enable IPV6. Use #undefine IP4 to disable IPV4
*   @note   Check initialisation is successful by calling GetNicVersion() which should be non-zero.
*   @note   Call Process() regularly (e.g. within main program loop)
*/
class ribanENC28J60
{
    friend class Socket;
    public:
        /** @brief  Initialise the interface
        *   @param  addressMAC The MAC address for the network interface
        *   @param  nChipSelectPin Arduino pin number used as chip select. Default = 10.
        *   @return <i>bool</i> True on success
        */
        bool Initialise(Address &addressMAC, byte nChipSelectPin = 10);

        /** @brief  Get the ENC28J60 silicon version
        *   @return <i>byte</i> Version. Zero if not correctly initialised
        */
        byte GetNicVersion();

        /** @brief  Process recieved data and send any pending data
        *   @return <i>byte</i> Quantity of packets processed (including unrecognised and invalid packets)
        *   @note   Processes default protocols then iterates through sockets then drops unprocessed packets
        */
        byte Process();

        /** @brief  Set the handler function for transmission errors
        *   @param  TxErrorHandler Pointer to error handler function
        *   @note   Error handler function should be declared: void HandleTxError();
        */
        void SetTxErrorHandler(void (*HandleTxError)());

        /** @brief  Get the local hardware (MAC) address
        *   @return <i>Address*</i> Pointer to the MAC address
        */
        Address* GetMac() { return &m_addressLocalMac; };

        /** @brief  Starts a raw transmission transaction
        *   @param  pMac Optional pointer to remote host MAC address. Default is broadcast address FF:FF:FF:FF:FF:FF
        *   @param  nEthertype Optional Ethertype or length of this Ethernet packet. Default is 0x0800 (IPV4)
        *   @note   Call TxAppend to append data to the transmission transaction
        *   @note   Call TxEnd to close transaction and send packet
        */
        void TxBegin(Address* pMac = NULL, uint16_t nEthertype = 0x0800);

        /** @brief  Appends data to a raw transmission transaction
        *   @param  pData Pointer to data to append
        *   @param  nLen Quantity of bytes to append
        *   @return <i>bool</i> True on success. False if insufficient space left in Tx buffer
        *   @note   Call TxBegin before appending data
        *   @note   Call TxEnd to complete transaction and send packet
        */
        bool TxAppend(byte* pData, uint16_t nLen);

        /** @brief  Ends a transmission transaction
        */
        void TxEnd();

        /** @brief  Gets transmission error
        *   @return <i>byte</i> Bitwise flag of transmission errors
        */
        byte TxGetError() { return m_nic.TxGetError(); }

        #ifdef IP4
        IPV4 ipv4;
        #endif // IP4

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

        Address m_addressLocalMac; //!< Pointer to local host hardware MAC address
        Address m_addressRemoteMac; //!< Pointer to remote host hardware MAC address

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
        byte m_nNicVersion; //!< ENC28J60 silicon version - zero if ENC28J60 not initialised succesfully
};
