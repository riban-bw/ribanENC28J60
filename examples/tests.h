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

/** @brief  Test that nic initialises without error
*   @return True on success
*/
bool TestInitialisation();

ribanENC28J60* g_pNIC; //Pointer to the network interface
