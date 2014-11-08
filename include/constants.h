/*  Constant values used by various protocols */

#pragma once

//EtherTypes
const static unsigned int ETHTYPE_IPV4          = 0x0800;
const static unsigned int ETHTYPE_ARP           = 0x0806;
const static unsigned int ETHTYPE_IEEE801_10    = 0x8100;
const static unsigned int ETHTYPE_IPV6          = 0x86DD;

//MAC
const static uint16_t MAC_HEADER_SIZE           = 14;
const static uint16_t MAC_OFFSET_DESTINATION    = 0;
const static uint16_t MAC_OFFSET_SOURCE         = 6;
const static uint16_t MAC_OFFSET_TYPE           = 12;

//IPV4
const static uint16_t IPV4_HEADER_SIZE          = 20;
const static uint16_t IPV4_MINLEN               = 26;
const static uint16_t IPV4_OFFSET_VERSION       = 0;
const static uint16_t IPV4_OFFSET_DSCP          = 1;
const static uint16_t IPV4_OFFSET_LENGTH        = 2;
const static uint16_t IPV4_OFFSET_ID            = 4;
const static uint16_t IPV4_OFFSET_FLAGS         = 6;
const static uint16_t IPV4_OFFSET_TTL           = 8;
const static uint16_t IPV4_OFFSET_PROTOCOL      = 9;
const static uint16_t IPV4_OFFSET_CHECKSUM      = 10;
const static uint16_t IPV4_OFFSET_SOURCE        = 12;
const static uint16_t IPV4_OFFSET_DESTINATION   = 16;

//IP Protocol types
const static uint16_t IP_PROTOCOL_ICMP  = 1;
const static uint16_t IP_PROTOCOL_IGMP  = 2;
const static uint16_t IP_PROTOCOL_TCP   = 6;
const static uint16_t IP_PROTOCOL_UDP   = 17;
const static uint16_t IP_PROTOCOL_ENCAP = 41;
const static uint16_t IP_PROTOCOL_OSPF  = 89;
const static uint16_t IP_PROTOCOL_SCTP  = 132;

//ARP
const static uint16_t ARP_IPV4_LEN  = 28;
const static uint16_t ARP_REQUEST   = 1;
const static uint16_t ARP_REPLY     = 2;
const static uint16_t ARP_HTYPE     = 0;
const static uint16_t ARP_PTYPE     = 2;
const static uint16_t ARP_HLEN      = 4;
const static uint16_t ARP_PLEN      = 5;
const static uint16_t ARP_OPER      = 6;
const static uint16_t ARP_SHA       = 8;
const static uint16_t ARP_SPA       = 14;
const static uint16_t ARP_THA       = 18;
const static uint16_t ARP_TPA       = 24;

//ICMP
const static uint16_t ICMP_HEADER_SIZE      = 8;
const static byte ICMP_TYPE_ECHOREPLY       = 0;
const static byte ICMP_TYPE_ECHOREQUEST     = 8;
const static uint16_t ICMP_OFFSET_TYPE      = 0;
const static uint16_t ICMP_OFFSET_CODE      = 1;
const static uint16_t ICMP_OFFSET_CHECKSUM  = 2;

//UDP
const static uint16_t UDP_HEADER_SIZE               = 8;
const static uint16_t UDP_OFFSET_SOURCE_PORT        = 0;
const static uint16_t UDP_OFFSET_DESTINATION_PORT   = 2;
const static uint16_t UDP_OFFSET_LENGTH             = 4;
const static uint16_t UDP_OFFSET_CHECKSUM           = 6;

//DHCP
const static byte DHCP_DISABLED             = 0; //!< DHCP disabled - using static IP configuration
const static byte DHCP_RESET                = 1; //!< DHCP enabled but not yet requested
const static byte DHCP_DISCOVERY            = 2; //!< DHCP discovery - asked for a new IP
const static byte DHCP_REQUESTED            = 3; //!< DHCP requested - requested specifi IP based on offer
const static byte DHCP_BOUND                = 4; //!< DHCP bound to valid address - DHCP complete and ready to roll
const static byte DHCP_RENEWING             = 5; //!< DHCP bound, renewing lease - requested lease renewal
const static uint16_t DHCP_PACKET_SIZE      = 249; //!< Quantity of bytes in DHCP messages sent from this host (including consistent set of options)
const static uint16_t DHCP_SERVER_PORT      = 67; //!< UDP port used by DHCP server
const static uint16_t DHCP_CLIENT_PORT      = 68; //!< UDP port used by DHCP client
const static uint16_t DHCP_OFFSET_OP        = 0;
const static uint16_t DHCP_OFFSET_HTYPE     = 1;
const static uint16_t DHCP_OFFSET_HLEN      = 2;
const static uint16_t DHCP_OFFSET_HOPS      = 3;
const static uint16_t DHCP_OFFSET_XID       = 4;
const static uint16_t DHCP_OFFSET_SECS      = 8;
const static uint16_t DHCP_OFFSET_FLAGS     = 10;
const static uint16_t DHCP_OFFSET_CIADDR    = 12; //!<DHCP client IP address
const static uint16_t DHCP_OFFSET_YIADDR    = 14; //!<DHCP your IP address
const static uint16_t DHCP_OFFSET_SIADDR    = 16; //!<DHCP server IP address
const static uint16_t DHCP_OFFSET_GIADDR    = 18; //!<DHCP gateway IP address
const static uint16_t DHCP_OFFSET_CHADDR    = 20; //!<DHCP client hardware address
const static uint16_t DHCP_OFFSET_OPTIONS   = 240; //!< Start of DHCP options
const static uint16_t DHCP_OPTION_PAD       = 0; //!< DHCP Option 0: Pads DHCP options, e.g. to meet word boundaries
const static uint16_t DHCP_OPTION_MASK      = 1; //!< DHCP Option 1: Subnetmask
const static uint16_t DHCP_OPTION_ROUTER    = 3; //!< DHCP Option 3: Router
const static uint16_t DHCP_OPTION_DNS       = 6; //!< DHCP Option 6: DNS servers
const static uint16_t DHCP_OPTION_REQ_IP    = 50; //!< DHCP Option 50: Requested IP address
const static uint16_t DHCP_OPTION_LEASE     = 51; //!< DHCP Option 51: Lease time
const static uint16_t DHCP_OPTION_TYPE      = 53; //!< DHCP Option 53: Message type
const static uint16_t DHCP_OPTION_SERVER    = 54; //!< DHCP Option 54: DHCP server
const static uint16_t DHCP_OPTION_PARAM     = 55; //!< DHCP Option 55: Parameter list
//!@todo Check DHCP Type constants (Wikipedia does not define these)
const static uint16_t DHCP_TYPE_DISCOVER    = 1; //!< DHCP Type 1: Discover
const static uint16_t DHCP_TYPE_OFFER       = 2; //!< DHCP Type 2: Offer
const static uint16_t DHCP_TYPE_REQUEST     = 3; //!< DHCP Type 3: Request
const static uint16_t DHCP_TYPE_ACK         = 4; //!< DHCP Type 4: Acknowledge

