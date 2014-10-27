/*  Test application
    Tests core functionality of ribanENC28J60
*/
#include "Arduino.h"
#include "include/ribanENC28J60.h"
#include "tests.h"

static const byte ETHERNET_CS_PIN = 10;

byte g_nTest; //Current test number


/** Initialisation */
void setup()
{
    g_pNIC = NULL;
    Serial.begin(9600);
    Serial.println(F("ribanENC28J60 Unit Tests"));
//    Serial.print(F("Test initialisation - "));
//    Serial.println(TestInitialisation()?"Pass":"Fail");
    ShowMenu();
}

/** Main program loop */
void loop()
{
    if(g_pNIC)
        g_pNIC->Process();
    if(Serial.available() > 0)
    {
        char cInput = Serial.read();
        switch(cInput)
        {
            case ' ':
                ShowMenu();
                break;
            case '1':
                Serial.println(TestAddress()?"Pass":"Fail");
                break;
            case 'i':
                Serial.print("Test NIC initialisation - ");
                Serial.println(TestInitialisation()?"Pass":"Fail");
                break;
            case 'r':
                setup();
                break;
            case 'u':
//                Socket socket(&nic, SOCK_UDP);
//
//                byte pData[5] = {'h','e','l','l','o'};
//                socket.TxBegin();
//                socket.TxAppend(pData, sizeof(pData));
//                socket.TxEnd();
//                //socket.Send(pData, sizeof(pData));
                break;
        }
    }
}

void ShowMenu()
{
    Serial.println("Menu");
    Serial.println("1 - Test Address");
    Serial.println("i - Initialise");
    Serial.println("r - Reset");
}

bool TestAddress()
{
    bool bResult = true;
    bool bSuccess;
    byte pAddress[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
    Address addressMac(ADDR_TYPE_MAC, pAddress);
    Serial.print(F("Print MAC: Expected result is '12:34:56:78:9A:BC'. Actual result is '"));
    addressMac.PrintAddress();
    Serial.println("'");
    byte pIp[4] = {192,168,0,234};
    Address addressIPV4(ADDR_TYPE_IPV4, pIp);
    Serial.print(F("Print IP: Expected result is '192.168.0.234'. Actual result is '"));
    addressIPV4.PrintAddress();
    Serial.println("'");
    Address addressIPV6(ADDR_TYPE_IPV6, pAddress);
    Serial.print(F("Print IPV6: Expected result is '1234:5678:9ABC:DEF0:0011:2233:4455:6677'. Actual result is '"));
    addressIPV6.PrintAddress();
    Serial.println("'");
    Serial.print(F("Compare MAC Address Objects - "));
    Address addressMac_2(ADDR_TYPE_MAC, pAddress);
    bSuccess = (addressMac == addressMac_2);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV4 Address Objects - "));
    Address addressIPV4_2(ADDR_TYPE_IPV4, pIp);
    bSuccess = (addressIPV4 == addressIPV4_2);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV6 Address Objects - "));
    Address addressIPV6_2(ADDR_TYPE_IPV6, pAddress);
    bSuccess = (addressIPV6 == addressIPV6_2);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare MAC Address Objects != - "));
    Address addressMac_3(ADDR_TYPE_MAC, pAddress);
    bSuccess = !(addressMac != addressMac_3);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV4 Address Objects != - "));
    Address addressIPV4_3(ADDR_TYPE_IPV4, pIp);
    bSuccess = !(addressIPV4 != addressIPV4_3);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV6 Address Objects != - "));
    Address addressIPV6_3(ADDR_TYPE_IPV6, pAddress);
    bSuccess = !(addressIPV6 != addressIPV6_3);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare MAC Address Object with array - "));
    bSuccess = (addressMac == pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV4 Address Object with array - "));
    bSuccess = (addressIPV4 == pIp);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV6 Address Object with array - "));
    bSuccess = (addressIPV6 == pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare MAC Address Object with array != - "));
    bSuccess = !(addressMac != pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV4 Address Object with array != - "));
    bSuccess = !(addressIPV4 != pIp);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Compare IPV6 Address Object with array != - "));
    bSuccess = !(addressIPV6 != pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Address = Address MAC copy operator - "));
    Address addressMac_4(ADDR_TYPE_MAC);
    addressMac_4 = addressMac;
    bSuccess = (addressMac == addressMac_4);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Address = Address IPV4 copy operator - "));
    Address addressIPV4_4(ADDR_TYPE_IPV4);
    addressIPV4_4 = addressIPV4;
    bSuccess = (addressIPV4 == addressIPV4_4);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Address = Address IPV6 copy operator - "));
    Address addressIPV6_4(ADDR_TYPE_IPV6);
    addressIPV6_4 = addressIPV6;
    bSuccess = (addressIPV6 == addressIPV6_4);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Address = byte* MAC copy operator - "));
    Address addressMac_5(ADDR_TYPE_MAC);
    addressMac_5 = pAddress;
    bSuccess = (addressMac_5 == pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Address = byte* IPV4 copy operator - "));
    Address addressIPV4_5(ADDR_TYPE_IPV4);
    addressIPV4_5 = pAddress;
    bSuccess = (addressIPV4_5 == pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Address = byte* IPV6 copy operator - "));
    Address addressIPV6_5(ADDR_TYPE_IPV6);
    addressIPV6_5 = pAddress;
    bSuccess = (addressIPV6_5 == pAddress);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get MAC size - "));
    bSuccess = (6 == addressMac.GetSize());
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get IPV4 size - "));
    bSuccess = (4 == addressIPV4.GetSize());
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get IPV6 size - "));
    bSuccess = (16 == addressIPV6.GetSize());
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get MAC Address array - "));
    byte* pPtr = addressMac.GetAddress();
    bSuccess = (memcmp(pAddress, pPtr, addressMac.GetSize()) == 0);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get IPV4 Address array - "));
    pPtr = addressIPV4.GetAddress();
    bSuccess = (memcmp(pIp, pPtr, addressIPV4.GetSize()) == 0);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get IPV6 Address array - "));
    pPtr = addressIPV6.GetAddress();
    bSuccess = (memcmp(pAddress, pPtr, addressIPV6.GetSize()) == 0);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Populate array from MAC Address- "));
    byte pMAC[6];
    addressMac.GetAddress(pMAC);
    bSuccess = (memcmp(addressMac.GetAddress(), pMAC, 6) == 0);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Populate array from IPV4 Address- "));
    byte pIPV4[4];
    addressIPV4.GetAddress(pIPV4);
    bSuccess = (memcmp(addressIPV4.GetAddress(), pIPV4, 4) == 0);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Populate array from IPV6 Address - "));
    byte pIPV6[16];
    addressIPV6.GetAddress(pIPV6);
    bSuccess = (memcmp(addressIPV6.GetAddress(), pIPV6, 16) == 0);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Set MAC Address - "));
    byte pMAC_2[6] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA};
    addressMac_4.SetAddress(pMAC_2);
    bSuccess = (addressMac_4 == pMAC_2);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Set IPV4 Address - "));
    byte pIPV4_2[4] = {10,0,100,1};
    addressIPV4_4.SetAddress(pIPV4_2);
    bSuccess = (addressIPV4_4 == pIPV4_2);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Set IPV6 Address - "));
    byte pIPV6_2[16] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x55,0x33,0x22,0x11,0x00};
    addressIPV6_4.SetAddress(pIPV6_2);
    bSuccess = (addressIPV6_4 == pIPV6_2);
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get MAC type - "));
    bSuccess = (ADDR_TYPE_MAC == addressMac.GetType());
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get IPV4 type - "));
    bSuccess = (ADDR_TYPE_IPV4 == addressIPV4.GetType());
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    Serial.print(F("Get IPV6 type - "));
    bSuccess = (ADDR_TYPE_IPV6 == addressIPV6.GetType());
    Serial.println(bSuccess?"Pass":"Fail");
    bResult &= bSuccess;
    return bResult;
}

bool TestInitialisation()
{
    if(g_pNIC)
        delete g_pNIC;
    byte pMac[6] = {0x12,0x34,0x56,0x78,0x9A,0xBC};
    Address addressMAC(ADDR_TYPE_MAC, pMac);
    g_pNIC = new ribanENC28J60(addressMAC, ETHERNET_CS_PIN);
    return(addressMAC == g_pNIC->GetMac()->GetAddress());
}
