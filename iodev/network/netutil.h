/////////////////////////////////////////////////////////////////////////
// $Id: netutil.h 13836 2020-05-02 21:21:53Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004-2020  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//

//  netutil.h  - shared code for eth_vnet.cc and bxhub.cc

#ifndef BX_NETUTIL_H
#define BX_NETUTIL_H

#define ETHERNET_MAC_ADDR_LEN 6

#if defined(_MSC_VER)
#pragma pack(push, 1)
#elif defined(__MWERKS__) && defined(macintosh)
#pragma options align=packed
#endif

typedef struct ethernet_header {
#if defined(_MSC_VER) && (_MSC_VER>=1300)
  __declspec(align(1))
#endif
  Bit8u  dst_mac_addr[ETHERNET_MAC_ADDR_LEN];
  Bit8u  src_mac_addr[ETHERNET_MAC_ADDR_LEN];
  Bit16u type;
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
ethernet_header_t;

typedef struct arp_header {
#if defined(_MSC_VER) && (_MSC_VER>=1300)
  __declspec(align(1))
#endif
  Bit16u  hw_addr_space;
  Bit16u  proto_addr_space;
  Bit8u   hw_addr_len;
  Bit8u   proto_addr_len;
  Bit16u  opcode;
  /* HW address of sender */
  /* Protocol address of sender */
  /* HW address of target*/
  /* Protocol address of target */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
arp_header_t;

typedef struct ip_header {
#if defined(_MSC_VER) && (_MSC_VER>=1300)
  __declspec(align(1))
#endif
#ifdef BX_LITTLE_ENDIAN
  Bit8u header_len : 4;
  Bit8u version : 4;
#else
  Bit8u version : 4;
  Bit8u header_len : 4;
#endif
  Bit8u tos;
  Bit16u total_len;
  Bit16u id;
  Bit16u frag_offs;
  Bit8u ttl;
  Bit8u protocol;
  Bit16u checksum;
  Bit32u src_addr;
  Bit32u dst_addr;
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
ip_header_t;

typedef struct udp_header {
#if defined(_MSC_VER) && (_MSC_VER>=1300)
  __declspec(align(1))
#endif
  Bit16u src_port;
  Bit16u dst_port;
  Bit16u length;
  Bit16u checksum;
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
udp_header_t;

typedef struct tcp_header {
#if defined(_MSC_VER) && (_MSC_VER>=1300)
  __declspec(align(1))
#endif
  Bit16u src_port;
  Bit16u dst_port;
  Bit32u seq_num;
  Bit32u ack_num;
#ifdef BX_LITTLE_ENDIAN
  Bit8u reserved : 4;
  Bit8u data_offset : 4;
#else
  Bit8u data_offset : 4;
  Bit8u reserved : 4;
#endif
  struct {
#ifdef BX_LITTLE_ENDIAN
    Bit8u fin : 1;
    Bit8u syn : 1;
    Bit8u rst : 1;
    Bit8u psh : 1;
    Bit8u ack : 1;
    Bit8u urg : 1;
    Bit8u ece : 1;
    Bit8u cwr : 1;
#else
    Bit8u cwr : 1;
    Bit8u ece : 1;
    Bit8u urg : 1;
    Bit8u ack : 1;
    Bit8u psh : 1;
    Bit8u rst : 1;
    Bit8u syn : 1;
    Bit8u fin : 1;
#endif
  } flags;
  Bit16u window;
  Bit16u checksum;
  Bit16u urgent_ptr;
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
tcp_header_t;

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__MWERKS__) && defined(macintosh)
#pragma options align=reset
#endif

// DHCP configuration structure
typedef struct {
  Bit8u host_macaddr[6];
  Bit8u host_ipv4addr[4];
  Bit8u dns_ipv4addr[4];
} dhcp_cfg_t;

// vnet functions shared with bxhub
#ifdef BXHUB
#define bx_devmodel_c void
#endif
Bit16u ip_checksum(const Bit8u *buf, unsigned buf_len);

typedef struct tftp_session {
  char     filename[BX_PATHNAME_LEN];
  Bit16u   tid;
  bx_bool  write;
  unsigned options;
  size_t   tsize_val;
  unsigned blksize_val;
  unsigned timeout_val;
  unsigned timestamp;
  struct tftp_session *next;
} tftp_session_t;

// VNET server

#define VNET_MAX_CLIENTS 6
#define LAYER4_LISTEN_MAX  128

typedef int (*layer4_handler_t)(
  void *this_ptr,
  const Bit8u *ipheader,
  unsigned ipheader_len,
  unsigned sourceport,
  unsigned targetport,
  const Bit8u *data,
  unsigned data_len,
  Bit8u *reply
  );

class vnet_server_c {
public:
  vnet_server_c();
  virtual ~vnet_server_c();

  void init(bx_devmodel_c *netdev, dhcp_cfg_t *dhcpc, const char *tftp_rootdir);
  void init_client(Bit8u clientid, const Bit8u *macaddr, const Bit8u *default_ipv4addr);
  int handle_packet(const Bit8u *buf, unsigned len, Bit8u *reply);
#ifdef BXHUB
  void init_log(const char *logfn);
#endif

  layer4_handler_t get_layer4_handler(unsigned ipprotocol, unsigned port);
  bx_bool register_layer4_handler(unsigned ipprotocol, unsigned port,
                                  layer4_handler_t func);
  bx_bool unregister_layer4_handler(unsigned ipprotocol, unsigned port);

private:
#ifdef BXHUB
  void bx_printf(const char *fmt, ...);
#endif
  bx_bool find_client(const Bit8u *mac_addr, Bit8u *clientid);

  int process_arp(Bit8u clientid, const Bit8u *buf, unsigned len, Bit8u *reply);
  int process_ipv4(Bit8u clientid, const Bit8u *buf, unsigned len, Bit8u *reply);

  int process_icmpipv4(Bit8u clientid, const Bit8u *ipheader, unsigned ipheader_len,
                       const Bit8u *l4pkt, unsigned l4pkt_len, Bit8u *reply);
  int process_tcpipv4(Bit8u clientid, const Bit8u *ipheader, unsigned ipheader_len,
                      const Bit8u *l4pkt, unsigned l4pkt_len, Bit8u *reply);
  int process_udpipv4(Bit8u clientid, const Bit8u *ipheader, unsigned ipheader_len,
                      const Bit8u *l4pkt, unsigned l4pkt_len, Bit8u *reply);

  static int udpipv4_dhcp_handler(void *this_ptr, const Bit8u *ipheader,
                                  unsigned ipheader_len, unsigned sourceport,
                                  unsigned targetport, const Bit8u *data,
                                  unsigned data_len, Bit8u *reply);
  int udpipv4_dhcp_handler_ns(const Bit8u *ipheader, unsigned ipheader_len,
                              unsigned sourceport, unsigned targetport,
                              const Bit8u *data, unsigned data_len, Bit8u *reply);
  static int udpipv4_tftp_handler(void *this_ptr, const Bit8u *ipheader,
                                  unsigned ipheader_len, unsigned sourceport,
                                  unsigned targetport, const Bit8u *data,
                                  unsigned data_len, Bit8u *reply);
  int udpipv4_tftp_handler_ns(const Bit8u *ipheader, unsigned ipheader_len,
                              unsigned sourceport, unsigned targetport,
                              const Bit8u *data, unsigned data_len, Bit8u *reply);
  static int udpipv4_dns_handler(void *this_ptr, const Bit8u *ipheader,
                                 unsigned ipheader_len, unsigned sourceport,
                                 unsigned targetport, const Bit8u *data,
                                 unsigned data_len, Bit8u *reply);
  int udpipv4_dns_handler_ns(const Bit8u *ipheader, unsigned ipheader_len,
                             unsigned sourceport, unsigned targetport,
                             const Bit8u *data, unsigned data_len, Bit8u *reply);

  void tftp_parse_options(const char *mode, const Bit8u *data, unsigned data_len,
                          tftp_session_t *s);

#ifdef BXHUB
  FILE *logfd;
#endif
  bx_devmodel_c *netdev;
  dhcp_cfg_t *dhcp;
  const char *tftp_root;

  struct {
    bx_bool init;
    const Bit8u *macaddr;
    const Bit8u *default_ipv4addr;
    Bit8u ipv4addr[4];
    char *hostname;
  } client[VNET_MAX_CLIENTS];

  struct {
    unsigned ipprotocol;
    unsigned port;
    layer4_handler_t func;
  } l4data[LAYER4_LISTEN_MAX];

  unsigned l4data_used;
};

#endif
