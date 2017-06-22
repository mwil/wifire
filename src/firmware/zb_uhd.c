//
// Copyright 2010 Ettus Research LLC
//
/*
 * Copyright 2007,2008 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//#define DBG
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "zb_wifire.h"

#include <lwip/ip.h>
#include <lwip/udp.h>
#include "u2_init.h"
#include "memory_map.h"
#include "spi.h"
#include "hal_io.h"
#include "buffer_pool.h"
#include "pic.h"
#include <stdbool.h>
#include "ethernet.h"
#include "nonstdio.h"
#include <net/padded_eth_hdr.h>
#include <net_common.h>
#include "memcpy_wa.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "clocks.h"
#include "usrp2/fw_common.h"
#include <i2c_async.h>
#include <i2c.h>
#include <ethertype.h>
#include <arp_cache.h>

#define ETH_RX_BUF 		1

// The mac address of the host we're sending to.
eth_mac_addr_t host_mac_addr;

static void print_ip_addr(const void *t){
    uint8_t *p = (uint8_t *)t;
    printf("%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

//setup the output data
static usrp2_ctrl_data_t ctrl_data_out;
static struct socket_address i2c_src;
static struct socket_address spi_src;

static volatile bool i2c_done = false;
void i2c_read_done_callback(void) {
  //printf("I2C read done callback\n");
  i2c_async_data_ready(ctrl_data_out.data.i2c_args.data);
  i2c_done = true;
  i2c_register_callback(0);
}

void i2c_write_done_callback(void) {
  //printf("I2C write done callback\n");
  i2c_done = true;
  i2c_register_callback(0);
}

static volatile bool spi_done = false;
//static volatile uint32_t spi_readback_data;

void get_spi_readback_data(void) {
  ctrl_data_out.data.spi_args.data = spi_get_data();
  spi_done = true;
  spi_register_callback(0);
}

void handle_udp_ctrl_packet(
    struct socket_address src, struct socket_address dst,
    unsigned char *payload, int payload_len
){
    //printf("Got ctrl packet #words: %d\n", (int)payload_len);
    const usrp2_ctrl_data_t *ctrl_data_in = (usrp2_ctrl_data_t *)payload;
    uint32_t ctrl_data_in_id = ctrl_data_in->id;

    //ensure that the protocol versions match
    if (payload_len >= sizeof(uint32_t) && ctrl_data_in->proto_ver != USRP2_FW_COMPAT_NUM){
        printf("!Error in control packet handler: Expected compatibility number %d, but got %d\n",
            USRP2_FW_COMPAT_NUM, ctrl_data_in->proto_ver
        );
        ctrl_data_in_id = USRP2_CTRL_ID_WAZZUP_BRO;
    }

    //ensure that this is not a short packet
    if (payload_len < sizeof(usrp2_ctrl_data_t)){
        printf("!Error in control packet handler: Expected payload length %d, but got %d\n",
            (int)sizeof(usrp2_ctrl_data_t), payload_len
        );
        ctrl_data_in_id = USRP2_CTRL_ID_HUH_WHAT;
    }

    //setup the output data
    ctrl_data_out.proto_ver = USRP2_FW_COMPAT_NUM;
    ctrl_data_out.id=USRP2_CTRL_ID_HUH_WHAT;
    ctrl_data_out.seq=ctrl_data_in->seq;

    //handle the data based on the id
    switch(ctrl_data_in_id){

    /*******************************************************************
     * Addressing
     ******************************************************************/
    case USRP2_CTRL_ID_WAZZUP_BRO:
        ctrl_data_out.id = USRP2_CTRL_ID_WAZZUP_DUDE;
        memcpy(&ctrl_data_out.data.ip_addr, get_ip_addr(), sizeof(struct ip_addr));
        send_udp_pkt(USRP2_UDP_CTRL_PORT, src, &ctrl_data_out, sizeof(ctrl_data_out));
        break;

    /*******************************************************************
     * SPI
     ******************************************************************/
    case USRP2_CTRL_ID_TRANSACT_ME_SOME_SPI_BRO:{
#ifdef DBG
            printf("SPI Tansaction: dev=0x%x, data=0x%x\n",
                ctrl_data_in->data.spi_args.dev, ctrl_data_in->data.spi_args.data);
#endif
            //transact
            /*bool success =*/ spi_async_transact(
                //(ctrl_data_in->data.spi_args.readback == 0)? SPI_TXONLY : SPI_TXRX,
                ctrl_data_in->data.spi_args.dev,      //which device
                ctrl_data_in->data.spi_args.data,     //32 bit data
                ctrl_data_in->data.spi_args.num_bits, //length in bits
                (ctrl_data_in->data.spi_args.mosi_edge == USRP2_CLK_EDGE_RISE)? SPIF_PUSH_FALL : SPIF_PUSH_RISE | //flags
                (ctrl_data_in->data.spi_args.miso_edge == USRP2_CLK_EDGE_RISE)? SPIF_LATCH_RISE : SPIF_LATCH_FALL,
                get_spi_readback_data //callback
            );

            //load output
            ctrl_data_out.id = USRP2_CTRL_ID_OMG_TRANSACTED_SPI_DUDE;
            spi_src = src;
        }
//        send_udp_pkt(USRP2_UDP_CTRL_PORT, src, &ctrl_data_out, sizeof(ctrl_data_out));
        break;

    /*******************************************************************
     * I2C
     ******************************************************************/
    case USRP2_CTRL_ID_DO_AN_I2C_READ_FOR_ME_BRO:{
#if 0
            printf("I2C Read: addr=0x%x len=%d\n",
                ctrl_data_in->data.i2c_args.addr, ctrl_data_in->data.i2c_args.bytes);
#endif
            uint8_t num_bytes = ctrl_data_in->data.i2c_args.bytes;
            i2c_register_callback(i2c_read_done_callback);
            i2c_async_read(
                ctrl_data_in->data.i2c_args.addr,
                num_bytes
            );
            i2c_src = src;
//            i2c_dst = dst;
            ctrl_data_out.id = USRP2_CTRL_ID_HERES_THE_I2C_DATA_DUDE;
            ctrl_data_out.data.i2c_args.bytes = num_bytes;
        }
        break;

    case USRP2_CTRL_ID_WRITE_THESE_I2C_VALUES_BRO:{
#ifdef DBG
            printf("I2C Write: addr=0x%x len=%d\n",
                ctrl_data_in->data.i2c_args.addr,ctrl_data_in->data.i2c_args.bytes);
#endif

            uint8_t num_bytes = ctrl_data_in->data.i2c_args.bytes;
            i2c_register_callback(i2c_read_done_callback);
            i2c_async_write(
                ctrl_data_in->data.i2c_args.addr,
                ctrl_data_in->data.i2c_args.data,
                num_bytes
            );
            i2c_src = src;
//            i2c_dst = dst;
            ctrl_data_out.id = USRP2_CTRL_ID_COOL_IM_DONE_I2C_WRITE_DUDE;
            ctrl_data_out.data.i2c_args.bytes = num_bytes;
        }
        break;

    /*******************************************************************
     * Peek and Poke Register
     ******************************************************************/
    case USRP2_CTRL_ID_POKE_THIS_REGISTER_FOR_ME_BRO:
#ifdef DBG
        printf("Poked register 0x%x with 0x%x.\n", ctrl_data_in->data.poke_args.addr,
            ctrl_data_in->data.poke_args.num_bytes == 4 ? (uint32_t)ctrl_data_in->data.poke_args.data
                : (ctrl_data_in->data.poke_args.num_bytes == 2 ? (uint16_t)ctrl_data_in->data.poke_args.data
                : (uint8_t)ctrl_data_in->data.poke_args.data));
#endif
        if (0){//ctrl_data_in->data.poke_args.addr < 0xC000){
            printf("error! tried to poke into 0x%x\n", ctrl_data_in->data.poke_args.addr);
        }
        else switch(ctrl_data_in->data.poke_args.num_bytes){
        case sizeof(uint64_t):
            *((uint32_t *) ctrl_data_in->data.poke_args.addrhi) = (uint32_t)ctrl_data_in->data.poke_args.datahi;
            //continue to uint32_t for low addr:

        case sizeof(uint32_t):
            *((uint32_t *) ctrl_data_in->data.poke_args.addr) = (uint32_t)ctrl_data_in->data.poke_args.data;
            break;

        case sizeof(uint16_t):
            *((uint16_t *) ctrl_data_in->data.poke_args.addr) = (uint16_t)ctrl_data_in->data.poke_args.data;
            break;

        case sizeof(uint8_t):
            *((uint8_t *) ctrl_data_in->data.poke_args.addr) = (uint8_t)ctrl_data_in->data.poke_args.data;
            break;

        }
        ctrl_data_out.id = USRP2_CTRL_ID_OMG_POKED_REGISTER_SO_BAD_DUDE;
        send_udp_pkt(USRP2_UDP_CTRL_PORT, src, &ctrl_data_out, sizeof(ctrl_data_out));
        break;

    case USRP2_CTRL_ID_PEEK_AT_THIS_REGISTER_FOR_ME_BRO:
#ifdef DBG
        printf("Peek register 0x%x with 0x%x.\n", ctrl_data_in->data.poke_args.addr,
            ctrl_data_in->data.poke_args.num_bytes == 4 ? (uint32_t)ctrl_data_in->data.poke_args.data
                : (ctrl_data_in->data.poke_args.num_bytes == 2 ? (uint16_t)ctrl_data_in->data.poke_args.data
                : (uint8_t)ctrl_data_in->data.poke_args.data));
#endif
        switch(ctrl_data_in->data.poke_args.num_bytes){
        case sizeof(uint64_t):
            ctrl_data_out.data.poke_args.datahi = *((uint32_t *) ctrl_data_in->data.poke_args.addrhi);
            //continue to uint32_t for low addr:

        case sizeof(uint32_t):
            ctrl_data_out.data.poke_args.data = *((uint32_t *) ctrl_data_in->data.poke_args.addr);
            break;

        case sizeof(uint16_t):
            ctrl_data_out.data.poke_args.data = *((uint16_t *) ctrl_data_in->data.poke_args.addr);
            break;

        case sizeof(uint8_t):
            ctrl_data_out.data.poke_args.data = *((uint8_t *) ctrl_data_in->data.poke_args.addr);
            break;

        }
        ctrl_data_out.id = USRP2_CTRL_ID_WOAH_I_DEFINITELY_PEEKED_IT_DUDE;
        send_udp_pkt(USRP2_UDP_CTRL_PORT, src, &ctrl_data_out, sizeof(ctrl_data_out));
        break;

    case USRP2_CTRL_ID_SO_LIKE_CAN_YOU_READ_THIS_UART_BRO:{
      //executes a readline()-style read, up to num_bytes long, up to and including newline
      int num_bytes = ctrl_data_in->data.uart_args.bytes;
      if(num_bytes > 20) num_bytes = 20;
      num_bytes = fngets_timeout(ctrl_data_in->data.uart_args.dev, (char *) ctrl_data_out.data.uart_args.data, num_bytes);
      ctrl_data_out.id = USRP2_CTRL_ID_I_HELLA_READ_THAT_UART_DUDE;
      ctrl_data_out.data.uart_args.bytes = num_bytes;
      break;
    }

    case USRP2_CTRL_ID_HEY_WRITE_THIS_UART_FOR_ME_BRO:{
      int num_bytes = ctrl_data_in->data.uart_args.bytes;
      if(num_bytes > 20) num_bytes = 20;
      //before we write to the UART, we flush the receive buffer
      //this assumes that we're interested in the reply
      hal_uart_rx_flush(ctrl_data_in->data.uart_args.dev);
      fnputstr(ctrl_data_in->data.uart_args.dev, (char *) ctrl_data_in->data.uart_args.data, num_bytes);
      ctrl_data_out.id = USRP2_CTRL_ID_MAN_I_TOTALLY_WROTE_THAT_UART_DUDE;
      ctrl_data_out.data.uart_args.bytes = num_bytes;
      break;
    }

    default:
        ctrl_data_out.id = USRP2_CTRL_ID_HUH_WHAT;
        send_udp_pkt(USRP2_UDP_CTRL_PORT, src, &ctrl_data_out, sizeof(ctrl_data_out));
        break;
    }
    
}

/*
 * Called when an ethernet packet is received.
 * Return true if we handled it here, otherwise
 * it'll be passed on to the DSP Tx pipe
 */
bool
eth_pkt_inspector(dbsm_t *sm, int bufno)
{
  //point me to the ethernet frame
  uint32_t *buff = (uint32_t *)buffer_ram(bufno);

  //pass it to the slow-path handler
  size_t len = buffer_pool_status->last_line[bufno] - 3;
  handle_eth_packet(buff, len);
  return true;
}

//------------------------------------------------------------------

/*
 * Called when eth phy state changes (w/ interrupts disabled)
 */
volatile bool link_is_up = false;	// eth handler sets this
void link_changed_callback(int speed)
{
  link_is_up = speed != 0;
  hal_set_leds(link_is_up ? LED_RJ45 : 0x0, LED_RJ45);
  printf("\neth link changed: speed = %d\n", speed);
  if (link_is_up) send_gratuitous_arp();
}

int main(void)
{
	u2_init();

	putstr("\nTxRx-NEWETH\n");
	print_mac_addr(ethernet_mac_addr()->addr);
	newline();
	print_ip_addr(get_ip_addr()); newline();
	printf("FPGA compatibility number: %d\n", USRP2_FPGA_COMPAT_NUM);
	printf("Firmware compatibility number: %d\n", USRP2_FW_COMPAT_NUM);

	//1) register the addresses into the network stack
	register_mac_addr(ethernet_mac_addr());
	register_ip_addr(get_ip_addr());
  
	//2) register callbacks for udp ports we service
	register_udp_listener(USRP2_UDP_CTRL_PORT, handle_udp_ctrl_packet);
	//register_udp_listener(USRP2_UDP_DATA_PORT, handle_udp_data_packet);

	//3) setup ethernet hardware to bring the link up
	ethernet_register_link_changed_callback(link_changed_callback);
	ethernet_init();

	// initialize wifire stuff
	wifire_init();

	// setup ethernet
	bp_receive_to_buf(ETH_RX_BUF, PORT_ETH, 1, 0, BP_LAST_LINE);

	while(1) {
		if(i2c_done) {
			i2c_done = false;
			send_udp_pkt(USRP2_UDP_CTRL_PORT, i2c_src, &ctrl_data_out, sizeof(ctrl_data_out));
		}

		if(spi_done) {
			spi_done = false;
			send_udp_pkt(USRP2_UDP_CTRL_PORT, spi_src, &ctrl_data_out, sizeof(ctrl_data_out));
		}
	}

	return 0;
}
