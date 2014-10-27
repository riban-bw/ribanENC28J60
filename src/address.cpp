#include "address.h"
#include "new.h"

Address::Address(byte nType, byte* pAddress)
{
    m_nType = nType;
    switch(nType)
    {
        case ADDR_TYPE_MAC:
            m_nSize = 6;
            break;
        case ADDR_TYPE_IPV4:
            m_nSize = 4;
            break;
        case ADDR_TYPE_IPV6:
            m_nSize = 16;
            break;
        default:
            m_nSize = 0;
    }
    m_pAddress = new byte[m_nSize];
    if(pAddress)
        memcpy(m_pAddress, pAddress, m_nSize);
    else
        memset(m_pAddress, 0, m_nSize);
}

Address::~Address()
{
    delete m_pAddress;
}

bool Address::operator==(Address& address)
{
    return (address == m_pAddress);
}

bool Address::operator==(byte* pAddress)
{
    return (0 == memcmp(pAddress, m_pAddress, m_nSize));
}

bool Address::operator!=(Address& address)
{
    return (address != m_pAddress);
}

bool Address::operator!=(byte* pAddress)
{
    return (0 != memcmp(pAddress, m_pAddress, m_nSize));
}

Address& Address::operator=(Address& address)
{
    //!@todo Check for same type
    m_nSize = address.m_nSize; //!@todo This corrupts memory for different types
    m_nType = address.m_nType;
    memcpy(m_pAddress, address.m_pAddress, m_nSize);
    return *this;
}

Address& Address::operator=(byte* pAddress)
{
    memcpy(m_pAddress, pAddress, m_nSize);
    return *this;
}

byte* Address::GetAddress()
{
    return m_pAddress;
}

void Address::GetAddress(byte* pBuffer)
{
    memcpy(pBuffer, m_pAddress, m_nSize);
}

void Address::SetAddress(byte* pAddress)
{
    memcpy(m_pAddress, pAddress, m_nSize);
}
byte Address::GetType()
{
    return m_nType;
}

byte Address::GetSize()
{
    return m_nSize;
}

void Address::PrintAddress()
{
    for(byte i = 0; i < m_nSize; ++i)
    {
        switch(m_nType)
        {
            case ADDR_TYPE_IPV4:
                Serial.print(m_pAddress[i], DEC);
                if(i < m_nSize - 1)
                    Serial.print(".");
                break;
            case ADDR_TYPE_IPV6:
            {
                uint16_t nValue = (m_pAddress[i] << 8) + m_pAddress[i+1];
                if(nValue < 0x1000)
                    Serial.print("0");
                if(nValue < 0x100)
                    Serial.print("0");
                if(nValue < 0x10)
                    Serial.print("0");
                Serial.print(nValue, HEX);
                if(i < m_nSize - 1)
                    Serial.print(":");
                ++i;
                break;
            }
            case ADDR_TYPE_MAC:
                if(m_pAddress[i] < 0x10)
                    Serial.print("0");
                Serial.print(m_pAddress[i], HEX);
                if(i < m_nSize - 1)
                    Serial.print(":");
                break;
        }
    }
}
