#include "ipv4.h"
#include "enc28j60.h"

IPV4::IPV4() :
    m_bIcmpEnabled(true), //Respond to ICMP echo requests (pings) by default
    m_addressLocal(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_addressRemote(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_addressGw(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_addressDns(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_addressMask(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_addressSubnet(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_addressBroadcast(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
    m_nDhcpStatus(DHCP_RESET), //Assume DHCP required until explicit request for static IP
    m_nArpCursor(2), //First two ARP entries are for gateway (router) and DNS
    m_nIdentification(0)
{
}

void IPV4::Initialise(ENC28J60* pInterface)
{
    m_pInterface = pInterface;
}

void IPV4::Process(uint16_t nLen)
{
    if(nLen < IPV4_HEADER_SIZE)
        return;

    byte nProtocol = m_pInterface->RxGetByte(MAC_HEADER_SIZE + IPV4_OFFSET_PROTOCOL);
    m_nHeaderLength = (m_pInterface->RxGetByte(MAC_HEADER_SIZE + IPV4_OFFSET_VERSION) & 0x0F) * 4;
    uint16_t nPayload = m_pInterface->RxGetWord(MAC_HEADER_SIZE + IPV4_OFFSET_LENGTH);
    if(nLen < nPayload)
        return; //!@todo Should we indicate failure to process packet?
    nPayload -= m_nHeaderLength;
    byte nTmp;
    m_pInterface->RxGetData(&nTmp, 1, MAC_HEADER_SIZE + m_nHeaderLength); //Position read cursor at payload
    switch(nProtocol)
    {
        case IP_PROTOCOL_ICMP:
            if(m_bIcmpEnabled)
                ProcessIcmp(nPayload);
            break;
        case IP_PROTOCOL_IGMP:
            #ifdef _DEBUG_
            Serial.println("IPV4 IGMP not handled");
            #endif // _DEBUG_
            break;
        case IP_PROTOCOL_TCP:
            #ifdef _DEBUG_
            Serial.println("IPV4 TCP not handled");
            #endif // _DEBUG_
            break;
        case IP_PROTOCOL_UDP:
            #ifdef _DEBUG_
            Serial.println("IPV4 UDP not handled");
            #endif // _DEBUG_
            break;
        default:
            #ifdef _DEBUG_
            Serial.print("IPV4 unhandled IP protocol ");
            Serial.println(nProtocol);
            #endif // _DEBUG_
            break;
    }
}

void IPV4::ProcessArp(uint16_t nLen)
{
    #ifdef _DEBUG_
    Serial.println("IPV4::ProcessArp");
    #endif // _DEBUG_
    if(nLen < ARP_IPV4_LEN)
        return;
    byte pBuffer[ARP_IPV4_LEN];
    m_pInterface->RxGetData(pBuffer, sizeof pBuffer, MAC_HEADER_SIZE);
    //Assume ARP header is valid IPV4 ARP
    if(m_pInterface->RxGetWord(MAC_HEADER_SIZE + ARP_OPER) == ARP_REQUEST)
    {
        #ifdef _DEBUG_
        Serial.println("IPV4::ProcessArp ARP Request");
        #endif // _DEBUG_
        if(m_addressLocal != pBuffer + ARP_TPA)
            return; //Not for me

        //!@todo consider whether using DMA would be advantagous

        //ARP request - Create response, reusing Rx buffer
        pBuffer[ARP_OPER + 1] = 2; //Change type to reply
        //Set MAC addresses
        memcpy(pBuffer + ARP_THA, pBuffer + ARP_SHA, 6);
        m_pInterface->GetMac(pBuffer + ARP_SHA);
        //Swap sender and target IP
        byte pTmp[4];
        memcpy(pTmp, pBuffer + ARP_SPA, 4);
        memcpy(pBuffer + ARP_SPA, pBuffer + ARP_TPA, 4);
        memcpy(pBuffer + ARP_TPA, pTmp, 4);
        m_pInterface->TxBegin(NULL, 0x0806);
        m_pInterface->TxAppend(pBuffer, ARP_IPV4_LEN);
        m_pInterface->TxEnd();
        #ifdef _DEBUG_
        Serial.print("Sent ARP reply to ");
        Address pIp(ADDR_TYPE_IPV4, pTmp);
        pIp.PrintAddress();
        Serial.println();
        #endif // _DEBUG_
    }
    else if(m_pInterface->RxGetWord(MAC_HEADER_SIZE + ARP_OPER) == ARP_REPLY)
    {
        #ifdef _DEBUG_
        Serial.println("IPV4::ProcessArp ARP Reply");
        #endif // _DEBUG_
        //Search ARP table for IP address
        for(byte i = 0; i < ARP_TABLE_SIZE; ++i)
        {
            if(0 == memcmp(pBuffer + ARP_SPA, m_aArpTable[i].ip, 4))
            {
                //Found entry in ARP table so update table with MAC address
                memcpy(m_aArpTable[i].mac, pBuffer + ARP_SHA, 6);
                break;
            }
        }
    }
    else
    {
        #ifdef _DEBUG_
        Serial.print("IPV4::ProcessArp Unhandled ARP message with OPER=");
        Serial.println(m_pInterface->RxGetWord(MAC_HEADER_SIZE + ARP_OPER));
        #endif // _DEBUG_
    }
}

void IPV4::GetRemoteIp(Address& address)
{
    byte pBuffer[4];
    m_pInterface->RxGetData(pBuffer, 4);
    address.SetAddress(pBuffer);
}

bool IPV4::ProcessIcmp(uint16_t nLen)
{
    #ifdef _DEBUG_
    Serial.println("IPV4::ProcessIcmp");
    #endif // _DEBUG_
    if(nLen < ICMP_HEADER_SIZE)
        return false;
    m_pInterface->DMACopy(0, MAC_HEADER_SIZE + m_nHeaderLength, nLen); //Populate TxBuffer with ICMP header and payload (not Ethernet or IPV4 header)
    m_pInterface->TxWriteWord(ICMP_OFFSET_CHECKSUM, 0); //Clear checksum field
    uint16_t nRxChecksum = m_pInterface->RxGetWord(MAC_HEADER_SIZE + m_nHeaderLength + ICMP_OFFSET_CHECKSUM);
    uint16_t nCalcChecksum = ENC28J60::SwapBytes(m_pInterface->GetChecksum(0, nLen)); //Calculate checksum of ICMP header and payload in TxBuffer
    if(nRxChecksum != nCalcChecksum)
       return false; //Fails checksum
    #ifdef _DEBUG_
    #endif // _DEBUG_
    switch(m_pInterface->RxGetByte(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + ICMP_OFFSET_TYPE))
    {
        case ICMP_TYPE_ECHOREPLY:
            //This is a response to an echo request (ping) so call our hanlder if defined
            #ifdef _DEBUG_
            Serial.println("Echo reply");
            #endif // _DEBUG_
            if(m_pHandleEchoResponse)
                m_pHandleEchoResponse(m_pInterface->RxGetWord(MAC_HEADER_SIZE + m_nHeaderLength + 6)); //!@todo Pass parameters to handler?
            //!@todo This may be prone to DoS attack by targetting unsolicited echo responses at this host - may be less significant than limited recieve handling - Just check we are expecting it in handler?
            break;
        case ICMP_TYPE_ECHOREQUEST:
            //This is an echo request (ping) from a remote host so send an echo reply (pong)
            #ifdef _DEBUG_
            Serial.println("Echo request");
            #endif // _DEBUG_
            //Reuse recieve buffer and send reply
            m_pInterface->TxBegin();
            m_pInterface->DMACopy(0, 0, MAC_HEADER_SIZE + IPV4_HEADER_SIZE + nLen);
            m_pInterface->TxSwap(MAC_OFFSET_DESTINATION, MAC_OFFSET_SOURCE, 6);
            m_pInterface->TxSwap(MAC_HEADER_SIZE + IPV4_OFFSET_DESTINATION, MAC_HEADER_SIZE + IPV4_OFFSET_SOURCE, 4);
            m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + ICMP_OFFSET_TYPE, ICMP_TYPE_ECHOREPLY);
            m_pInterface->TxWriteWord(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + ICMP_OFFSET_CHECKSUM, 0);
            m_pInterface->TxWriteWord(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + ICMP_OFFSET_CHECKSUM, m_pInterface->GetChecksum(MAC_HEADER_SIZE + IPV4_HEADER_SIZE, nLen));
            m_pInterface->TxEnd();
            break;
        default:
            //Unhandled message types
            #ifdef _DEBUG_
            Serial.println("Unhandled ICMP message");
            #endif // _DEBUG_
            ;
    }
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

void IPV4::ConfigureStaticIp(Address *pIp,
                             Address *pGw,
                             Address *pDns,
                             Address *pNetmask)
{
    m_nDhcpStatus = DHCP_DISABLED;
    if(pIp != 0)
        m_addressLocal.SetAddress(pIp->GetAddress());
    if(pGw != 0)
    {
        memcpy(m_aArpTable[ARP_GATEWAY_INDEX].ip, pGw->GetAddress(), 4);
        //!@todo lookup gw mac
    }
    if(pDns != 0)
        memcpy(m_aArpTable[ARP_DNS_INDEX].ip, pDns->GetAddress(), 4);
        //!@todo lookup dns gw
    if(pNetmask != 0)
        m_addressMask.SetAddress(pNetmask->GetAddress());
    //Update broadcast address
    for(byte i = 0; i < 4; ++i)
        m_addressBroadcast.GetAddress()[i] = m_addressLocal.GetAddress()[i] | ~m_addressMask.GetAddress()[i];
    //Update subnet address
    for(byte i = 0; i < 4; ++i)
        m_addressSubnet.GetAddress()[i] = m_addressLocal.GetAddress()[i] & m_addressMask.GetAddress()[i];
}

void IPV4::ConfigureDhcp()
{
    m_nDhcpStatus = DHCP_RESET;
    byte pNull[] = {0,0,0,0};
    byte pBroadcast[] = {255,255,255,255};
    m_addressBroadcast.SetAddress(pBroadcast);
    m_addressLocal.SetAddress(pNull);
    //!@todo Clear other addresses?
    TxBegin(&m_addressBroadcast, IP_PROTOCOL_UDP);
    TxAppend((uint16_t)68); //UDP source port
    TxAppend((uint16_t)67); //UDP destination port
    TxAppend(DHCP_PACKET_SIZE);
    TxAppend(pNull, 2); //Clear checksum
    byte pPayload[DHCP_PACKET_SIZE] = {0x01, 0x01, 0x06, 0x00, 0x39, 0x03, 0xF3, 0x26};
    m_pInterface->GetMac(pPayload + 8);
    memset(pPayload, 0, DHCP_PACKET_SIZE - 18); //Pad rest of packet with null values up to magic packet
//    memcpy(pPayload + 28, m_pLocalMac, 6);
    pPayload[DHCP_PACKET_SIZE - 4] = 0x63; //Magic packet
    pPayload[DHCP_PACKET_SIZE - 3] = 0x82;
    pPayload[DHCP_PACKET_SIZE - 2] = 0x53;
    pPayload[DHCP_PACKET_SIZE - 1] = 0x63;
    TxAppend(pPayload, DHCP_PACKET_SIZE);
    TxEnd();

    //!@todo Send UDP packet
    //!@todo Wait for offer
    //!@todo Send request
    //!@todo Wait for acknowledge
    //!@todo Assign IP, netmask, etc
}

uint16_t IPV4::Ping(Address* pIp, void (*HandleEchoResponse)(uint16_t nSequence))
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
    return(m_addressLocal == pIp);
}

bool IPV4::IsOnLocalSubnet(byte* pIp)
{
    for(byte i = 0; i < 4; ++i)
        if((m_addressSubnet.GetAddress()[i] & pIp[i]) != m_addressSubnet.GetAddress()[i])
            return false;
    return true;
}

bool IPV4::IsBroadcast(byte* pIp)
{
    bool bReturn = true;
    for(byte i = 0; i < 4; ++i)
        bReturn &= (*(pIp + i) == 255);
    return bReturn | (m_addressBroadcast == pIp);
}

bool IPV4::IsMulticast(byte* pIp)
{
    return((*pIp & 0xE0) == 0xE0);
}

byte* IPV4::ArpLookup(byte* pIp, uint16_t nTimeout)
{
    byte pArpPacket[44] = {0x08,0x06,0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01}; //Populate Ethernet type, H/W type, Protocol type, H/W address len, Protocol address len, Operation
//    memcpy(pArpPacket + 10, m_pLocalMac, 6);
    memcpy(pArpPacket + 16, m_addressLocal.GetAddress(), 4);
    memcpy(pArpPacket + 26, pIp, 4);
    byte pBroadcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
//    TxListEntry txListEntry(pArpPacket, 42);
//    TxPacket(&txListEntry, pBroadcast);
    uint16_t nExpire = millis() + nTimeout; //Store expiry time
//    while(nExpire > millis())
//        Process();
    //!@todo get ARP to return
    //!@todo detect ARP response

    return NULL;
}

void IPV4::TxBegin(Address* pDestination, uint16_t nProtocol)
{
    //!@todo Derive MAC from pDestination (IP address)

    m_pInterface->TxBegin();
    //Clear IPV4 header
    byte nZero = 0;
    for(byte nOffset = 0; nOffset < IPV4_HEADER_SIZE; ++nOffset)
        m_pInterface->TxAppend(&nZero, 1);

    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_OFFSET_VERSION, 0x45);
    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_OFFSET_TTL, 64);
    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_OFFSET_PROTOCOL, nProtocol);
    m_pInterface->TxWrite(MAC_HEADER_SIZE + IPV4_OFFSET_SOURCE, m_addressLocal.GetAddress(), 4);
    if(pDestination)
        m_pInterface->TxWrite(MAC_HEADER_SIZE + IPV4_OFFSET_DESTINATION, pDestination->GetAddress(), 4);
    else
        m_pInterface->DMACopy(MAC_HEADER_SIZE + IPV4_OFFSET_DESTINATION, MAC_HEADER_SIZE + IPV4_OFFSET_SOURCE, 4);
    m_nTxPayload = 0;
}

bool IPV4::TxAppend(byte nData)
{
    if(m_pInterface->TxAppend(&nData, 1))
    {
        ++m_nTxPayload;
        return true;
    }
    return false;
}

bool IPV4::TxAppend(uint16_t nData)
{
    if(m_pInterface->TxAppend((byte*)&nData, 2))
    {
        m_nTxPayload += 2;
        return true;
    }
    return false;
}

bool IPV4::TxAppend(byte* pData, uint16_t nLen)
{
    if(m_pInterface->TxAppend(pData, nLen))
    {
        m_nTxPayload += nLen;
        return true;
    }
    return false;
}

void IPV4::TxWrite(uint16_t nOffset, byte nData)
{
    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + nOffset, nData);
    m_nTxPayload = max(m_nTxPayload, nOffset);
}

void IPV4::TxWrite(uint16_t nOffset, uint16_t nData)
{
    m_pInterface->TxWriteWord(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + nOffset, nData);
    m_nTxPayload = max(m_nTxPayload, nOffset);
}


void IPV4::TxWrite(uint16_t nOffset, byte* pData, uint16_t nLen)
{
    m_pInterface->TxWrite(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + nOffset, pData, nLen);
    m_nTxPayload = max(m_nTxPayload, nOffset + nLen);
}

void IPV4::TxEnd()
{
    m_pInterface->TxWriteWord(MAC_HEADER_SIZE + IPV4_OFFSET_ID, m_nIdentification++);
    m_pInterface->TxWriteWord(MAC_HEADER_SIZE + IPV4_OFFSET_LENGTH, IPV4_HEADER_SIZE + m_nTxPayload);
    m_pInterface->TxWriteWord(MAC_HEADER_SIZE + IPV4_OFFSET_CHECKSUM, ENC28J60::SwapBytes(m_pInterface->GetChecksum(MAC_HEADER_SIZE, IPV4_HEADER_SIZE)));
    m_pInterface->TxEnd();
}

