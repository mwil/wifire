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

#ifndef CTIME
#define CTIME "not known"
#endif

#include "zb_wifire.h"

#include "buffer_pool.h"
#include "memory_map.h"
#include "pic.h"
#include <net_common.h>
#include <stdarg.h>
#include "nonstdio.h"
#include "hal_io.h"
#include <string.h>

#include <i2c.h>

#define ETH_RX_BUF 1
#define DSP_RX_BUF 2
#define SYM_TX_BUF_0 4
#define SYM_TX_BUF_1 5

static wifire_state_t fw_state;
static uint32_t seqnr = 0;

bool eth_pkt_inspector(dbsm_t *sm, int bufno);
static void start_receiving(void);
/*
#define MY_FRAME_LEN 60
static const uint8_t MY_FRAME[] = {0x00, 0x00, 0x00, 0x00, 0xA7, 0x18, 0x41, 0x88,
								   0x6D, 0x22, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x3F,
								   0x00, 0xFF, 0xFF, 0x00, 0x00, 0x04, 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00, 0x7E, 0xF9};
*/
#define MY_FRAME_LEN 20
static const uint8_t MY_FRAME[] = {0x00, 0x00, 0x00, 0xA7, 0x0F, 0x00, 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00};

tx_symbols_t tx_symbols;

// convert bytes to 802.15.4 symbols (2 * 4 bits), nibbles are switched!
#define FRAME_SYM(buf, i) ((buf[(i>>1)]  >> ((i&1) << 2)) & 0xF)

static uint32_t restore_p[5];
uint32_t restore_at, vita_hdr_len;

uint8_t running, react;
uint32_t react_delay = 4800;

static const char * STATE_NAMES[] = {"IDLE", "REACT", "TX_ZIGBEE"};
static const char * CMD_NAMES[] = {"NOP", "Change OPMODE", "Set Periodic IRQ",
								   "Set TX scale_iq", "Set react delay", "Set TX packet"};


static void append_symbol(volatile uint32_t *p, uint8_t symb, uint32_t at)
{
	uint32_t idx, chip, pulse;
	uint8_t bit_i, bit_q;

	// 16 possible chipping sequences for symbols 0--15
	uint32_t SYM[] = {0xd9c3522e, 0xed9c3522, 0x2ed9c352, 0x22ed9c35,
					  0x522ed9c3, 0x3522ed9c, 0xc3522ed9, 0x9c3522ed,
					  0x8c96077b, 0xb8c96077, 0x7b8c9607, 0x77b8c960,
					  0x077b8c96, 0x6077b8c9, 0x96077b8c, 0xc96077b8};


	// half-sine pulse shape sin(0), sin(pi/4), sin(pi/2), sin(3*pi/4)
	uint32_t PULSE_I[] = {0, 0x5A82, 	0x7FFF, 0x5A82, 0, 		0};
	// Q shifted by half a pulse (2 samples) for quadrature
	uint32_t PULSE_Q[] = {0, 0, 		0, 		0x5A82, 0x7FFF, 0x5A82};

	// negative pulses (going downwards)
	uint32_t NPULSE_I[] = {0, 0xA57D, 	0x8000, 0xA57D, 0, 		0};
	uint32_t NPULSE_Q[] = {0, 0, 		0, 		0xA57D, 0x8000, 0xA57D};

	for (chip = 0; chip < 32; chip += 2) {
		idx = at + 4 * (chip >> 1);

		// select the current chipping bit
		bit_i = (SYM[symb] >> (31 - chip)) & 0x1;
		bit_q =	(SYM[symb] >> (30 - chip)) & 0x1;

		// pulse-shape it, and shift Q back
		for (pulse = 0; pulse < 6; pulse++) {
			if ((idx+pulse) < BP_NLINES) {
				p[idx+pulse] |= ((bit_i?PULSE_I[pulse]:NPULSE_I[pulse]) << 16)
					   		  | ((bit_q?PULSE_Q[pulse]:NPULSE_Q[pulse]) & 0xFFFF);
			}
		}
	}
}

/**
 * Multi-symbol buffer (with a full PHY header) for test purposes.
 */
/*
static void prepare_PHY_header(void)
{
	uint32_t *p = buffer_ram(SYM_BUF_0);

	uint32_t i;
	uint8_t symb;
	uint32_t current = 2;

	uint8_t PHYheader[] = {0, 0, 0, 0, 7, 10, 15};

	// clear the samples in the buffer
	for (i = 0; i < BP_NLINES; i++) {
		p[i] = 0;
	}

	for (symb = 0; symb < 7; symb++) {
		append_symbol(p, PHYheader[symb], current);
		current += 64;
	}

	// jam_lines = 2 + 64 * 7;//BP_NLINES;
}

static void send_header(void) {
	volatile uint32_t *p = buffer_ram(SYM_BUF_0);

	// set vita header
	p[0] = seqnr;
	p[1] = ((64 * 7) + 1) // last byte (WITH HEADER!)
		 | ((seqnr & 0xf) << 16); // ++seqnr to make vita happy

	if (seqnr == 0) { // Start a burst on the first symbol
		 p[1] |= VITA_HDR_SOB;
	}

	bp_send_from_buf(SYM_BUF_0, PORT_DSP, 1, 0, (2 + 64 * 7) - 1);

	seqnr++;
}
*/
/*
static void prepare_next_buffer(void)
{
	uint32_t *p = buffer_ram(curr_buf);

	uint32_t i;
	uint8_t symb, limit;
	uint32_t current = 2;
	uint8_t symbols_left = 60 - current_sym;

	// clear the samples in the buffer
	for (i = 0; i < BP_NLINES; i++) {
		p[i] = 0;
	}

	limit = symbols_left > 6 ? 7 : symbols_left;

	for (symb = 0; symb < limit; symb++) {
		append_symbol(p, MY_FRAME[current_sym+symb], current);
		current += 64;
	}

	if (symbols_left > 6) {
		current_sym += 7;
	} else {
		current_sym = 0;
	}

	jam_lines = 2 + 64 * limit;
}
*/

static void prepare_sym_bufs(void)
{
	volatile uint32_t *p0 = buffer_ram(SYM_TX_BUF_0);
	volatile uint32_t *p = buffer_ram(SYM_TX_BUF_1);

	uint32_t i;
	uint32_t curr = 2;

	uint8_t SYM[] = {7, 7, 0xF, 0xF};

	// clear the samples in the buffer
	for (i = 0; i < BP_NLINES; i++) {
		p[i] = 0; p0[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		append_symbol(p, SYM[i], curr);
		curr += 64;
	}

	printf("Preparing the symbol buffer(s) ... done.\n");
}

static uint32_t get_symbol_pos(uint8_t sym)
{
	uint32_t pos;

	pos = (sym < 8) ? 0 : 128;
	// magical shift (e.g., "6" is a "7" shifted by 4 chips (2 I/Q!!) to the left, and one chip is 4 samples long)
	pos += ((sym ^ 7) & 7) << 3;

	return pos;
}

static uint8_t get_next_symbol(void)
{
	uint8_t res = 0;

	res = FRAME_SYM(tx_symbols.buf, tx_symbols.curr);

	return res;
}

static void send_symbol(uint8_t sym)
{
	volatile uint32_t *p = buffer_ram(SYM_TX_BUF_1);
	uint32_t i;

	vita_hdr_len = 2;
	uint32_t at = vita_hdr_len + get_symbol_pos(sym);
	restore_at = at - vita_hdr_len;

	// rescue words before overwriting them with the vita header
	for (i = 0; i < vita_hdr_len; i++) {
		restore_p[i] = p[at-vita_hdr_len+i];
	}

	// set vita header
	p[at-2] = seqnr;
	p[at-1] = (64 + vita_hdr_len - 1) // length
			| VITA_HDR_SOB			// always start-of-burst (good for continuous streaming)
		    | ((seqnr & 0xf) << 16); // sequence no

	if (tx_symbols.curr == tx_symbols.last) {
		p[at-1] |= VITA_HDR_EOB;
	}

	bp_send_from_buf(SYM_TX_BUF_1, PORT_DSP, 1, at - vita_hdr_len, (at + 64) - 1);

	seqnr++;
	tx_symbols.curr++;
}

static void send_symbol_at_tick(uint8_t sym, uint32_t ticks)
{
	volatile uint32_t *p = buffer_ram(SYM_TX_BUF_1);

	uint32_t i;
	vita_hdr_len = 5;
	uint32_t at = vita_hdr_len + get_symbol_pos(sym);
	restore_at = at - vita_hdr_len;

	// rescue words before overwriting them with the vita header
	for (i = 0; i < vita_hdr_len; i++) {
		restore_p[i] = p[at-vita_hdr_len+i];
	}

	// set vita header
	p[at-5] = seqnr;
	p[at-4] = (64 + vita_hdr_len - 1) // length
		    | ((seqnr & 0xf) << 16) // sequence no
		    | VITA_HDR_SOB
		    | VITA_HDR_HAS_SECS
		    | VITA_HDR_HAS_TICKS;
	p[at-3] = 0; // send in this second
	p[at-2] = 0; // unused ticks
	p[at-1] = ticks; // ticks of the 100MHz clock in the future

	if (tx_symbols.curr == tx_symbols.last) {
		p[at-1] |= VITA_HDR_EOB;
	}

	bp_send_from_buf(SYM_TX_BUF_1, PORT_DSP, 1, at - vita_hdr_len, (at + 64) - 1);
	//putchar('t');

	seqnr++;
	tx_symbols.curr++;
}

// -----------------------------------------------------------------------

static wifire_reply_t handle_set_opmode(uint32_t opmode, wifire_udp_reply_t *reply)
{
	printf("SET_OPMODE %s --> %s\n", STATE_NAMES[fw_state], STATE_NAMES[opmode]);

	switch(opmode) {
		case WIFIRE_OPMODE_IDLE:
			// cleanup previous state
			switch(fw_state) {
				case STATE_REACT:
					// stop receiving wireless signals
					bp_clear_buf(DSP_RX_BUF);
					break;

				case STATE_TX_ZIGBEE:
					pic_regs->pending = PIC_PERIODIC_INT | PIC_BUFFER_INT;
					break;

				default:
					break;
			}

			// change into new state
			fw_state = STATE_IDLE;

			sr_simple_timer->periodic = 0;
			break;

		case WIFIRE_OPMODE_REACT:
			switch(fw_state) {
				case STATE_TX_ZIGBEE:
					//txrx_stop();
					pic_regs->pending = PIC_PERIODIC_INT | PIC_BUFFER_INT;
					sr_simple_timer->periodic = 0;
					break;

				default:
					break;
			}

			// change into new state
			fw_state = STATE_REACT;

			// start looking for SFD fields
			start_receiving();
			break;

		case WIFIRE_OPMODE_TX_ZIGBEE:
			// cleanup previous state
			switch(fw_state) {
				case STATE_REACT:
					pic_regs->pending = PIC_PERIODIC_INT;
					// stop receiving wireless signals
					bp_clear_buf(DSP_RX_BUF);
					break;

				default:
					break;
			}

			// change into new state
			fw_state = STATE_TX_ZIGBEE;

			// start timer to kick off ZigBee packet TX
			sr_simple_timer->periodic = 25e6;
			//sr_tx_ctrl->policy = 2;
			break;

		default:
			return WIFIRE_REPLY_FAILURE;
			break;
	}

	return WIFIRE_REPLY_SUCCESS;
}

static void handle_wifire_control_packet(
	struct socket_address src, struct socket_address dst,
	unsigned char *payload, int payload_len)
{
	wifire_reply_t ok;

	if (payload_len < 4) {
		printf("Packet Length does not match: Expected at least 4 byte but got %d\n", payload_len);
		return;
	}

	const wifire_udp_req_t *req = (wifire_udp_req_t *)payload;

	wifire_udp_reply_t reply;
	reply.cmd = req->cmd;

	printf("\nControl packet: %s\n", CMD_NAMES[req->cmd]);

	switch (req->cmd) {
		case WIFIRE_CMD_SET_OPMODE:
			ok = handle_set_opmode(req->simple.args, &reply);
			break;

		case WIFIRE_CMD_SET_PERIODIC:
			pic_regs->pending = PIC_PERIODIC_INT;
			sr_simple_timer->periodic = req->simple.args;
			ok = WIFIRE_REPLY_SUCCESS;
			break;

		case WIFIRE_CMD_SET_TX_SCALE:
			dsp_tx_regs->scale_iq = req->simple.args;  // default 0x0400'0400
			ok = WIFIRE_REPLY_SUCCESS;
			break;

		case WIFIRE_CMD_SET_REACT_DELAY:
			react_delay = req->simple.args;
			ok = WIFIRE_REPLY_SUCCESS;
			break;

		case WIFIRE_CMD_SET_SYM_BUF:
			memcpy(&tx_symbols, &(req->sym.tx_symbols), sizeof(tx_symbols_t));
			ok = WIFIRE_REPLY_SUCCESS;
			break;

		default:
			printf("Received unknown WiFire command %d.\n", req->cmd);
			ok = WIFIRE_REPLY_FAILURE;
			break;
	}

	reply.flags = ok;
	send_udp_pkt(WIFIRE_UDP_CTRL_PORT, src, &reply, sizeof(reply));
}

// -----------------------------------------------------------------------

static const uint32_t rx_ctrl_word = 1 << 16;
#define DSP_RX_FIRST_LINE sizeof(rx_ctrl_word)/sizeof(uint32_t)

void buffer_irq_handler(unsigned irq)
{
	volatile uint32_t *p;
	uint32_t i;

	int ea = hal_disable_ints();
	uint32_t status = buffer_pool_status->status;

	if (status & BPS_ERROR_ALL) {
		putchar('E');
		bp_clear_buf(ETH_RX_BUF);
		bp_clear_buf(SYM_TX_BUF_0);
		bp_clear_buf(SYM_TX_BUF_1);
	}

	if (status & BPS_DONE(SYM_TX_BUF_0)) {
		switch(fw_state) {
			case STATE_TX_ZIGBEE:
				bp_clear_buf(SYM_TX_BUF_0);

				if(!running) {
					//send_nop();
				} else {
					tx_symbols.curr = 0;
					// send_symbol(get_next_symbol());
				}
				break;

			default:
				break;
		}
	}

	if (status & BPS_DONE(SYM_TX_BUF_1)) {
		p = buffer_ram(SYM_TX_BUF_1);

		// restore overwritten words (by the vita header)
		for (i = 0; i < vita_hdr_len; i++) {
			p[restore_at+i] = restore_p[i];
		}

		bp_clear_buf(SYM_TX_BUF_1);

		switch(fw_state) {
			case STATE_REACT:
				//dsp_rx_regs->rx_mux = 0x4;  // switch back to normal reception mode
				if (tx_symbols.curr <= tx_symbols.last) {
					send_symbol(get_next_symbol());
				} else {
					running = 0;
					tx_symbols.curr = 0;
					putchar('V');

					bp_clear_buf(DSP_RX_BUF);
					bp_receive_to_buf(DSP_RX_BUF, PORT_DSP, 1, DSP_RX_FIRST_LINE, BP_LAST_LINE);
				}
				break;

			case STATE_TX_ZIGBEE:
				if (tx_symbols.curr <= tx_symbols.last) {
					send_symbol(get_next_symbol());
				} else {
					running = 0;
					tx_symbols.curr = 0;
					putchar('S');
				}
				break;

			default:
				break;
		}
	}

	if (status & BPS_DONE(DSP_RX_BUF)) {
		bp_clear_buf(DSP_RX_BUF);

		if (!running && !react) {
			bp_receive_to_buf(DSP_RX_BUF, PORT_DSP, 1, DSP_RX_FIRST_LINE, BP_LAST_LINE);
		}

		if (react) {
			react = 0;
			send_symbol_at_tick(get_next_symbol(), react_delay);
		}
	}

	// handle ethernet data if some arrived
	if (status & BPS_DONE(ETH_RX_BUF)) {
		eth_pkt_inspector(NULL, ETH_RX_BUF);

		bp_clear_buf(ETH_RX_BUF);
		bp_receive_to_buf(ETH_RX_BUF, PORT_ETH, 1, 0, BP_LAST_LINE);

	} else if (status & (BPS_IDLE(ETH_RX_BUF))) {
		putstr("Ethernet idles");
		bp_receive_to_buf(ETH_RX_BUF, PORT_ETH, 1, 0, BP_LAST_LINE);
	}

	hal_restore_ints(ea);
}

void timer_irq_handler(unsigned irq)
{
	//running = 1;
	//sr_time64->ticks = 0;
	//sr_time64->imm = 1;
	//sr_time64->secs = 0;
	putchar('T');

	switch(fw_state) {
		case STATE_TX_ZIGBEE:
			if (!running) {
				send_symbol(get_next_symbol());
				running = 1;
			}
			break;

		default:
			break;
	}
}

void SFD_irq_handler(unsigned irq)
{
	uint32_t c;
	//putchar ('D');
	sr_time64->ticks = 0;
	sr_time64->imm = 1;
	sr_time64->secs = 0;

	//for(c=0;c<800;c++) asm("NOP");

	switch(fw_state) {
		case STATE_REACT:
			if (!running) {
				react = 1;
				running = 1;

				// stop receiving wireless signals
				//bp_clear_buf(DSP_RX_BUF);
				//sr_rx_ctrl->clear_overrun = 1;
				//dsp_rx_regs->rx_mux = 0xF; // drive DDC with constant 0

				//send_symbol_at_tick(get_next_symbol(), 10000);
				//send_symbol(get_next_symbol());
			}
			break;

		default:
			break;
	}

	sr_wifire_ctrl->clear_header_intr = 1;
}


void header_irq_handler(unsigned irq)
{
	putchar('H');
	sr_wifire_ctrl->clear_header_intr = 1;
}

void overrun_irq_handler(unsigned irq)
{
	putchar('O');
}

void underrun_irq_handler(unsigned irq) {
	putchar('U');
}

// -----------------------------------------------------------------------

static void start_receiving()
{
#if (DBOARD == DBOARD_XCVR)
	putstr("Setting up full duplex mode (XCVR Daughterboard).\n");

	// set ATR registers
	atr_reg(ATR_IDLE_TXSIDE) = (HB_PA_OFF_TXIO | TX_DIS_TXIO);
	atr_reg(ATR_INTX_TXSIDE) = (ANTSEL_TX2_RX1_TXIO | HB_PA_OFF_TXIO | TX_ENB_TXIO);
	atr_reg(ATR_INRX_TXSIDE) = (ANTSEL_TX2_RX1_TXIO | HB_PA_OFF_TXIO | TX_DIS_TXIO);
	atr_reg(ATR_FULL_TXSIDE) = (ANTSEL_TX2_RX1_TXIO | HB_PA_OFF_TXIO | TX_ENB_TXIO);

	atr_reg(ATR_IDLE_RXSIDE) = (POWER_UP_RXIO | RX_DIS_RXIO);
	atr_reg(ATR_INTX_RXSIDE) = (POWER_UP_RXIO | RX_DIS_RXIO);
	atr_reg(ATR_INRX_RXSIDE) = (POWER_UP_RXIO | RX_ENB_RXIO);
	atr_reg(ATR_FULL_RXSIDE) = (POWER_UP_RXIO | RX_ENB_RXIO);
#elif (DBOARD == DBOARD_RFX)
	putstr("Setting up full duplex mode (RFX Daughterboard).\n");

	// set ATR registers
	atr_reg(ATR_IDLE_TXSIDE) = (POWER_UP | ANT_XX | MIXER_DIS);
	atr_reg(ATR_INTX_TXSIDE) = (POWER_UP | ANT_TX | MIXER_ENB);
	atr_reg(ATR_INRX_TXSIDE) = (POWER_UP | ANT_RX | MIXER_DIS);
	atr_reg(ATR_FULL_TXSIDE) = (POWER_UP | ANT_TX | MIXER_ENB);

	atr_reg(ATR_IDLE_RXSIDE) = (POWER_UP | ANT_XX | MIXER_DIS);
	atr_reg(ATR_INTX_RXSIDE) = (POWER_UP | ANT_XX | MIXER_DIS);
	atr_reg(ATR_INRX_RXSIDE) = (POWER_UP | ANT_RX2 | MIXER_ENB);
	atr_reg(ATR_FULL_RXSIDE) = (POWER_UP | ANT_RX2 | MIXER_ENB);
#endif

	// putstr("Forcing RX-part to start receiving.\n");

	sr_rx_ctrl->clear_overrun = 1;

	bp_clear_buf(DSP_RX_BUF);
	bp_receive_to_buf(DSP_RX_BUF, PORT_DSP, 1, 0, BP_LAST_LINE);

	// force rx-ctrl to start receiving
	sr_rx_ctrl->cmd = (0xE0000000 // cmd
			| BP_NLINES); // number of lines
	sr_rx_ctrl->time_secs = 0;
	sr_rx_ctrl->time_ticks = 0;
}

#define I2C_DEV_EEPROM	0x50		// 24LC02[45]:  7-bits 1010xxx
#define	I2C_ADDR_TX_A	(I2C_DEV_EEPROM | 0x4)
#define	I2C_ADDR_RX_A	(I2C_DEV_EEPROM | 0x5)

void read_dbids()
{
	  unsigned char dbid_tx[2];
	  unsigned char dbid_rx[2];
	  bool ok;

	  ok = eeprom_read(I2C_ADDR_TX_A, 1, dbid_tx, 2);
	  if (!ok){
	    puts("failed to read Tx Daugherboard EEPROM");
	  }
	  else {
	    putstr("Tx Daugherboard ID: ");
	    puthex8(dbid_tx[1]);    // MSB
	    puthex8(dbid_tx[0]);    // LSB
	    newline();
	  }

	  ok = eeprom_read(I2C_ADDR_RX_A, 1, dbid_rx, 2);
	  if (!ok){
	    puts("failed to read Rx Daugherboard EEPROM");
	  }
	  else {
	    putstr("Rx Daugherboard ID: ");
	    puthex8(dbid_rx[1]);    // MSB
	    puthex8(dbid_rx[0]);    // LSB
	    newline();
	  }
}

void wifire_init(void)
{
	printf("\nWifire App (built: %s)\n", CTIME);

	register_udp_listener(WIFIRE_UDP_CTRL_PORT, &handle_wifire_control_packet);

	prepare_sym_bufs();
	tx_symbols.curr = 0;
	memcpy(tx_symbols.buf, MY_FRAME, MY_FRAME_LEN);
	tx_symbols.len = MY_FRAME_LEN;
	tx_symbols.last = (tx_symbols.len * 2) - 1;

	pic_register_handler(IRQ_PERIODIC, timer_irq_handler);
	pic_register_handler(IRQ_BUFFER, buffer_irq_handler);
	pic_register_handler(IRQ_UNDERRUN, underrun_irq_handler);
	pic_register_handler(IRQ_OVERRUN, overrun_irq_handler);

	//pic_register_handler(IRQ_SFD, SFD_irq_handler);
	pic_register_handler(IRQ_HEADER, SFD_irq_handler);
	// pic_register_handler(IRQ_HEADER, header_irq_handler);

	fw_state = STATE_IDLE;
	running = 0;

	read_dbids();
}
