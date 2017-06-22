
module wifire
  #(parameter BASE=0)
   (input dsp_clk, input reset,
    input set_stb, input [7:0] set_addr, input [31:0] set_data,
    
    // Receiver Interface
    input [3:0] rcv_sym_i,
    input rcv_sym_stb_i,
    input rcv_sfd_i,
    input rcv_running_i,
    input [31:0] rcv_power_level_i,
    
    // wishbone output
    input wb_clk_i,
    input wb_rst_i,
    input wb_we_i,
    input wb_stb_i,
    input [15:0] wb_adr_i,
    input [31:0] wb_dat_i,
    output [31:0] wb_dat_o,
    output reg wb_ack_o,
    output wb_err_o,
    output wb_rty_o,
    
    // pic output
    output reg irq_header_o,
    output reg irq_msdu_o
   );
   
   assign wb_err_o = 1'b0;  // Unused for now
   assign wb_rty_o = 1'b0;  // Unused for now
   
   // settings /////////////////////////////////////////////////////////////////
   wire clear_header_intr, clear_msdu_intr;
   
   // settings: clear frame intr
   setting_reg #(.my_addr(BASE)) wifire_clear_header_intr
     (.clk(dsp_clk),.rst(reset),.strobe(set_stb),.addr(set_addr),.in(set_data),
      .changed(clear_header_intr));
   
   // settings: clear msdu intr
   setting_reg #(.my_addr(BASE+1)) wifire_clear_msdu_intr
     (.clk(dsp_clk),.rst(reset),.strobe(set_stb),.addr(set_addr),.in(set_data),
      .changed(clear_msdu_intr));
   
   // frame decoder ////////////////////////////////////////////////////////////
   wire [6:0] length;
   wire [15:0] frame_ctrl;
   wire [7:0] seqno;
   wire [15:0] dst_pan, src_pan;
   wire [63:0] dst_addr, src_addr;
   wire valid_len, valid_frame_ctrl, valid_seqno, valid_addr, valid_msdu;
   wire [7:0] msdu, msdu_pos;
   wire msdu_stb;
   
   wifire_frame_decoder decoder
   (.dsp_clk(dsp_clk),.reset(reset), .en(1),
    .rcv_sym_i(rcv_sym_i),.rcv_sym_stb_i(rcv_sym_stb_i),.rcv_sfd_i(rcv_sfd_i),
    .rcv_running_i(rcv_running_i),
    .len_o(length), .frame_ctrl_o(frame_ctrl), .seqno_o(seqno),
    .dst_pan_o(dst_pan), .dst_addr_o(dst_addr),
    .src_pan_o(src_pan), .src_addr_o(src_addr),
    .valid_len_o(valid_len), .valid_frame_ctrl_o(valid_frame_ctrl),
    .valid_seqno_o(valid_seqno), .valid_addr_o(valid_addr),
    .valid_msdu_o(valid_msdu), .msdu_o(msdu), .msdu_pos_o(msdu_pos),
    .msdu_stb_o(msdu_stb));
   
   
   // MSDU Ram /////////////////////////////////////////////////////////////////
   wire [31:0] msdu_wb;
   xlnx_bram_128_8_to_32_32 msdu_ram
   (.clka(dsp_clk),.ena(valid_msdu),.wea(msdu_stb),.addra(msdu_pos[6:0]),.dina(msdu),
    .clkb(dsp_clk),.addrb(wb_adr_i[6:2]),.doutb(msdu_wb));
   
   // behaviour ////////////////////////////////////////////////////////////////
   // store power level at sfd
   /*always @(posedge rcv_sfd_i)
     power_sfd <= rcv_power_level_i;*/
   
   reg valid_addr_prev;
   wire header_ready_stb = ~valid_addr_prev & valid_addr;
   
   always @(posedge dsp_clk)
     valid_addr_prev <= valid_addr;
   
	reg [63:0] time_tics;
	reg [63:0] irq_header_tics;
	
	// behaviour
   always @(posedge dsp_clk)
     if (reset | rcv_sfd_i) begin
       irq_header_o <= 0;
       irq_msdu_o <= 0;
     end else begin
       if (header_ready_stb) begin
         irq_header_o <= 1;
			irq_header_tics <= time_tics;
       end else if (clear_header_intr)
         irq_header_o <= 0;
       
       if (msdu_stb)
         irq_msdu_o <= 1;
       else if (clear_msdu_intr)
         irq_msdu_o <= 0;
     end
   
   always @(posedge dsp_clk)
     if (reset)
       time_tics <= 0;
     else
       time_tics <= time_tics + 1;
   
   reg [63:0] sfd_tics;
   always @(posedge dsp_clk)
	  if (rcv_sfd_i)
       sfd_tics <= time_tics;
   
   // wishbone output //////////////////////////////////////////////////////////
   
	reg [31:0] frame_wb;
   
   assign wb_dat_o = (wb_adr_i[7] ? msdu_wb : frame_wb);
	
   always @(posedge wb_clk_i)
     if (!wb_rst_i & wb_stb_i & ~wb_ack_o) begin
       case (wb_adr_i[6:2])
       0: frame_wb <= { 25'd0, length };
       1: frame_wb <= { 16'd0, frame_ctrl };
       2: frame_wb <= { 24'd0, seqno };
       3: frame_wb <= { 16'd0, dst_pan };
       4: frame_wb <= dst_addr[63:32];
       5: frame_wb <= dst_addr[31:0];
       6: frame_wb <= { 16'd0, src_pan };
       7: frame_wb <= src_addr[63:32];
       8: frame_wb <= src_addr[31:0];
       9: frame_wb <= sfd_tics[63:32];
       10: frame_wb <= sfd_tics[31:0];
       11: frame_wb <= {24'd0, msdu_pos };
       12: frame_wb <= time_tics[63:32];
       13: frame_wb <= time_tics[31:0];
       14: frame_wb <= irq_header_tics[63:32];
		 15: frame_wb <= irq_header_tics[31:0];
       default:
         frame_wb <= 32'd0;
       endcase
       wb_ack_o <= 1;
     end else wb_ack_o <= 0;
   

endmodule

