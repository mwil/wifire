//
// Copyright 2009-2010 Ettus Research LLC
// Copyright 2009-2011 Disco Labs, TU Kaiserslautern
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "memory_map.h"

// ports
#define WIFIRE_UDP_CTRL_PORT 49154

// commands
#define WIFIRE_CMD_NOP 0
#define WIFIRE_CMD_SET_OPMODE 1
#define WIFIRE_CMD_SET_PERIODIC 2
#define WIFIRE_CMD_SET_TX_SCALE 3
#define WIFIRE_CMD_SET_REACT_DELAY 4
#define WIFIRE_CMD_SET_SYM_BUF 5

// opmodes
#define WIFIRE_OPMODE_IDLE 0
#define WIFIRE_OPMODE_REACT 1
#define WIFIRE_OPMODE_TX_ZIGBEE 2

#define VITA_HDR_HAS_TICKS (1 << 20)
#define VITA_HDR_HAS_SECS (1 << 22)
#define VITA_HDR_EOB (1 << 24)
#define VITA_HDR_SOB (1 << 25)

typedef struct {
	uint8_t len;
	uint8_t buf[64];
	uint8_t curr;
	uint8_t last;
} tx_symbols_t;

typedef struct {
	uint32_t cmd;
	union {
		struct {
			uint32_t args;
		} simple;
		struct {
			tx_symbols_t tx_symbols;
		} sym;
	};
} wifire_udp_req_t;

typedef struct {
	uint32_t cmd;
	uint32_t flags;
} wifire_udp_reply_t;

// wifire settings register
#define SR_WIFIRE_CTRL 240

typedef struct {
  volatile uint32_t     clear_header_intr;
  volatile uint32_t     clear_msdu_intr;
} sr_wifire_ctrl_t;

#define sr_wifire_ctrl ((sr_wifire_ctrl_t *) _SR_ADDR(SR_WIFIRE_CTRL))

// interrupt
#define IRQ_SFD        13
#define IRQ_HEADER     14
#define IRQ_MSDU       15

#define PIC_SFD_INT     IRQ_TO_MASK(IRQ_SFD)
#define PIC_HEADER_INT  IRQ_TO_MASK(IRQ_HEADER)
#define PIC_MSDU_INT    IRQ_TO_MASK(IRQ_MSDU)

// wifire app states
typedef enum {
	STATE_IDLE,
	STATE_REACT,
	STATE_TX_ZIGBEE
} wifire_state_t;

typedef enum {
	WIFIRE_REPLY_FAILURE,
	WIFIRE_REPLY_SUCCESS
} wifire_reply_t;

typedef struct  {
	volatile uint32_t len;
	volatile uint32_t frame_ctrl;
	volatile uint32_t seqno;
	volatile uint32_t dst_pan;
	volatile uint32_t dst_addr_h;
	volatile uint32_t dst_addr_l;
	volatile uint32_t src_pan;
	volatile uint32_t src_addr_h;
	volatile uint32_t src_addr_l;
	volatile uint32_t sfd_tics_h;
	volatile uint32_t sfd_tics_l;

	volatile uint32_t msdu_pos;

	volatile uint32_t time_h;
	volatile uint32_t time_l;

	volatile uint32_t header_tics_h;
	volatile uint32_t header_tics_l;
} wifire_t;

#define wifire_status ((const wifire_t*)0xf000)

#define wifire_data ((const volatile uint32_t*)0xf200)

#define BPS_EVENT(bufno)	(BPS_DONE(bufno) | BPS_ERROR(bufno) | BPS_IDLE(bufno))

void wifire_init(void);
void wifire_handler(void);

#define DBOARD_XCVR 1
#define DBOARD_RFX 2


// stolen from dboard_iface
#define ATR_IDLE_TXSIDE  0
#define ATR_IDLE_RXSIDE  2
#define ATR_INTX_TXSIDE  4
#define ATR_INTX_RXSIDE  6
#define ATR_INRX_TXSIDE  8
#define ATR_INRX_RXSIDE  10
#define ATR_FULL_TXSIDE  12
#define ATR_FULL_RXSIDE  14

#if (DBOARD == DBOARD_XCVR)
// TX IO Pins
#define HB_PA_OFF_TXIO      (1 << 15)    // 5GHz PA, 1 = off, 0 = on
#define LB_PA_OFF_TXIO      (1 << 14)    // 2.4GHz PA, 1 = off, 0 = on
#define ANTSEL_TX1_RX2_TXIO (1 << 13)    // 1 = Ant 1 to TX, Ant 2 to RX
#define ANTSEL_TX2_RX1_TXIO (1 << 12)    // 1 = Ant 2 to TX, Ant 1 to RX
#define TX_EN_TXIO          (1 << 11)    // 1 = TX on, 0 = TX off
#define AD9515DIV_TXIO      (1 << 4)     // 1 = Div  by 3, 0 = Div by 2

#define TXIO_MASK (HB_PA_OFF_TXIO | LB_PA_OFF_TXIO | ANTSEL_TX1_RX2_TXIO | ANTSEL_TX2_RX1_TXIO | TX_EN_TXIO | AD9515DIV_TXIO)

// TX IO Functions
#define HB_PA_TXIO               LB_PA_OFF_TXIO
#define LB_PA_TXIO               HB_PA_OFF_TXIO
#define TX_ENB_TXIO              TX_EN_TXIO
#define TX_DIS_TXIO              0
#define AD9515DIV_3_TXIO         AD9515DIV_TXIO
#define AD9515DIV_2_TXIO         0

// RX IO Pins
#define LOCKDET_RXIO (1 << 15)           // This is an INPUT!!!
#define POWER_RXIO   (1 << 14)           // 1 = power on, 0 = shutdown
#define RX_EN_RXIO   (1 << 13)           // 1 = RX on, 0 = RX off
#define RX_HP_RXIO   (1 << 12)           // 0 = Fc set by rx_hpf, 1 = 600 KHz

#define RXIO_MASK (POWER_RXIO | RX_EN_RXIO | RX_HP_RXIO)

// RX IO Functions
#define POWER_UP_RXIO            POWER_RXIO
#define POWER_DOWN_RXIO          0
#define RX_ENB_RXIO              RX_EN_RXIO
#define RX_DIS_RXIO              0

#elif (DBOARD == DBOARD_RFX)
#define POWER_IO     (1 << 7)   // Low enables power supply
#define ANTSW_IO     (1 << 6)   // On TX DB, 0 = TX, 1 = RX, on RX DB 0 = main ant, 1 = RX2
#define MIXER_IO     (1 << 5)   // Enable appropriate mixer
#define LOCKDET_MASK (1 << 2)   // Input pin

// Mixer constants
#define MIXER_ENB    MIXER_IO
#define MIXER_DIS    0

// Power constants
#define POWER_UP     0
#define POWER_DOWN   POWER_IO

// Antenna constants
#define ANT_TX       0          //the tx line is transmitting
#define ANT_RX       ANTSW_IO   //the tx line is receiving
#define ANT_TXRX     0          //the rx line is on txrx
#define ANT_RX2      ANTSW_IO   //the rx line in on rx2
#define ANT_XX       0          //dont care how the antenna is set

#else
#error "No Daughterboard definition: define macro DBOARD to either DBOARD_XCVR or DBOARD_RFX"
#endif

#define atr_reg(x) *(volatile uint16_t *)(0xE400+x)

