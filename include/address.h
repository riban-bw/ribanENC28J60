/** Class provides address for Ethernet protocols
*/
#pragma once
#include "Arduino.h"

static const byte ADDR_TYPE_MAC = 0;
static const byte ADDR_TYPE_IPV4 = 1;
static const byte ADDR_TYPE_IPV6 = 2;

class Address
{
    public:
        /** @brief  Create an instance of an address
        *   @param  nType Address type: ADDR_TYPE_MAC | ADDR_TYPE_IPV4 | ADDR_TYPE_IPV6
        *   @param  pAddress Pointer to buffer holding new address. Default is empty (null) address
        */
        Address(byte nType, byte* pAddress = 0);

        virtual ~Address();

        /** @brief  Comparison with another Address operator */
        bool operator==(Address& address);

        /** @brief  Comparison with a byte array pointer operator */
        bool operator==(byte* pAddress);

        /** @brief  Comparison with another Address operator, not equal */
        bool operator!=(Address& address);

        /** @brief  Comparison with a byte array pointer operator, not equal */
        bool operator!=(byte* pAddress);

        // Overloaded copy operators to allow initialisation of IPAddress objects from other types
        /** @brief  Copy from another Address operator */
        Address& operator=(Address& address);

        /** @brief  Copy from a byte array pointer operator */
        Address& operator=(byte* pAddress);

        /** @brief  Gets pointer to byte array conatining address
        *   @return <i>byte*</i> Pointer to address byte array
        */
        byte* GetAddress();

        /** @brief  Populates byte array with address
        *   @param  pBuffer Pointer to byte array to populate
        */
        void GetAddress(byte* pBuffer);

        /** @brief  Sets address from byte array
        *   @param  pAddress Pointer to byte array containing address
        */
        void SetAddress(byte* pAddress);

        /** @brief  Gets the address type
        *   @return <i>byte</i> Address type: ADDR_TYPE_MAC | ADDR_TYPE_IPV4 | ADDR_TYPE_IPV6
        */
        byte GetType();

        /** @brief  Gets address size
        *   @return <i>byte</i> Length of address
        */
        byte GetSize();

        /** @brief  Prints address
        *   @note   Uses common output format for each address type
        *   @note   Assumes Serial is initialised
        */
        void PrintAddress();

    protected:
    private:
        byte m_nType; //!< Address type MAC | IPV4 | IPV6
        byte m_nSize; //!< Size of address
        byte* m_pAddress; //!< Pointer to address
};

