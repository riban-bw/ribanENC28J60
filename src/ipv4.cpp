#include "ipv4.h"
#include "enc28j60.h"

IPV4::IPV4(ENC28J60* pInterface) :
    m_pInterface(pInterface)
{
    m_pLocalIp = new Address(ADDR_TYPE_IPV4);
}

IPV4::~IPV4()
{
    delete m_pLocalIp;
}

void IPV4::Init()
{
    m_bIcmpEnabled = true; //Respond to ICMP echo requests (pings) by default
    m_nDhcpStatus = DHCP_RESET; //Assume DHCP required until explicit request for static IP
    m_nArpCursor = 2; //First two ARP entries are for gateway (router) and DNS
}

void IPV4::Process(uint16_t nLen)
{
    #ifdef _DEBUG_
    Serial.print("+");
    #endif // _DEBUG_
    //Check for ARP message
    if(nLen < IPV4_HEADER_SIZE) //|| (pBuffer[2] & 0x40) != 0x40)  Don't think we need to check for version=4 because IPV6 uses different ethertype
        return;

    byte pBuffer[IPV4_HEADER_SIZE];
    //Get IPV4 header parameters
    m_pInterface->RxGetData(pBuffer, IPV4_HEADER_SIZE);
    uint16_t nHeaderLen = (pBuffer[0] & 0x0F) * 4;
//    uint8_t nDSCP_ECN = pBuffer[1]; //Not implemented in this library
//    uint16_t nTotalLen = (pBuffer[2] << 8) + pBuffer[3];
//    uint16_t nId =  (pBuffer[4] << 8) + pBuffer[5];
//    uint8_t nFlags = pBuffer[6] & 0x60;
//    uint16_t nFragOffset = (pBuffer[6] & 0x1F) << 8) + pBuffer[7];
//    uint8_t nTtl = pBuffer[8];
    m_nIpv4Protocol = pBuffer[9];
//    uint16_t nChecksum = (pBuffer[10] << 8) + pBuffer[11];
//    if(ribanEthernetProtocol::CreateChecksum(pBuffer, nHeaderLen))
//        return nLen; //Checksum error
//        //!@todo Warn on checksum error
    memcpy(m_pRemoteIp, pBuffer + 12, 4); //Store IP address of remote host
//    memcpy(m_pDestIp, pBuffer[16], 4);
    if(nHeaderLen > 20)
        ; //!@todo Handle IPV4 options - Why bother in a small, embedded library?
//    if(ProcessIcmp(pBuffer + nHeaderLen, nLen - nHeaderLen - 2)) //!@todo This is wrong - only have header here
//        return nLen; //ICMP message so consume whole packet
}

void IPV4::ProcessArp(uint16_t nLen)
{
    byte pBuffer[30];
    m_pInterface->RxGetData(pBuffer, 30);
    //ARP message
    if(pBuffer[ARP_OPER] == ARP_REQUEST)
    {
        if(m_pLocalIp->GetAddress() != pBuffer + ARP_TPA)
            return; //Not for me

        //ARP request - Create response, reusing Rx buffer
        pBuffer[ARP_OPER] = 2; //Change type to reply
        //Set MAC addresses
        memcpy(pBuffer + ARP_THA, pBuffer + ARP_SHA, 6);
        m_pInterface->GetMac(pBuffer + ARP_SHA);
        //Swap sender and target IP
        byte pTmp[4];
        memcpy(pTmp, pBuffer + ARP_SPA, 4);
        memcpy(pBuffer + ARP_SPA, pBuffer + ARP_TPA, 4);
        memcpy(pBuffer + ARP_TPA, pTmp, 4);
        m_pInterface->TxBegin();
        m_pInterface->TxAppend(pBuffer, 30);
        #ifdef _DEBUG_
        Serial.print("Sent ARP reply to ");
        PrintIp(pTmp);
        Serial.println();
        #endif // _DEBUG_
    }
    else if(pBuffer[7] == ARP_RESPONSE)
    {
        //ARP reply
        for(byte i = 0; i < ARP_TABLE_SIZE; ++i)
        {
            if(0 == memcmp(pBuffer + 14, m_aArpTable[i].ip, 4))
            {
                //Found entry in ARP table so update table with MAC address
                memcpy(m_aArpTable[i].mac, pBuffer + 8, 6);
                break;
            }
        }
    }
}

bool IPV4::ProcessIcmp(byte* pData, uint16_t nLen)
{
//    if(nLen < ICMP_HEADER_SIZE)
//        return false;
//    byte nType = pData[0];
////    byte nCode = pData[1];
//    if(ribanEthernetProtocol::CreateChecksum(pData, nLen))
//        return true; //ICMP with invalid checksum
//        //!@todo Create error on checksum error
//    switch(nType)
//    {
//        case ICMP_TYPE_ECHOREPLY:
//            //This is a response to an echo request (ping) so call our hanlder if defined
//            if(m_pHandleEchoResponse)
//                m_pHandleEchoResponse((*(pData + 6) << 8) + (*(pData + 7) & 0xFF)); //!@todo Pass parameters to handler?
//            //!@todo This may be prone to DoS attack by targetting unsolicited echo responses at this host - may be less significant than limited recieve handling
//            break;
//        case ICMP_TYPE_ECHOREQUEST:
//            if(m_bIcmpEnabled)
//            {
//                //This is an echo request (ping) from a remote host so send an echo reply (pong)
//                //Reuse recieve buffer and send reply
//                pData[0] = ICMP_TYPE_ECHOREPLY;
//                pData[ICMP_CHECKSUM_OFFSET] = 0;
//                pData[ICMP_CHECKSUM_OFFSET + 1] = 0;
//                uint16_t nChecksum = ribanEthernetProtocol::CreateChecksum(pData, nLen);
//                pData[ICMP_CHECKSUM_OFFSET] = nChecksum >> 8;;
//                pData[ICMP_CHECKSUM_OFFSET + 1] = nChecksum & 0xFF;
//                SendPacket(pData, nLen, IP_PROTOCOL_ICMP);
//                break;
//            }
//        default:
//            //Unhandled messgae types
//            ;
//    }
    return true; //Valid ICMP message
}

//void IPV4::SendPacket(TxListEntry* pTxListEntry, byte nProtocol, byte* pDestination)
//{
//    byte pHeader[IPV4_HEADER_SIZE + 2];
//    memset(pHeader, 0, IPV4_HEADER_SIZE + 2);
//    pHeader[0] = 0x08; //Populate IP version (4) and header size (5 32-bit words (=20))
//    pHeader[2] = 0x45; //Populate IP version (4) and header size (5 32-bit words (=20))
//    pHeader[4] = (IPV4_HEADER_SIZE + pTxListEntry->GetLen()) >> 8; //Populate total length
//    pHeader[5] = (IPV4_HEADER_SIZE + pTxListEntry->GetLen()) & 0xFF;
//    //!@todo We may be able to omit ipv4 packet identification which is mostly used for frameneted packets (which we do not create)
//    pHeader[6] = m_nIdentification >> 8; //Populate packet identification
//    pHeader[7] = m_nIdentification++ & 0xFF;
//    pHeader[10] = 64; //Populate TTL - let's make it quite big as we don't really care how many hops this message takes
//    pHeader[11] = nProtocol;
//    memcpy(pHeader + IPV4_SOURCE_IP_OFFSET + 2, m_pLocalIp, 4); //Always send IPV4 packets from local host's IP address
//    if(pDestination)
//        memcpy(pHeader + IPV4_DESTINATION_IP_OFFSET + 2, pDestination, 4);
//    else //use last recieved packet's source as (return) destination
//        memcpy(pHeader + IPV4_DESTINATION_IP_OFFSET + 2, m_pRemoteIp, 4);
//    uint16_t nChecksum = ribanEthernetProtocol::CreateChecksum(pHeader + 2, IPV4_HEADER_SIZE);
//    pHeader[IPV4_CHECKSUM_OFFSET + 2] = nChecksum >> 8;
//    pHeader[IPV4_CHECKSUM_OFFSET + 3] = nChecksum & 0xFF;
//
//    TxListEntry txlistentryHeader(pHeader, IPV4_HEADER_SIZE + 2, pTxListEntry);
//    //Figure out what IP & MAC address to target
//    /*
//    Is IP local host?
//    Is IP last recieved packet?
//    Is IP subnet broadcast? - Use FF:FF:FF:FF:FF:FF
//    Is IP global broadcast? - Use FF:FF:FF:FF:FF:FF
//    Is IP multicast? Use multicast address???
//    Is IP withn subnet? Lookup remote host MAC
//    Else use gateway router MAC
//    */
//    if(IsLocalIp(pHeader + IPV4_DESTINATION_IP_OFFSET + 2))
//    {
//        //Local host IP so ignore
//        return;
//    }
//    else if(0 == memcmp(pHeader + IPV4_DESTINATION_IP_OFFSET + 2, m_pRemoteIp, 4))
//    {
//        //Same IP address as last recieved message so return to same MAC
//        TxPacket(&txlistentryHeader, m_pRemoteMac);
//        return;
//    }
//    else if(IsBroadcast(pHeader + IPV4_DESTINATION_IP_OFFSET + 2))
//    {
//        //Broadast IP so send to broadcast MAC
//        byte pBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//        TxPacket(&txlistentryHeader, pBroadcast);
//        return;
//    }
//    else if(IsMulticast(pHeader + IPV4_DESTINATION_IP_OFFSET + 2))
//    {
//        //Multicast IP so send to multicast MAC
//        //!@todo Send multicast packet
//    }
//    else if(IsOnLocalSubnet(pHeader + IPV4_DESTINATION_IP_OFFSET + 2))
//    {
//        //On local subnet so find MAC to send direct
//        for(uint16_t i = 0; i < ARP_TABLE_SIZE + 2; ++i)
//        {
//            if(0 == memcmp(pHeader + IPV4_DESTINATION_IP_OFFSET + 2, m_aArpTable[i].ip, 4))
//            {
//                TxPacket(&txlistentryHeader, m_aArpTable[i].mac);
//                return;
//            }
//        }
//        //!@todo ARP lookup of remote host then (somehow) send message
//        /*
//        Send ARP lookup
//        Populate Tx buffer using m_pInterface->TxBegin and m_pInterface->TxAppend
//        Wait for reply
//        If reply, send packet using m_pInterface->TxEnd
//        */
//        ArpLookup(pHeader + IPV4_DESTINATION_IP_OFFSET + 2);
//    }
//    else if(m_aArpTable[ARP_GATEWAY_INDEX].ip[0])
//    {
//        //Use gateway if it is defined
//        TxPacket(&txlistentryHeader, m_aArpTable[ARP_GATEWAY_INDEX].mac);
//    }
//    else
//    {
//        //Haven't been able to send packet - what to do now?
//    }
//}

void IPV4::ConfigureStaticIp(const uint8_t* pIp,
                             const uint8_t* pGw,
                             const uint8_t* pDns,
                             const uint8_t* pNetmask)
{
    m_nDhcpStatus = DHCP_DISABLED;
    if (pIp != 0)
        memcpy(m_pLocalIp, pIp, 4);
    if (pGw != 0)
    {
        memcpy(m_aArpTable[ARP_GATEWAY_INDEX].ip, pGw, 4);
        //!@todo lookup gw mac
    }
    if (pDns != 0)
        memcpy(m_aArpTable[ARP_DNS_INDEX].ip, pDns, 4);
        //!@todo lookup dns gw
    if(pNetmask != 0)
        memcpy(m_pNetmask, pNetmask, 4);
    //Update broadcast address
    for(byte i = 0; i < 4; ++i)
        m_pBroadcastIp[i] = *(m_pLocalIp->GetAddress() + i) | ~m_pNetmask[i];
    //Update subnet address
    for(byte i = 0; i < 4; ++i)
        m_pSubnetIp[i] = *(m_pLocalIp->GetAddress() + i) & m_pNetmask[i];
}

void IPV4::ConfigureDhcp()
{
    m_nDhcpStatus = DHCP_RESET;
    byte pPayload[DHCP_PACKET_SIZE] = {0x01, 0x01, 0x06, 0x00, 0x39, 0x03, 0xF3, 0x26};
    memset(pPayload, 8, DHCP_PACKET_SIZE - 8);
//    memcpy(pPayload + 28, m_pLocalMac, 6);
    pPayload[DHCP_PACKET_SIZE - 4] = 0x63; //Magic packet
    pPayload[DHCP_PACKET_SIZE - 3] = 0x82;
    pPayload[DHCP_PACKET_SIZE - 2] = 0x53;
    pPayload[DHCP_PACKET_SIZE - 1] = 0x63;
    //!@todo Send UDP packet
    //!@todo Wait for offer
    //!@todo Send request
    //!@todo Wait for acknowledge
    //!@todo Assign IP, netmask, etc
}

uint16_t IPV4::Ping(byte* pIp, void (*HandleEchoResponse)(uint16_t nSequence))
{
    byte pPayload[32] = {8}; //Populate type=8 (echo request)
    memset(pPayload + 1, 0, 5); //Clear next 5 bytes (code, checksum, identifier)
    pPayload[6] = (byte)(m_nPingSequence >> 8); //Populate 16-bit sequence number
    pPayload[7] = (byte)(m_nPingSequence & 0xFF);
    for(byte i = 8; i < 32; ++i)
        pPayload[i] = i; //Populate payload with gash but disernable data (not actually used beyond checksum checking) - we use sequential numbers 8 - 31
//    uint16_t nChecksum = ribanEthernetProtocol::CreateChecksum(pPayload, 32);
//    pPayload[2] = nChecksum >> 8; //Populate checksum (which is calcuated on payload with checksum filed set to zero)
//    pPayload[3] = nChecksum & 0xFF;
    m_pHandleEchoResponse = HandleEchoResponse; //Populate echo response event handler

//    SendPacket(pPayload, sizeof(pPayload), IP_PROTOCOL_ICMP, pIp);

    return m_nPingSequence++; //Return this sequence number and increment for next ping
}

void IPV4::EnableIcmp(bool bEnable)
{
    m_bIcmpEnabled = bEnable;
}

bool IPV4::IsLocalIp(byte* pIp)
{
    return(0 == memcmp(pIp, m_pLocalIp, 4));
}

bool IPV4::IsOnLocalSubnet(byte* pIp)
{
    for(byte i = 0; i < 4; ++i)
        if((m_pSubnetIp[i] & pIp[i]) != m_pSubnetIp[i])
            return false;
    return true;
}

bool IPV4::IsBroadcast(byte* pIp)
{
    bool bReturn = true;
    for(byte i = 0; i < 4; ++i)
        bReturn &= (*(pIp + i) == 255);
    bReturn |= (0 == memcmp(pIp, m_pBroadcastIp, 4));
    return bReturn;
}

bool IPV4::IsMulticast(byte* pIp)
{
    return((*pIp & 0xE0) == 0xE0);
}

byte* IPV4::ArpLookup(byte* pIp, uint16_t nTimeout)
{
    byte pArpPacket[44] = {0x08,0x06,0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01}; //Populate Ethernet type, H/W type, Protocol type, H/W address len, Protocol address len, Operation
//    memcpy(pArpPacket + 10, m_pLocalMac, 6);
    memcpy(pArpPacket + 16, m_pLocalIp, 4);
    memcpy(pArpPacket + 26, pIp, 4);
    byte pBroadcast[6] = {255,255,255,255,255,255};
//    TxListEntry txListEntry(pArpPacket, 42);
//    TxPacket(&txListEntry, pBroadcast);
    uint16_t nExpire = millis() + nTimeout; //Store expiry time
//    while(nExpire > millis())
//        Process();
    //!@todo get ARP to return
    //!@todo detect ARP response

    return NULL;
}

void IPV4::PrintIp(byte* pIp)
{
    Serial.print(*pIp);
    Serial.print(".");
    Serial.print(*(pIp + 1));
    Serial.print(".");
    Serial.print(*(pIp + 2));
    Serial.print(".");
    Serial.print(*(pIp + 3));
}


void IPV4::TxBegin(Address* pDestination)
{
    /*
    Call parent TxBegin()
    Populate header
    */
    //!@todo Derive MAC from pDestination (IP address)


    byte pBuffer[20] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; //Buffer for header. Start by using first 6 bytes for source and destination MAC
    m_pInterface->GetMac(pBuffer + 6); //!@todo Should TxBegin populate source MAC?
    m_pInterface->TxBegin();
    m_pInterface->TxAppend(pBuffer, 12); //Populate destination and source MAC
    memset(pBuffer, 0, sizeof(pBuffer)); //Clear buffer for rest of header
    pBuffer[0] = 0x08; //Ether type IP
    pBuffer[1] = 0x00;
    m_pInterface->TxAppend(pBuffer, 2); //Populate Ether Type
    //Start IPV4 header
    pBuffer[0] = 0x45; //IPV4
    pBuffer[3] = 20 + 8; //IP length = header (20) + payload (8) = 28 (0x1C)
    pBuffer[8] = 64; //TTL 0x40
    //pBuffer[9] = nProtocol; //IP Protocol, e.g. UDP=0x11
    pBuffer[10] = 0x00; //Checksum - clear to allow correct generation by hardware
    pBuffer[11] = 0x00;
    memcpy(pBuffer + IPV4_SOURCE_IP_OFFSET, m_pLocalIp, 4);
    if(pDestination)
        memcpy(pBuffer + IPV4_DESTINATION_IP_OFFSET, pDestination->GetAddress(), pDestination->GetSize());
    else
        m_pInterface->DMACopy(14 + IPV4_SOURCE_IP_OFFSET, 14 + IPV4_SOURCE_IP_OFFSET + 4, IPV4_DESTINATION_IP_OFFSET);
    m_pInterface->TxAppend(pBuffer, 20); //Populate IP header
    //Total length and header checksum are not set. Need to set these in TxEnd().
    //Protocol not set. Need to set this in child protocol routine???
}

void IPV4::TxEnd(uint16_t nLen)
{
    //!@todo Implement TxEnd
    /*
    Add any end-of-datagram info, e.g. checksum
    Populate payload specific elements in header, e.g. size, CRC, etc
    Call parent TxEnd()
    */
//    pHeader[4] = (IPV4_HEADER_SIZE + pTxListEntry->GetLen()) >> 8; //Populate total length
//    pHeader[5] = (IPV4_HEADER_SIZE + pTxListEntry->GetLen()) & 0xFF;

}
