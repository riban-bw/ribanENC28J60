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
    m_addressDhcp(ADDR_TYPE_IPV4), //!@todo Is this right? Initialse object in constructor with parenthesis?
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
    if(m_timerDhcp.IsTriggered())
        SendDhcpPacket(DHCP_RENEWING);

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
            ProcessUdp(nPayload);
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

byte IPV4::ProcessArp(uint16_t nLen)
{
    #ifdef _DEBUG_
    Serial.println("IPV4::ProcessArp");
    #endif // _DEBUG_
    if(nLen < ARP_IPV4_LEN)
        return ARP_EOF;
    byte pBuffer[ARP_IPV4_LEN];
    m_pInterface->RxGetData(pBuffer, sizeof pBuffer, MAC_HEADER_SIZE);
    //Assume ARP header is valid IPV4 ARP
    if(m_pInterface->RxGetWord(MAC_HEADER_SIZE + ARP_OPER) == ARP_REQUEST)
    {
        #ifdef _DEBUG_
        Serial.println("IPV4::ProcessArp ARP Request");
        #endif // _DEBUG_
        if(m_addressLocal != pBuffer + ARP_TPA)
            return ARP_EOF; //Not for me

        //!@todo Consider whether using DMA would be advantagous within IPV4::ProcessArp

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
        for(byte i = 0; i < ARP_TABLE_SIZE + 2; ++i)
        {
            if(0 == memcmp(pBuffer + ARP_SPA, m_aArpTable[i].ip, 4))
            {
                //Found entry in ARP table so update table with MAC address
                memcpy(m_aArpTable[i].mac, pBuffer + ARP_SHA, 6);
                return i;
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
    return ARP_EOF;
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

void IPV4::ProcessUdp(uint16_t nLen)
{
    #define _DEBUG_
    #ifdef _DEBUG_
    Serial.println("IPV4::ProcessUdp");
    #endif // _DEBUG_
    byte pBuffer[UDP_HEADER_SIZE];
    m_pInterface->RxGetData(pBuffer, UDP_HEADER_SIZE);
    //Check for DHCP
    if((DHCP_CLIENT_PORT == pBuffer[UDP_OFFSET_DESTINATION_PORT]) && DHCP_DISCOVERY == m_nDhcpStatus)
    {
        //Expecting DHCP OFFER and recieved a DHCP message
        #ifdef _DEBUG_
        Serial.println("Recieved DHCP offer");
        #endif // _DEBUG_
        if(m_pInterface->RxGetByte() != 2)
            return; //!@todo Should we bother to check for OP code when all messages targetted at port 68 should be from server to client?
        if(!FindDhcpOption(53, nLen))
            return; //Not a DHCP offer
        //Store IP/MAC in ARP table
        m_pInterface->RxGetData(m_aArpTable[m_nArpCursor].ip, MAC_HEADER_SIZE + IPV4_HEADER_SIZE + DHCP_OFFSET_SIADDR, 4);
        if(m_nArpCursor++ >= ARP_TABLE_SIZE + 2)
            m_nArpCursor = 2;
        //Store local IP and DHCP server IP addresses
        m_pInterface->RxGetData(m_addressLocal.GetAddress(), 4, MAC_HEADER_SIZE + m_nHeaderLength + UDP_HEADER_SIZE + DHCP_OFFSET_YIADDR); //!@todo Should we store this during offer? Used by request but maybe we should clear during request and set during acknowledge
        m_pInterface->RxGetData(m_addressDhcp.GetAddress(), 4, MAC_HEADER_SIZE + m_nHeaderLength + UDP_HEADER_SIZE + DHCP_OFFSET_SIADDR);
        SendDhcpPacket(DHCP_REQUESTED);
    }
    else if((DHCP_CLIENT_PORT == pBuffer[UDP_OFFSET_DESTINATION_PORT]) && DHCP_REQUESTED == m_nDhcpStatus)
    {
        //Expecting DHCP ACK and recieved a DHCP message
        #ifdef _DEBUG_
        Serial.println("Recieved DHCP acknowledgement");
        #endif // _DEBUG_
        //Check this is an acknowledgement
        if(!FindDhcpOption(DHCP_OPTION_TYPE, nLen))
            return;
        m_pInterface->RxGetByte();
        if(DHCP_TYPE_ACK != m_pInterface->RxGetByte())
            return;
        if(FindDhcpOption(DHCP_OPTION_MASK, nLen))
        {
            m_pInterface->RxGetByte(); //Get length but assume it is correct
            m_pInterface->RxGetData(m_addressMask.GetAddress(), 4); //Set subnet mask
        }
        if(FindDhcpOption(DHCP_OPTION_ROUTER, nLen))
        {
            m_pInterface->RxGetByte(); //Get length but assume it is correct
            m_pInterface->RxGetData(m_addressGw.GetAddress(), 4); //Set gateway router address
        }
        if(FindDhcpOption(DHCP_OPTION_LEASE, nLen))
        {
            m_pInterface->RxGetByte(); //Get length but assume it is correct
            uint32_t lLease = ((m_pInterface->RxGetWord()) & (uint32_t(m_pInterface->RxGetWord()) << 16)) / 2 + millis();
            m_timerDhcp.start(lLease & 0xFFFF);
        }
        if(FindDhcpOption(DHCP_OPTION_DNS, nLen))
        {
            m_pInterface->RxGetByte(); //Get length but assume it is correct
            m_pInterface->RxGetData(m_addressDns.GetAddress(), 4); //Set DNS to first offered DNS (this class only supports one DNS server
        }
        m_pInterface->RxGetData(m_addressLocal.GetAddress(), 4, MAC_HEADER_SIZE + m_nHeaderLength + UDP_HEADER_SIZE + DHCP_OFFSET_YIADDR); //Set local IP
        m_nDhcpStatus = DHCP_BOUND; //Our work here is done - until lease renewal
    }
    //!@todo Process UDP listening sockets
}

void IPV4::SendDhcpPacket(byte nType)
{
    byte pBuffer[] = {0,0,0,0,0,0};
    byte pBroadcast[] = {255,255,255,255};
    m_addressBroadcast.SetAddress(pBroadcast); //Set our broadcast address to the IPV4 global broadcast
    if(DHCP_DISCOVERY == nType)
    {
        m_addressLocal.SetAddress(pBuffer); //Reset our local IP address
        //!@todo Clear other addresses?
    }
    if(DHCP_RENEWING == nType)
        TxBegin(&m_addressDhcp, IP_PROTOCOL_UDP);
    else
        TxBegin(&m_addressBroadcast, IP_PROTOCOL_UDP);
    //Write UDP header
    //!@todo Implement TxUdpBegin and TxUdpEnd?
    TxAppendWord(DHCP_CLIENT_PORT); //UDP source port
    TxAppendWord(DHCP_SERVER_PORT); //UDP destination port
    if(DHCP_DISCOVERY == nType)
        TxAppendWord(UDP_HEADER_SIZE + DHCP_OFFSET_OPTIONS + 10); //Options exclude requested IP
    else
        TxAppendWord(UDP_HEADER_SIZE + DHCP_OFFSET_OPTIONS + 16); //Options include requested IP
    TxAppendWord(0x0000); //Clear checksum
    //Write DHCP Discover message
    TxAppendByte(0x01); //Boot request
    TxAppendByte(0x01); //Ethernet
    TxAppendByte(0x06); //Hardware address length
    TxAppendByte(0x00); //Hops
    TxAppendByte(0x44); //Random transaction ID
    TxAppendByte(0x48); //!@todo Do we need to store DHCP transaction ID or do we use same one every time?
    TxAppendByte(0x43);
    TxAppendByte(0x50);
    for(uint16_t nPos = 0; nPos < 20; ++nPos)
        TxAppendByte(0);
    //!@todo Write own address for request
    m_pInterface->GetMac(pBuffer);
    TxAppend(pBuffer, 6); //Write own MAC
    for(uint16_t nPos = 0; nPos < 202; ++nPos)
        TxAppendByte(0); //Pad with zeros
    TxAppendByte(0x63); //Magic cookie
    TxAppendByte(0x82); //Magic cookie
    TxAppendByte(0x53); //Magic cookie
    TxAppendByte(0x63); //Magic cookie
    //DHCP Options
    //Adjust DHCP_PACKET_SIZE to suit quantity of options. Currently DHCP_OFFSET_OPTIONS(240) + 9
    TxAppendByte(53); //Option 53: Message type
    TxAppendByte(1);
    switch(nType)
    {
        case DHCP_DISCOVERY:
            TxAppendByte(DHCP_TYPE_DISCOVER);
            break;
        case DHCP_REQUESTED:
        case DHCP_RENEWING: //!@todo Check renewal is same as request
            TxAppendByte(DHCP_TYPE_REQUEST);
            break;
    }
    if(nType != DHCP_DISCOVERY)
    {
        TxAppendByte(50); //Option 50: Requested IP
        TxAppendByte(4); //Length of IP address = 4
        TxAppend(m_addressLocal.GetAddress(), 4);
    }
    TxAppendByte(55); //Option 55: Parameter Request List
    TxAppendByte(4); //Length 3
    TxAppendByte(1); //Request Subnet Mask
    TxAppendByte(3); //Request Router
    TxAppendByte(6); //Request DNS
    TxAppendByte(51); //Request lease time
    TxAppendByte(255); //Option 255: END
    TxEnd(); //Send DHCP discover
    if(DHCP_REQUESTED == nType)
    {
        //Blank local IP address until DHCP acknowledge recieved
        byte pNull[] = {0,0,0,0};
        m_addressLocal.SetAddress(pNull);
    }
    m_nDhcpStatus = nType;
}

bool IPV4::FindDhcpOption(byte nOption, uint16_t nLen)
{
    uint16_t nPos = MAC_HEADER_SIZE + IPV4_HEADER_SIZE + UDP_HEADER_SIZE + DHCP_OFFSET_OPTIONS;
    byte nResult = m_pInterface->RxGetByte(nPos);
    while((nPos < MAC_HEADER_SIZE + IPV4_HEADER_SIZE + UDP_HEADER_SIZE + nLen) && (nResult != 0xFF))
    {
        if(nResult == nOption)
            return true;
        byte nLength = m_pInterface->RxGetByte();
        byte pBuffer[nLength];
        m_pInterface->RxGetData(pBuffer, nLen);
        nPos = nPos + nLength + 2;
        nResult = m_pInterface->RxGetByte(nPos);
    }
    return false;
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
    m_timerDhcp.stop();
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
    SendDhcpPacket(DHCP_DISCOVERY);
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

bool IPV4::IsOnLocalSubnet(Address* pIp)
{
    for(byte i = 0; i < 4; ++i)
        if((m_addressSubnet.GetAddress()[i] & pIp->GetAddress()[i]) != m_addressSubnet.GetAddress()[i])
            return false;
    return true;
}

bool IPV4::IsBroadcast(Address* pIp)
{
    bool bReturn = true;
    for(byte i = 0; i < 4; ++i)
        bReturn &= (*(pIp->GetAddress() + i) == 255);
    return bReturn | (m_addressBroadcast == (*pIp));
}

bool IPV4::IsMulticast(byte* pIp)
{
    return((*pIp & 0xE0) == 0xE0);
}

byte* IPV4::ArpLookup(Address* pIp, uint16_t nTimeout)
{
    //Search ARP table
    for(byte nIndex = 0; nIndex < ARP_TABLE_SIZE + 2; ++nIndex)
    {
        if((*pIp) == m_aArpTable[nIndex].ip, 4)
            return m_aArpTable[nIndex].mac;
    }
    //Do ARP lookup
    m_pInterface->TxBegin(NULL, ETHTYPE_ARP);
    m_pInterface->TxAppendWord(0x0001); //HTYPE = Ethernet
    m_pInterface->TxAppendWord(0x0008); //PTYPE = IP (note: TxAppendWord expects host byte order)
    m_pInterface->TxAppendByte(0x06); //HSIZE = Ethernet address (MAC) length
    m_pInterface->TxAppendByte(0x04); //PSIZE = IP address length
    m_pInterface->TxAppendWord(0x0001); //OPCODE = Request
    byte pBuffer[6];
    m_pInterface->GetMac(pBuffer);
    m_pInterface->TxAppend(pBuffer, 6); //Sender MAC
    m_pInterface->TxAppend(m_addressLocal.GetAddress(), 4); //Sender IP
    m_pInterface->TxAppendWord(0x0000); //Target MAC
    m_pInterface->TxAppendWord(0x0000);
    m_pInterface->TxAppend(pIp->GetAddress(), 4); //Target IP
    m_pInterface->TxEnd(); //Send ARP request
    //Add entry to ARP table with empty MAC
    memcpy(m_aArpTable[m_nArpCursor].ip, pIp->GetAddress(), 4);
    memset(m_aArpTable[m_nArpCursor].mac, 0, 6);
    ++m_nArpCursor;
    if(m_nArpCursor >= ARP_TABLE_SIZE + 2)
        m_nArpCursor = 2;
    //Wait for ARP response
    uint16_t nExpire = millis() + nTimeout; //Store expiry time
    while(nExpire > millis())
    {
        //Will only run if nTimeout set but will block and disguard all recieved packets until ARP response or timeout
        if(m_pInterface->RxBegin() >= int16_t(MAC_HEADER_SIZE + ARP_IPV4_LEN))
        {
            byte nIndex;
            if((nIndex = ProcessArp(ARP_IPV4_LEN)) != ARP_EOF)
                   return m_aArpTable[nIndex].mac;
        }
    }
    return NULL;
}

void IPV4::TxBegin(Address* pTarget, uint16_t nProtocol)
{
    if(IsBroadcast(pTarget))
        m_pInterface->TxBegin();
    else if(IsOnLocalSubnet(pTarget))
        m_pInterface->TxBegin(ArpLookup(pTarget)); //Begin Tx transaction with MAC address of target host or broadcast if ARP fails
    else
        m_pInterface->TxBegin(ArpLookup(&m_addressGw));
    //Clear IPV4 header
    byte nZero = 0;
    for(byte nOffset = 0; nOffset < IPV4_HEADER_SIZE; ++nOffset)
        m_pInterface->TxAppend(&nZero, 1);

    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_OFFSET_VERSION, 0x45);
    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_OFFSET_TTL, 64);
    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_OFFSET_PROTOCOL, nProtocol);
    m_pInterface->TxWrite(MAC_HEADER_SIZE + IPV4_OFFSET_SOURCE, m_addressLocal.GetAddress(), 4);
    if(pTarget)
        m_pInterface->TxWrite(MAC_HEADER_SIZE + IPV4_OFFSET_DESTINATION, pTarget->GetAddress(), 4);
    else
        m_pInterface->DMACopy(MAC_HEADER_SIZE + IPV4_OFFSET_DESTINATION, MAC_HEADER_SIZE + IPV4_OFFSET_SOURCE, 4);
    m_nTxPayload = 0;
}

bool IPV4::TxAppendByte(byte nData)
{
    if(m_pInterface->TxAppendByte(nData))
    {
        ++m_nTxPayload;
        return true;
    }
    return false;
}

bool IPV4::TxAppendWord(uint16_t nData)
{
    if(m_pInterface->TxAppendWord(nData))
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

void IPV4::TxWriteByte(uint16_t nOffset, byte nData)
{
    m_pInterface->TxWriteByte(MAC_HEADER_SIZE + IPV4_HEADER_SIZE + nOffset, nData);
    m_nTxPayload = max(m_nTxPayload, nOffset);
}

void IPV4::TxWriteWord(uint16_t nOffset, uint16_t nData)
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

