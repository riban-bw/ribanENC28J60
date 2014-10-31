/** ribanENC28J60 Unit Tests
*   Unit tests for the network interface
*/

/** @brief  Show the test menu on serial port
*/
void ShowMenu();

/** @brief  Test all public functions for each type of address
*   @return True on success
*/
bool TestAddress();

/** @brief  Test that nic has initialised without error and GetMac() works
*   @return True on success
*/
bool TestInitialised();

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

