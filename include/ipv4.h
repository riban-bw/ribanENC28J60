/**     IPV4 provides IPv4 class derived from ribanEthernet
*       Copyright (c) 2014, Brian Walton. All rights reserved. GLPL.
*       Source availble at https://github.com/riban-bw/ribanEthernet.git
*
*       Allows different protocols to be added
*/

///!@note   Configure ARP table size with #define ARP_TABLE_SIZE. Default size is 8. 2 entries are used internally for gateway and DNS.

#pragma once

const static unsigned int ARP_GATEWAY_INDEX = 0;
const static unsigned int ARP_DNS_INDEX     = 1;

#include "address.h"
#include "constants.h"

class ENC28J60;

class ArpEntry
{
    public:
        ArpEntry()
        {
            //Reset IP and MAC addresses to all zeros
            memset(ip, 0, 4);
            memset(mac, 0, 6);
        };
        byte ip[4];
        byte mac[6];
};

class IPV4
{
    public:
        /** @brief  Construct an IPV4 protocol handler
        *   @param  pInterface Pointer to the network interface object
        */
        IPV4();

        /** @brief  Initialise IPV4 class
        *   @param  pInterface Pointer to the network interface object
        */
        void Initialise(ENC28J60* pInterface);

        /** @brief  Configure network interface with static IP
        *   @param  pIp Pointer to IP address (4 bytes). 0 for no change.
        *   @param  pGw Pointer to gateway address (4 bytes). 0 for no change. Default = 0
        *   @param  pDns Pointer to DNS address (4 bytes). 0 for no change. Default = 0
        *   @param  pNetmask Pointer to subnet mask (4 bytes). 0 for no change. Default = 0
        *   @return <i>bool</i> Returns true on success - actually always true
        */
        void ConfigureStaticIp(Address *pIp,
                            Address *pGw = 0,
                            Address *pDns = 0,
                            Address *pNetmask = 0);

        /** @brief  Configure network interface with DHCP
        */
        void ConfigureDhcp();

        /** @brief  Starts a transmission transaction
        *   @param  pDestination Pointer to the destination IP address. set to null to use source address in last recieved packet
        *   @param  nProtocol IPV4 protocol number
        *   @note   Creates Ethernet and IP header. Clears checksum and length fields
        */
        void TxBegin(Address* pDestination, uint16_t nProtocol);

        /** @brief  Append byte to transmission transaction
        *   @param  nData Single byte of data to append
        *   @return <i>bool</i> True on success. Fails if insufficient space in Tx buffer
        */
        bool TxAppend(byte nData);

        /** @brief  Append 16-bit word to transmission transaction
        *   @param  nData 16-bit word of data to append
        *   @return <i>bool</i> True on success. Fails if insufficient space in Tx buffer
        *   @todo   Move common functions to class and encapsulate - e.g. resuse for IPV6 protocol
        */
        bool TxAppend(uint16_t nData);

        /** @brief  Appends data to transmission transaction
        *   @param  pData Pointer to data
        *   @param  nLen Quantity of bytes to append
        *   @return <i>bool</i> True on success. Fails if insufficient space in Tx buffer
        */
        bool TxAppend(byte* pData, uint16_t nLen);

        /** @brief  Write a single byte to specific position in write buffer
        *   @param  nOffset Position offset from start of IPV4 payload
        *   @param  nData Data to write
        *   @note   Leaves append buffer cursor unchanged. Tx packet size is only changed if nOffset is greater than current size
        *   @todo   Add unit test for TxWriteByte
        */
        void TxWrite(uint16_t nOffset, byte nData);

        /** @brief  Write a 16-bit word to specific position in write buffer
        *   @param  nOffset Position offset from start of IPV4 payload
        *   @param  nData Data to write
        *   @note   nData is host byte order, word is written network byte order, i.e. bytes are swapped before writing to buffer
        *   @note   Leaves append buffer cursor unchanged. Tx packet size is only changed if nOffset is greater than current size
        *   @todo   Add unit test for TxWriteWord
        */
        void TxWrite(uint16_t nOffset, uint16_t nData);

        /** @brief  Write data to specific position in write buffer
        *   @param  nOffset Position offset from start of IPV4 payload
        *   @param  pData Pointer to data to be written
        *   @param  nLen Quantity of bytes to write to buffer
        *   @note   Leaves append buffer cursor and Tx packet size counter unchanged.
        */
        void TxWrite(uint16_t nOffset, byte* pData, uint16_t nLen);

        /** @brief  Ends a transmission transaction
        *   @note   Finishes populating header and requests packet be sent
        */
        void TxEnd();

        /** @brief  Process packet / data
        *   @param  nLen Quantity of data bytes in recieve buffer
        *   @note   Expects Rx read pointer to point to start of IP header
        */
        void Process(uint16_t nLen);

        /** @brief  Process ARP packet
        *   @param  nLen Quantity of bytes in ARP packet
        *   @note   Recieve pointer should point to start of ARP header
        *   @note   Assumes valid IPV4 ARP header
        *   @note   Library maintins an ARP table. If this message is an ARP reply, the table is updated if there is an entry with the same IP address.
        *   @note   See ArpLookup
        */
        void ProcessArp(uint16_t nLen);

        /** @brief  Send an echo request (ping)
        *   @param  pIp Pointer to remote host IP
        *   @param  pHandler Pointer to handler function
        *   @return <i>uint16_t</i> Sequence number of this ping (may be used to match response)
        *   @note   Handler function is called when an echo response is recieved
        *   @note   Only one echo response handler function may be defined. Redefining will result in all responses being handled by newly defined function
        *   @note   Handler function should be declared: void HandleEchoResponse(uint16_t nSequence); where nSequence is the echo response sequence number
        *   @todo   Add ping parameters, e.g. quantity of pings, response handler, etc.
        */
        uint16_t Ping(Address* pIp, void (*HandleEchoResponse)(uint16_t nSequence));

        /** @brief  Enable / disable ICMP (ping) responses
        *   @param  bEnable True to enable, false to disable
        */
        void EnableIcmp(bool bEnable);

        /** @brief  Perform ARP request and update ARP cache table
        *   @param  pIp Pointer to IP address
        *   @param  nTimeout Quantity of milliseconds to wait for ARP response before abandoning ARP request as failed
        *   @return <i>byte*</i> Pointer to resulting MAC address or NULL on failure (timeout)
        *   @note   Blocks program flow whilst waiting for ARP response. All recieved packets are lost.
        */
        byte* ArpLookup(byte* pIp, uint16_t nTimeout = 500);

        Address* GetIp() { return &m_addressLocal; };
        byte* GetGw() { return m_aArpTable[ARP_GATEWAY_INDEX].ip; };
        byte* GetDns() { return m_aArpTable[ARP_DNS_INDEX].ip; };
        Address* GetNetmask() { return &m_addressMask; };
        Address* GetBroadcastIp() { return &m_addressBroadcast; };

        /** @brief  Get the IP address of the remote host from the last recieved packet
        *   @param  address Address object to populate
        *   @brief  Implement GetRemoteIp
        */
        void GetRemoteIp(Address& address);

        bool IsUsingDhcp() { return m_nDhcpStatus != DHCP_DISABLED; };

    protected:

    private:
        /** @brief  Check for ICMP and process
        *   @param  nLen Quantity of bytes in payload
        *   @return <i>bool</i> True if ICMP packet processed
        */
        bool ProcessIcmp(uint16_t nLen);

        /** @brief  Checks whether IP address is same as local host IP address
        *   @param  pIp IP address to check
        *   @return <i>bool</i> True if same
        */
        bool IsLocalIp(byte* pIp);

        /** @brief  Checks whether IP address is on local subnet or whether a gatway is required to reach host
        *   @param  pIp IP address to check
        *   @return <i>bool</i> True if on local subnet
        *   @todo   Should IsOnLocalSubnet accept Address as parameter instead of byte*?
        */
        bool IsOnLocalSubnet(byte* pIp);

        /** @brief  Check whether IP address is a broadcast address (subnet or global)
        *   @param  pIp Pointer to IP address
        *   @return <i>bool</i> True if a broadcast address
        */
        bool IsBroadcast(byte* pIp);

        /** @brief  Check whether IP address is a multicast address
        *   @param  pIp Pointer to IP address
        *   @return <i>bool</i> True if a multicast address
        */
        bool IsMulticast(byte* pIp);

        bool m_bIcmpEnabled; //!< True to enable ICMP responses
        Address m_addressLocal; //!< IP address of local host
        Address m_addressRemote; //!< IP address of remote host
        Address m_addressGw; //!< Pointer to gateway / router IP address
        Address m_addressDns; //!< Pointer to DNS IP address
        Address m_addressMask; //!< Pointer to netmask
        Address m_addressSubnet; //!< Pointer to the subnet address
        Address m_addressBroadcast; //!< Pointer to broadcast address
        byte m_nDhcpStatus; //!< Status of DHCP configuration DHCP_DISABLED | DHCP_REQUESTED | DHCP_BOUND | DHCP_RENEWING

        byte m_nArpCursor; //!< Cursor holds index of next ARP table entry to update
        byte m_nIpv4Protocol; //!< IPv4 protocol of current message
        uint16_t m_nTxPayload; //!< Quantity of bytes in IPV4 Tx payload
        uint16_t m_nHeaderLength; //!< Quantity of bytes in recieved IPV4 header. Note: All transmitted packets have IPV4_HEADER_SIZE sized header
        uint16_t m_nPingSequence; //!< ICMP echo response sequence number
        uint16_t m_nIdentification; //!< IPv4 packet identification
        uint16_t m_nIpv4Port; //!< IPv4 port number

        ENC28J60* m_pInterface; //!< Pointer to network interface object

        #ifndef ARP_TABLE_SIZE
            #define ARP_TABLE_SIZE 8 //Default to ARP table of gateway, DNS plus 6 remote host addresses
        #endif // ARP_TABLE_SIZE
        ArpEntry m_aArpTable[ARP_TABLE_SIZE + 2]; //!< ARP table. Minimum size is 2 to hold gateway and DNS host IP/MAC
        void (*m_pHandleEchoResponse)(uint16_t nSequence); //!< Pointer to function to handle echo response (pong)

};

