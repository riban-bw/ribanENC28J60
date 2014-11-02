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

//DHCP
const static byte DHCP_DISABLED     = 0; //!< DHCP disabled
const static byte DHCP_RESET        = 1; //!< DHCP enabled but not yet requested
const static byte DHCP_DISCOVERY    = 2; //!< DHCP requested
const static byte DHCP_REQUESTED    = 3; //!< DHCP requested
const static byte DHCP_BOUND        = 4; //!< DHCP bound to valid address
const static byte DHCP_RENEWING     = 5; //!< DHCP bound, renewing lease
const static uint16_t DHCP_PACKET_SIZE  = 240; //!< Quantity of bytes in DHCP message
