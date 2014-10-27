#include "ribanENC28J60.h"
#include "enc28j60.h"
#include <Arduino.h>

static const uint16_t DEST_MAC_OFFSET   = 0;
static const uint16_t SRC_MAC_OFFSET    = 6;
static const uint16_t SIZE_TYPE_OFFSET  = 12;

ribanENC28J60::ribanENC28J60(Address addressMac, byte nChipSelectPin) :
    m_nChipSelectPin(nChipSelectPin)
{
    m_pHandleTxError = NULL;
    m_pLocalMac = new Address(ADDR_TYPE_MAC);
    m_pRemoteMac = new Address(ADDR_TYPE_MAC);

    m_nic.Initialize(addressMac.GetAddress(), nChipSelectPin);

    #ifdef IP4
    m_pIpv4 = new IPV4(&m_nic);
    #endif // IP4
    #ifdef IP6
    m_pIpv6 = new IPV6(&m_nic);
    #endif // IP6

}

ribanENC28J60::~ribanENC28J60()
{
    delete m_pLocalMac;
    delete m_pRemoteMac;
}

uint16_t ribanENC28J60::Process()
{
    /*
        Read each packet
        Identify Ethertype
        Process using protocol handler
    */
    uint16_t nQuant = m_nic.RxBegin();
    if(nQuant >= 14) //minimum Ethernet header size
    {
        //Get Ethertype from Ethernet header - ignore destination and source MAC for now
        uint16_t nType = m_nic.RxGetByte(12);
        switch(nType)
        {
            #ifdef IP4
            case ETHTYPE_ARP:
                m_pIpv4->ProcessArp(nQuant);
                break;
            case ETHTYPE_IPV4:
                m_pIpv4->Process(nQuant);
                break;
            #endif // IP4
            #ifdef IP6
            case ETHTYPE_IPV6:
                m_pIpv6->Process(nType, nQuant);
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
    return nQuant;
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

void ribanENC28J60::TxBegin(byte* pDestination)
{
    m_nic.TxBegin();
    if(pDestination)
        m_nic.TxAppend(pDestination, 6);
    else
        m_nic.TxAppend(m_pRemoteMac->GetAddress(), 6);
    m_nic.TxAppend(m_pLocalMac->GetAddress(), 6);
}

void ribanENC28J60::TxEnd(uint16_t nLen)
{
    //Check if EtherType is define
    uint16_t nType;
    m_nic.TxRead(SIZE_TYPE_OFFSET, (byte*)&nType, 2u);
    if(!nType)
        m_nic.TxWrite(SIZE_TYPE_OFFSET, (byte*)&nLen, 2); //EtherType not defined so use packet length
    m_nic.TxEnd();
}
