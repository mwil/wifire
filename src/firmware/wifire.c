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

#define DBG

#include "wifire.h"
#include <wftable.h>
#include <wfmatches.h>

#include "mdelay.h"
#include "dbsm.h"
#include "buffer_pool.h"
#include "memory_map.h"
#include "pic.h"
#include <net_common.h>
#include <stdarg.h>
#include "nonstdio.h"
#include "hal_io.h"
#include <string.h>
#include "rand2.h"

#define BUF_JAM 4
#define BUF_ETH 5
#define BUF_RCV 2

static wifire_state_t fw_state = STATE_TXRX; // start in normal TXRX-Mode
static uint32_t seqnr;
static uint32_t jam_cntr = 0;
static uint16_t how_many_buffers = 1; // 1 is very short but enough
static uint16_t jam_lines = BP_NLINES / 2;
static uint16_t irq_counter = 0;
static uint8_t wifire_debug = 0; // debug
static uint8_t wifire_jam_type = 0; // irq listening definition (SFD or WiFire or none)

static struct socket_address udp_report_dst = { .port = 49155 };
static uint8_t udp_reports = 0;
//static uint8_t report_power = 0;

#define WF_TABLE_MEMORY_SIZE 1024
uint8_t WF_tableMemory[WF_TABLE_MEMORY_SIZE];
struct WF_Chain * WF_mainChain = (struct WF_Chain*)WF_tableMemory;

extern dbsm_t dsp_tx_sm;
extern dbsm_t dsp_rx_sm;


static wifire_payload_t wifire_payload;

// is not defined in memory_map.h, so define it here..
#define PIC_PERIODIC_INT     IRQ_TO_MASK(IRQ_PERIODIC)


static void timer_irq_handler(unsigned irq);
static void irq_jam(unsigned irq);
//static void irq_count(unsigned irq);
//static void irq_tune_sfd(unsigned irq);
static void irq_header(unsigned irq);
static void irq_msdu(unsigned irq);
static void irq_tune_wifire(unsigned irq);
static inline void wifire_buffer_handler();
static void handle_wifire_control_packet(
	struct socket_address src, struct socket_address dst,
	unsigned char *payload, int payload_len);
bool
eth_pkt_inspector(dbsm_t *sm, int bufno);

static void send_report(const wifire_udp_reply_t * reply, size_t sz);
static void wifire_set_irqs();

static const char * STATE_NAMES[3] = {
	"TXRX",
	"WiFire",
	"JAMMING"
};

static bool periodic_jam_state; // true: jam;  false: idle
static uint32_t jam_ticperiod=100000000, idle_ticperiod=200000000;
static int32_t max_period_variance=0;


static inline void getTics(Tics * tics) {
	tics->tics_h = wifire_status->time_h;
	tics->tics_l = wifire_status->time_l;
}

uint32_t diff(Tics * start, Tics * end) {
	unsigned long h = (end->tics_h - start->tics_h);
	unsigned long l = (end->tics_l - start->tics_l);
	if (end->tics_l < start->tics_l) --h;
	//l = l / 100 + (((h % 100) << 5 / 25) << 25);
	return l;
}

void wifire_init() {
	printf("\nWifireApp (built: %s)\n", CTIME);
	
	register_udp_listener(WIFIRE_UDP_CTRL_PORT, &handle_wifire_control_packet);

	srand2(42);
	
	WF_initChain(WF_mainChain);

	WF_registerMatch(&WF_sourceMatch);
	WF_registerMatch(&WF_destMatch);
	WF_registerMatch(&WF_frameTypeMatch);
	WF_registerMatch(&WF_probabilityMatch);
	WF_registerMatch(&WF_ctpMatch);
	WF_registerMatch(&WF_lqiMatch);

	periodic_jam_state = true;
	sr_simple_timer->periodic = idle_ticperiod;	// number of clock cycles
	

	fw_state = STATE_TXRX;
}

static void start_receiving() {
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

	putstr("Forcing RX-part to start receiving.\n");

	sr_rx_ctrl->clear_overrun = 1;

	bp_clear_buf(BUF_RCV);
	bp_receive_to_buf(BUF_RCV, PORT_DSP, 1, 0, BP_LAST_LINE);

	// force rx-ctrl to start receiving
	sr_rx_ctrl->cmd = (0xE0000000 // cmd
			| BP_NLINES); // number of lines
	sr_rx_ctrl->time_secs = 0;
	sr_rx_ctrl->time_ticks = 0;
}

static void stop_receiving () {
	putstr("Forcing RX-part to stop receiving.\n");
	// force rx-ctrl to stop receiving
	sr_rx_ctrl->clear_overrun = 1;
/*
	sr_rx_ctrl->cmd = (0x00000000); // cmd
	sr_rx_ctrl->time_secs = 0;
	sr_rx_ctrl->time_ticks = 0;
*/
}

static void prepare_to_jam() {
	// fill buffer
	uint32_t *p = buffer_ram(BUF_JAM);
	uint32_t i;
	uint32_t sample = 0x00007fff;
	
	for (i = 2; i < BP_NLINES; i++) {
		p[i] = sample;
	}
	
	seqnr = 0;
	
	putstr("Filled buffer.\n");
}

static void jam() {
	bp_clear_buf(BUF_JAM);

	volatile uint32_t *p = buffer_ram(BUF_JAM);
	// set vita header
	p[0] = seqnr;
	p[1] = (jam_lines - 1)// length
			| (1 << 25)
			| (1 << 24)
			| ((seqnr & 0xf) << 16); // sequence no*/

	// fire it up
	bp_send_from_buf(BUF_JAM, PORT_DSP, 1, 0, /*BP_LAST_LINE*/ jam_lines - 1);

	++seqnr;
}

static uint32_t min = 0xffffffff, max = 0, avg = 0, avg2 = 0;


static void react() {
	uint16_t cntr = 0;

	// stop receiving wireless signals
	bp_clear_buf(BUF_RCV);

	// start in a well known state
	bp_clear_buf(BUF_JAM);

	// send data
	cntr = 0;
	while (cntr++ < how_many_buffers) { // send n packets
		jam();
		// wait for it to complete
		while ((buffer_pool_status->status & (BPS_DONE(BUF_JAM) | BPS_ERROR(BUF_JAM))) == 0)
			;
	}
	bp_clear_buf(BUF_JAM);
}

static void txrx_stop() {
	/* stop dbsm */
	dbsm_stop(&dsp_tx_sm);
	dbsm_stop(&dsp_rx_sm);

	/* reset vita chain */
	sr_tx_ctrl->clear_state = 1;
	sr_tx_ctrl->cyc_per_up = 0;
	sr_tx_ctrl->packets_per_up = 0;

	/* stop reporting context packets (vita_chain_tx.v) */
	sr_tx_ctrl->report = 0;

	/* clear interrupts */
	pic_regs->pending = PIC_UNDERRUN_INT | PIC_OVERRUN_INT;

	/* setup ethernet */
	bp_clear_buf(BUF_ETH);
	bp_receive_to_buf(BUF_ETH, PORT_ETH, 1, 0, BP_LAST_LINE);

	/* prepare jamming buffer */
	prepare_to_jam();
}

static void txrx_start() {
	/* stop all buffers */
	bp_clear_buf(BUF_JAM);
	bp_clear_buf(BUF_ETH);
	bp_clear_buf(BUF_RCV);

	/* clear vita */
	sr_tx_ctrl->clear_state = 1;
	/* generate context packets again */
	sr_tx_ctrl->report = 1;

	/* clear interrupts */
	pic_regs->pending = PIC_UNDERRUN_INT | PIC_OVERRUN_INT;

	/* restart dbsm */
	dbsm_start(&dsp_tx_sm);
	/* NOTE: rx will be started by handle_udp_data_packet */
}

static void wifire_start() {
	wifire_set_irqs();
	start_receiving();
}

static void wifire_stop() {
	pic_unregister_handler(IRQ_SFD);
	pic_unregister_handler(IRQ_HEADER);
	pic_unregister_handler(IRQ_MSDU);
	stop_receiving();
	pic_regs->pending = PIC_SFD_INT | PIC_HEADER_INT | PIC_MSDU_INT;
}

static void wifire_set_irqs() {
	if (fw_state == STATE_WIFIRE) {
		pic_regs->pending = PIC_SFD_INT | PIC_HEADER_INT | PIC_MSDU_INT;
		switch (wifire_jam_type) {
		case WIFIRE_JAM_SFD:
			pic_register_handler(IRQ_SFD, &irq_jam);
			pic_unregister_handler(IRQ_HEADER);
			pic_unregister_handler(IRQ_MSDU);
			break;
		case WIFIRE_JAM_WIFIRE:
			pic_register_handler(IRQ_HEADER, &irq_header);
			pic_unregister_handler(IRQ_SFD);
			pic_unregister_handler(IRQ_MSDU);
			break;
		case WIFIRE_JAM_MSDU:
			pic_register_handler(IRQ_MSDU, &irq_msdu);
			pic_unregister_handler(IRQ_HEADER);
			pic_unregister_handler(IRQ_SFD);
			break;
		/*case WIFIRE_JAM_TUNE_SFD:
			pic_register_handler(IRQ_SFD, &irq_tune_sfd);
			pic_unregister_handler(IRQ_WIFIRE);
			break;*/
		case WIFIRE_JAM_TUNE_WIFIRE:
			pic_register_handler(IRQ_HEADER, &irq_tune_wifire);
			pic_unregister_handler(IRQ_SFD);
			pic_unregister_handler(IRQ_MSDU);
			break;
		/*case WIFIRE_JAM_COUNT:
			pic_register_handler(IRQ_SFD, &irq_count);
			pic_unregister_handler(IRQ_WIFIRE);
			break;*/
		default:
			pic_unregister_handler(IRQ_SFD);
			pic_unregister_handler(IRQ_HEADER);
			pic_unregister_handler(IRQ_MSDU);
		}
	}
}

void printChain(struct WF_Chain * chain) {
	printf("at %x: Verdict: %d, name: %s, endrule: %x\n", chain, chain->policy, chain->name, chain->endrule);
	struct WF_Rule * r = chain->rules;
	while (r != chain->endrule) {
		struct WF_Match * m = r->matches;
		printf(" Rule: Target: %d\n", r->target.v);
		while (m != r->endmatch) {
			printf("  Match: %d %x %x\n", m->negate, m->fun, m->data);
			++m;
		}
		if (r->target.v == WF_JMP) {
			printf("Jumping into target chain\n");
			printChain(r->target.jmpTarget);
		}
		r = WF_nextRule(r);
	}
}

static void handle_wifire_control_packet(
	struct socket_address src, struct socket_address dst,
	unsigned char *payload, int payload_len) {

	if (payload_len < 4) {
		printf("Packet Length does not match: Expected at least 4 byte but got %d\n", payload_len);
		return;
	}

	uint32_t cmd = ntohl(((uint32_t*)payload)[0]);
	typedef union {
		struct {
			uint32_t buffers;
			uint32_t lines;
		} num;
		struct {
			uint32_t mode;
			uint32_t panid;
			uint32_t addr_h;
			uint32_t addr_l;
		} addr;
		struct {
			uint32_t low_en;
			uint32_t low;
			uint32_t high_en;
			uint32_t high;
		} pwr;
		struct {
			uint32_t en;
			uint32_t threshold;
			uint32_t timeout;
		} rfpwr;
		struct {
			uint32_t enable;
			uint32_t timeout;
			uint32_t threshold;
			uint32_t hysteresis;
		} rfkill;
		struct {
			uint32_t type;
		} ctrltype;
		struct {
			uint32_t enables;
		} rules;
		struct {
			uint32_t size;
			uint8_t data[0];
		} wifire;
		struct  {
		    uint32_t jam_period;
		    uint32_t idle_period;
		    uint32_t period_variance;
		} periods;
		uint32_t data;
	} data_t;

	const data_t * data = (data_t*)(payload + 4);

	switch (cmd) {
	case CMD_OPMODE_JAM:
		printf("Command: OPMODE_JAM. Current state: %s\n", STATE_NAMES[fw_state]);

		/* cleanup current state */
		switch (fw_state) {
		case STATE_TXRX:
			txrx_stop();
			break;
		case STATE_JAMMING:
			putstr("\tAlready in jamming mode, nothing to do.\n");
			return;
		case STATE_WIFIRE:
			wifire_stop();
			break;
		}

		/* change into new state */
		fw_state = STATE_JAMMING;
		putstr("New State: Jamming\n");

		pic_register_handler(IRQ_PERIODIC, timer_irq_handler);

		/* do it */
		// always start in jamming state, so BPS_DONE will be set later..
		jam();

		break;
	case CMD_OPMODE_TXRX:
		printf("Command: OPMODE_TXRX. Current state: %s\n", STATE_NAMES[fw_state]);

		/* cleanup current state */
		switch (fw_state) {
		case STATE_TXRX:
			putstr("\tAlready in txrx mode, nothing to do.\n");
			return;
		case STATE_JAMMING:
			pic_regs->pending = PIC_PERIODIC_INT;
			pic_unregister_handler(IRQ_PERIODIC);
			break;
		case STATE_WIFIRE:
			wifire_stop();
			break;
		}

		/* change into new state */
		fw_state = STATE_TXRX;
		putstr("New State: TXRX\n");

		/* do it */
		txrx_start();

		break;
	case CMD_OPMODE_WIFIRE:
		printf("Command: OPMODE_WIFIRE. Current state: %s\n", STATE_NAMES[fw_state]);

		/* cleanup current state */
		switch (fw_state) {
		case STATE_TXRX:
			txrx_stop();
			break;
		case STATE_JAMMING:
			pic_regs->pending = PIC_PERIODIC_INT;
			pic_unregister_handler(IRQ_PERIODIC);
			break;
		case STATE_WIFIRE:
			putstr("\tAlready in wifire mode, nothing to do.\n");
			return;
		}

		/* change into new state */
		fw_state = STATE_WIFIRE;
		putstr("New State: WiFire\n");

		/* do it */
		wifire_start();

		break;
	case CMD_SET_JAM_LENGTH:
		how_many_buffers = data->num.buffers;
		jam_lines = data->num.lines;
		irq_counter = 0;

		printf("Set jamming length to %d buffers and %d lines.\n", how_many_buffers, jam_lines);
		break;
	case CMD_SET_TIMER_PERIODS:
		jam_ticperiod = data->periods.jam_period;
		idle_ticperiod = data->periods.idle_period;
		max_period_variance = data->periods.period_variance;
		break;
	case CMD_SET_DEBUG:
		wifire_debug = data->data & 0xff;

		printf("Set debug to 0x%x\n", wifire_debug);
		break;
	case CMD_PING:
		putstr("Pong.\n");
		break;
	case CMD_PRINT_COUNTERS:
		if (udp_reports) {
			wifire_udp_reply_t rep;
			rep.cmd = REP_COUNTER;
			rep.counter.val = jam_cntr;

			send_report(&rep, sizeof rep.counter);
		} else {
			putstr("Counters:\n");
			printf("\tjammed packets: %d\n", jam_cntr);
		}

		break;
	case CMD_RESET_COUNTERS:
		putstr("Setting counters to zero.\n");
		jam_cntr = 0;

		putstr("Set counters to 0\n");
		break;
	case CMD_SET_JAM_TYPE:
		printf("Set Jamming Type to %d\n", data->data);
		wifire_jam_type = data->data;
		wifire_set_irqs();
		break;
	case CMD_WIFIRE_CONFIG_SIZE: {
		uint32_t data[3];
		data[0] = REP_WIFIRE_CONFIG_SIZE;
		data[1] = WF_TABLE_MEMORY_SIZE;
		data[2] = (uint32_t)WF_mainChain;
		send_udp_pkt(WIFIRE_UDP_CTRL_PORT, src, data, sizeof data);
		break;
	}
	case CMD_WIFIRE_MATCHES: {
		uint32_t size = WF_reportMatchesSize();
		uint32_t report[size + 1];
		report[0] = REP_WIFIRE_MATCHES;
		WF_reportMatches(report + 1);
		send_udp_pkt(WIFIRE_UDP_CTRL_PORT, src, report, sizeof report);
		break;
	}
	case CMD_WIFIRE_CONFIGURE: {
		uint32_t size = data->wifire.size;
		printf("Reported Size: %d, got: %d\n", size, payload_len - 8);
		memcpy(WF_mainChain, data->wifire.data, size);

		printChain(WF_mainChain);

		break;
	}
	/*
	case CMD_REPORT_POWER:
		report_power = data->data;
		break;*/
	case CMD_UDP_REPORTS:
		udp_reports = data->data;
		udp_report_dst.addr = src.addr;
		break;
	default:
		printf("Received unknown wifire command %d.\n", cmd);
	}
}

static inline void wifire_buffer_handler() {
	int ea = hal_disable_ints();

	uint32_t status = buffer_pool_status->status;

	// behavior according to wifire state
	switch (fw_state) {
	case STATE_JAMMING:
		// send jamming data if done
		if (status & (BPS_DONE(BUF_JAM) | BPS_ERROR(BUF_JAM)))
		{
		    if(periodic_jam_state)
			jam();
		}
		break;
	case STATE_WIFIRE:
		// listening, detecting (fpga) and reaction (here)
		// handle errors
		if (status & BPS_ERROR(BUF_JAM)) {
			putstr("E (jam)\n");
			bp_clear_buf(BUF_JAM); // just clear error
		}

		if (status & BPS_ERROR(BUF_RCV)) {
			putstr("E (rcv)\n");
			bp_clear_buf(BUF_RCV);
			bp_receive_to_buf(BUF_RCV, PORT_DSP, 1, 0, BP_LAST_LINE);
		}

		if (status & BPS_ERROR(BUF_ETH)) {
			putstr("E (eth)\n");
			bp_clear_buf(BUF_ETH);
			bp_receive_to_buf(BUF_ETH, PORT_ETH, 1, 0, BP_LAST_LINE);
		}

		// handle completed transactions
		if (status & BPS_DONE(BUF_JAM)) {
			// sent data, just clear. sending again has to be done somewhere else
			bp_clear_buf(BUF_JAM);
		}

		if (status & BPS_DONE(BUF_RCV)) {
			// read data from dsp, throw it away and read again
			bp_clear_buf(BUF_RCV);
			bp_receive_to_buf(BUF_RCV, PORT_DSP, 1, 0, BP_LAST_LINE);
		} else if (status & BPS_IDLE(BUF_RCV)) {
			bp_receive_to_buf(BUF_RCV, PORT_DSP, 1, 0, BP_LAST_LINE);
		}


/*
		if (report_power && udp_reports) {
			wifire_udp_reply_t rep;
			rep.cmd = REP_POWER;
			rep.power.power = wifire_status->power;
			rep.power.time_h = wifire_status->time_h;
			rep.power.time_l = wifire_status->time_l;

			send_report(&rep, sizeof rep.power);
		}*/

		break;
	default:
		printf("Wrong State %s", STATE_NAMES[fw_state]);
		hal_restore_ints(ea);
		return;
	}

	// handle ethernet data if some arrived
	if (status & (BPS_DONE(BUF_ETH) | BPS_ERROR(BUF_ETH))) {
		bp_clear_buf(BUF_ETH);
		eth_pkt_inspector(NULL, BUF_ETH);
		if (fw_state != STATE_TXRX) {
			/* if state has changed to TXRX, do not refill the buffer */
			bp_receive_to_buf(BUF_ETH, PORT_ETH, 1, 0, BP_LAST_LINE);
		}
	} else if (status & (BPS_IDLE(BUF_ETH))) {
		putstr("Ethernet idles");
		bp_receive_to_buf(BUF_ETH, PORT_ETH, 1, 0, BP_LAST_LINE);
	}

	hal_restore_ints(ea);
}

static void send_report(const wifire_udp_reply_t * reply, size_t sz) {
	send_udp_pkt(WIFIRE_UDP_CTRL_PORT, udp_report_dst, reply, sizeof(reply->cmd) + sz);
}

static void irq_jam(unsigned irq) {
	putchar('D');
	//if (++irq_counter >= irq_threshold) {
		irq_counter = 0;
		//jam();
		react();

		jam_cntr++;

		if (wifire_debug & PRINT_IRQ)
			printf("Detected packet #%d\n", jam_cntr);
	//}
	//sr_wifire_ctrl->clear_intr = 1;
}
/*
static void irq_tune_sfd(unsigned irq) {
	jam_cntr++;
	if (udp_reports) {
		wifire_udp_reply_t reply;
		reply.cmd = REP_TUNE_SFD;
		reply.sfd.pwr = wifire_status->power_sfd;
		reply.sfd.tics = wifire_status->sfdtics;
		send_report(&reply, sizeof reply.sfd);
	} else {
		uint32_t power = wifire_status->power_sfd;
		printf("SFD at %d tics, power %x %d\n", wifire_status->sfdtics, power, power);
	}
}*/
/*
static void irq_count(unsigned irq) {
	jam_cntr++;
}*/


static void timer_irq_handler(unsigned irq)
{
    int32_t period_variance = (rand2() % (2 * max_period_variance + 1)) - max_period_variance;

    pic_regs->pending = PIC_PERIODIC_INT;

    // alternate between idling and jamming and reset timer period
    if(periodic_jam_state) // currently jaming
    {
	sr_simple_timer->periodic = idle_ticperiod + period_variance;
    }
    else // currently idling
    {
	sr_simple_timer->periodic = jam_ticperiod + period_variance;
    }

    periodic_jam_state = !periodic_jam_state;

    if(udp_reports)
    {
	wifire_udp_reply_t reply;
	reply.cmd = REP_GENERIC;
	reply.generic.key = 0;
	reply.generic.value = periodic_jam_state;
	send_report(&reply, sizeof reply.generic);
    }

}

static void irq_header(unsigned irq) {
	putchar('H');
	enum WF_Verdict verdict = WF_getVerdict(WF_mainChain, (const struct WF_Header*)wifire_status, NULL);
	if (verdict == WF_JAM) {
		//printf("Verdict: Jam\n");
		react();
	} else {
		//printf("Verdict: Accept\n");
	}
	if (udp_reports) {
		wifire_udp_reply_t reply;
		reply.cmd = REP_TUNE_WIFIRE;
		reply.wifire.len = wifire_status->len;
		reply.wifire.fctrl = wifire_status->frame_ctrl;
		reply.wifire.seqno = wifire_status->seqno;
		reply.wifire.dstpan = wifire_status->dst_pan;
		reply.wifire.dstaddr_h = wifire_status->dst_addr_h;
		reply.wifire.dstaddr_l = wifire_status->dst_addr_l;
		reply.wifire.srcpan = wifire_status->src_pan;
		reply.wifire.srcaddr_h = wifire_status->src_addr_h;
		reply.wifire.srcaddr_l = wifire_status->src_addr_l;
		reply.wifire.verdict = verdict;
		reply.wifire.delay = 0;
		send_report(&reply, sizeof reply.wifire);
	}
	sr_wifire_ctrl->clear_header_intr = 1;
}

static void irq_msdu(unsigned irq)
{
    // wifire_data is uint32_t array, but need to access single bytes
    uint8_t byte_pos = (wifire_status->msdu_pos & ~3) | (3 - (wifire_status->msdu_pos & 3));
    uint8_t rx_byte = ((uint8_t*)wifire_data)[byte_pos];

    if(wifire_status->msdu_pos >= 128)
	goto out;

    wifire_payload.payload[wifire_status->msdu_pos] = rx_byte;
    wifire_payload.len = wifire_status->msdu_pos + 1;

    enum WF_Verdict verdict = WF_getVerdict(WF_mainChain, (const struct WF_Header*)wifire_status, (const struct WF_Payload*)&wifire_payload);
    if(verdict == WF_JAM)
	react();

    if(udp_reports)
    {
	wifire_udp_reply_t reply;
	reply.cmd = REP_MSDU;
	reply.msdu.len = wifire_status->len;
	reply.msdu.pos = wifire_status->msdu_pos;
	reply.msdu.rx_byte = rx_byte;
	//memcpy(&reply.msdu.payload, ((uint8_t*)wifire_data), wifire_status->len - 8);
	//reply.msdu.payload = ((uint8_t*)wifire_data)[wifire_status->msdu_pos];
	send_report(&reply, sizeof reply.msdu);
    }

out:
    sr_wifire_ctrl->clear_msdu_intr = 1;
}

static void irq_tune_wifire(unsigned irq) {
	Tics start, end;
	//getTics(&end); // ti
	getTics(&start);
	enum WF_Verdict verdict = WF_getVerdict(WF_mainChain, (const struct WF_Packet*)wifire_status, NULL);
	getTics(&end);

	/*start.tics_h = wifire_status->sfd_tics_h;
	start.tics_l = wifire_status->sfd_tics_l;
	end.tics_h = wifire_status->header_tics_h;
	end.tics_l = wifire_status->header_tics_l;*/
	uint32_t e = diff(&start, &end);
	if (e < min) min = e;
	if (e > max) max = e;
	avg+=e;
	avg2++;

	if (udp_reports) {
		wifire_udp_reply_t reply;
		reply.cmd = REP_TUNE_WIFIRE;
		reply.wifire.len = wifire_status->len;
		reply.wifire.fctrl = wifire_status->frame_ctrl;
		reply.wifire.seqno = wifire_status->seqno;
		reply.wifire.dstpan = wifire_status->dst_pan;
		reply.wifire.dstaddr_h = wifire_status->dst_addr_h;
		reply.wifire.dstaddr_l = wifire_status->dst_addr_l;
		reply.wifire.srcpan = wifire_status->src_pan;
		reply.wifire.srcaddr_h = wifire_status->src_addr_h;
		reply.wifire.srcaddr_l = wifire_status->src_addr_l;
		reply.wifire.verdict = verdict;
		reply.wifire.delay = e;
		reply.wifire.delay_min = min;
		reply.wifire.delay_max = max;
		reply.wifire.delay_avg = avg;
		reply.wifire.delay_avg2 = avg2;
		send_report(&reply, sizeof reply.wifire);
	}
	sr_wifire_ctrl->clear_header_intr = 1;
}

#if 0
void poll_irqs(unsigned pending) {
	if (pending & PIC_DETECTED_INT) {
		irq_detected(PIC_DETECTED_INT);
		pic_regs->pending = PIC_DETECTED_INT;
	}

	if (pending & PIC_NOT_ENCRYPTED_INT) {
		irq_not_encrypted(PIC_NOT_ENCRYPTED_INT);
		pic_regs->pending = PIC_NOT_ENCRYPTED_INT;
	}
}
#endif

void wifire_handler() {
	while (fw_state != STATE_TXRX) {
		/*uint32_t pending = pic_regs->pending;
		if (pending & PIC_WIFIRE_INT) {
			//sr_wifire_ctrl->en_rcv_clk = 0;
			irq_jam(14);
			//irq_tune_wifire(14);
			//sr_wifire_ctrl->en_rcv_clk = 0;
			pic_regs->pending = PIC_WIFIRE_INT;
		}*/

		wifire_buffer_handler();

#if 1
		uint32_t pending = pic_regs->pending;		// poll for under or overrun

		if (pending & PIC_UNDERRUN_INT){
			pic_regs->pending = PIC_UNDERRUN_INT;	// clear interrupt
			putchar('U');
		}

		if (pending & PIC_OVERRUN_INT){
			sr_rx_ctrl->clear_overrun = 1; // just clear

			printf("HERE\n");

			// hardware needs time
			//mdelay(10);

			// continue receiving
			bp_clear_buf(BUF_RCV);
			bp_receive_to_buf(BUF_RCV, PORT_DSP, 1, 0, BP_LAST_LINE);

			// force rx-ctrl to start receiving
			sr_rx_ctrl->cmd = (0xE0000000 // cmd
					| BP_NLINES); // number of lines
			sr_rx_ctrl->time_secs = 0;
			sr_rx_ctrl->time_ticks = 0;

			pic_regs->pending = PIC_OVERRUN_INT;	// clear pending interrupt

			putchar('O');
		}
	}
#endif
}
