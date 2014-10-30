/** ribanENC28J60 Unit Test s
*   Unit tests for the network interface
*/

/** @brief  Show the test menu on serial port
*/
void ShowMenu();

/** @brief  Test all public functions for each type of address
*   @return True on success
*/
bool TestAddress();

/** @brief  Test that nic initialises without error and GetMac() works
*   @return True on success
*/
bool TestInitialisation();

/** @brief  Test sending IPV4 packet
*   @return True on success
*/
bool TestSendIPV4();

/** @brief  Test transmission error reporting
*   @return True on success
*/
bool TestTxError();

/** @brief  Test setting IP address
*   @return True on success
*/
bool TestSetIp();

ribanENC28J60* g_pNIC; //Pointer to the network interface
