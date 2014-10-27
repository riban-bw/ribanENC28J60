#include "socket.h"
#include "ribanENC28J60.h"

Socket::Socket(byte nDomain, ribanEthernet* pInterface, byte nProtocol) :
    m_nProtocol(nProtocol),
    m_pInterface(pInterface)
{
    switch(nDomain)
    {
        case AF_INET:
            break;
        case AF_INET6:
            break;
        case AF_PACKET:
            break;
    }
//    m_pInterface->AddSocket(this);
}

Socket::~Socket()
{
//    m_pInterface->RemoveSocket(this);
}

void Socket::TxBegin(Address* pAddress, uint16_t nPort)
{
    switch(pAddress->GetType())
    {
        case AF_INET:
            m_pInterface->TxBegin(pAddress);
            break;
        case AF_INET6:
            m_pInterface->TxBegin(pAddress);
            break;
        case AF_PACKET:
            m_pInterface->TxBegin(pAddress);
            break;
    }
}

void Socket::TxAppend(byte* pData, uint16_t nSize)
{
    m_pProtocol->TxAppend(pData, nSize);
}

void Socket::TxEnd()
{
    m_pProtocol->TxEnd();
}

void Socket::Send(byte* pData, uint16_t nSize, Address* pAddress, uint16_t nPort)
{
    TxBegin(pAddress, nPort);
    TxAppend(pData, nSize);
    TxEnd();
}
