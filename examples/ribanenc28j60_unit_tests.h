/** ribanENC28J60 Unit Tests
*   Unit tests for the network interface
*/

/** @brief  Handle Tx errors
*/
void HandleTxError();

/** @brief  Handle Ping responses
*   @param  nSequence Ping sequence number
*/
void HandleEchoResponse(uint16_t nSequence);

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

/** @brief  Test transmission error reporting
*   @return True on success
*/
bool TestTxError();

/** @brief  Test setting IP address
*   @return True on success
*/
bool TestSetIp();

/** @brief  Test transmitting raw packet
*   @return True on success
*/
bool TestSendRaw();

//IPV4 tests
/** @brief  Test sending IPV4 packet
*   @return True on success
*/
bool TestSendIPV4();

/** @brief  Send UDP packet
*   @return True on success
*/

uint16_t g_nTime; //Used to measure asyncronous events
uint16_t g_nPingSequence; //Ping sequence number
