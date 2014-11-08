#include "ribanENC28J60.h"
#include "enc28j60.h"
#include <Arduino.h>

bool ribanENC28J60::Initialise(Address &addressMac, byte nChipSelectPin)
{
    m_nChipSelectPin = nChipSelectPin;
    m_nNicVersion = 0;
    #ifdef IP4
    ipv4.Initialise(&m_nic),
    #endif // IP4
    #ifdef IP6
    ipv6.Initialise(&m_nic),
    #endif // IP6
    m_pHandleTxError = NULL;
    m_addressLocalMac = addressMac;
    m_nNicVersion = m_nic.Initialize(addressMac.GetAddress(), nChipSelectPin);
    return (0 != m_nNicVersion);
}


byte ribanENC28J60::GetNicVersion()
{
    return m_nNicVersion;
}

byte ribanENC28J60::Process()
{
    #define _DEBUG_
    if(0 == m_nNicVersion)
        return 0; //Not correctly initialised so do nothing

    byte nRxCnt = 0;
    while(uint16_t nQuant = m_nic.RxBegin())
    {
        if(nQuant >= MAC_HEADER_SIZE)
        {
            //Get Ethertype from Ethernet header - ignore destination and source MAC for now
            uint16_t nType = m_nic.RxGetWord(MAC_OFFSET_TYPE);
            #ifdef _DEBUG_
            Serial.print("Packet length: ");
            Serial.println(nQuant);
            Serial.print("Rx packet type: ");
            Serial.println(nType, HEX);
            #endif // _DEBUG_
            switch(nType)
            {
                #ifdef IP4
                case ETHTYPE_ARP:
                    #ifdef _DEBUG_
                    Serial.println("ARP packet recieved");
                    #endif //_DEBUG_
                    ipv4.ProcessArp(nQuant); //!@todo Consider ARP messages for other protocols
                    break;
                case ETHTYPE_IPV4:
                    #ifdef _DEBUG_
                    Serial.println("IPV4 packet recieved");
                    #endif //_DEBUG_
                    ipv4.Process(nQuant - MAC_HEADER_SIZE);
                    break;
                #endif // IP4
                #ifdef IP6
                case ETHTYPE_IPV6:
                    #ifdef _DEBUG_
                    Serial.println("IPV6 packet recieved");
                    #endif //_DEBUG_
                    m_pIpv6->Process(nType, nQuant - MAC_HEADER_SIZE);
                    break;
                #endif // IP6
            }
        }
        if(m_pHandleTxError && m_nic.TxGetStatus() == ENC28J60_TX_FAILED)
        {
            m_pHandleTxError();
            m_nic.TxClearError();
        }
        m_nic.RxEnd();
        ++nRxCnt;
    }
    return nRxCnt;
}

//void ribanENC28J60::TxPacket(TxListEntry* pSendList, byte* pDestination)
//{
//    m_nic.TxBegin(); //Start Tx transaction
//    if(pDestination)
//        m_nic.TxAppend(pDestination, 6); //Use specified destination MAC
//    else
//        m_nic.TxAppend(m_pRemoteMac->GetAddress(), 6); //Use last known remote host MAC
//    m_nic.TxAppend(m_pLocalMac->GetAddress(), 6); //Use our own MAC
//    //Iterate through list of packet data buffers, appending to packet
//    TxListEntry* pNext = pSendList;
//    while(pNext)
//    {
//        m_nic.TxAppend(pNext->GetData(), pNext->GetLen());
//        pNext = pNext->GetNext();
//    }
//    m_nic.TxEnd(); //End Tx transaction and send the packet
//}

void ribanENC28J60::SetTxErrorHandler(void (*HandleTxError)())
{
    m_pHandleTxError = HandleTxError;
}

void ribanENC28J60::TxBegin(Address* pMac, uint16_t nEthertype)
{
    m_nic.TxBegin(pMac?pMac->GetAddress():NULL, nEthertype);
}

bool ribanENC28J60::TxAppend(byte* pData, uint16_t nLen)
{
    return m_nic.TxAppend(pData, nLen);
}

void ribanENC28J60::TxEnd()
{
    m_nic.TxEnd();
}
