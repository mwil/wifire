
module wifire_frame_decoder
   (input dsp_clk, input reset,
    input en,
    
    // Receiver Input
    input [3:0] rcv_sym_i,
    input rcv_sym_stb_i,
    input rcv_sfd_i,
    input rcv_running_i,
    
    // Frame Output
    output reg [6:0] len_o,
    output reg [15:0] frame_ctrl_o,
    output reg [7:0] seqno_o,
    output reg [15:0] dst_pan_o,
    output reg [63:0] dst_addr_o,
    output reg [15:0] src_pan_o,
    output reg [63:0] src_addr_o,
    output [7:0] msdu_o,
    output reg [7:0] msdu_pos_o,
    
    /* validity */
    output valid_len_o,
    output valid_frame_ctrl_o,
    output valid_seqno_o,
    output valid_addr_o,
    output valid_msdu_o,
    
    output reg msdu_stb_o
   );
   
   // translate symbol strobe to clock
   reg sym_stb_prev;
   wire int_sym_stb;
   
   always @(posedge dsp_clk)
     sym_stb_prev <= rcv_sym_stb_i;
   
   assign int_sym_stb = ~sym_stb_prev & rcv_sym_stb_i;
   
   // frame state
   localparam STATE_WAIT = 0;
   localparam STATE_LEN = 1;
   localparam STATE_CTRL_0 = 2;
   localparam STATE_CTRL_1 = 3;
   localparam STATE_SEQNO = 4;
   localparam STATE_ADDR = 5;
   localparam STATE_MSDU = 6;
   localparam STATE_IDLE = 7;
   
   // state
   reg [2:0] state;
   reg [4:0] addr_state;
   
   wire [1:0] dst_addr_mode = frame_ctrl_o[11:10];
   wire [1:0] src_addr_mode = frame_ctrl_o[15:14];
   wire panidc = frame_ctrl_o[6];
   
   // rebuild bytes from symbols
   reg sym_state;
   reg [3:0] tmp_symbol;
   wire [7:0] cur_data = { rcv_sym_i, tmp_symbol };
   
   // validity
   assign valid_len_o = state > STATE_LEN;
   assign valid_frame_ctrl_o = state > STATE_CTRL_1;
   assign valid_seqno_o = state > STATE_SEQNO;
   assign valid_addr_o = state > STATE_ADDR;
   assign valid_msdu_o = state == STATE_MSDU;
   
   // msdu
   assign msdu_o = cur_data;
   
   // frame decoder
   always @(posedge dsp_clk)
   if (reset) begin
     state <= STATE_WAIT;
     sym_state <= 0;
     msdu_stb_o <= 0;
   end else if (en) begin
     if (rcv_sfd_i) begin
       state <= STATE_LEN;
       sym_state <= 0;
       msdu_stb_o <= 0;
     end else if (rcv_running_i & int_sym_stb) begin
       if (~sym_state) begin
         tmp_symbol <= rcv_sym_i;
         msdu_stb_o <= 0;
       end else begin
         case (state)
         STATE_LEN: begin
           len_o <= cur_data[6:0];
           state <= STATE_CTRL_0;
           end
         STATE_CTRL_0: begin
           frame_ctrl_o[7:0] <= cur_data;
           state <= STATE_CTRL_1;
           end
         STATE_CTRL_1: begin
           frame_ctrl_o[15:8] <= cur_data;
           state <= STATE_SEQNO;
           end
         STATE_SEQNO: begin
           seqno_o <= cur_data;
           state <= STATE_ADDR;
           if (dst_addr_mode != 0)
             addr_state <= 0;
           else if(src_addr_mode != 0)
             addr_state <= 10;
           else begin
             state <= STATE_MSDU;
             msdu_pos_o <= ~0;
           end
           end
         STATE_ADDR: begin
           case(addr_state)
           0: dst_pan_o[7:0] <= cur_data;
           1: dst_pan_o[15:8] <= cur_data;
           2: dst_addr_o[7:0] <= cur_data;
           3: dst_addr_o[15:8] <= cur_data;
           4: dst_addr_o[23:16] <= cur_data;
           5: dst_addr_o[31:24] <= cur_data;
           6: dst_addr_o[39:32] <= cur_data;
           7: dst_addr_o[47:40] <= cur_data;
           8: dst_addr_o[55:48] <= cur_data;
           9: dst_addr_o[63:56] <= cur_data;
           10: src_pan_o[7:0] <= cur_data;
           11: src_pan_o[15:8] <= cur_data;
           12: src_addr_o[7:0] <= cur_data;
           13: src_addr_o[15:8] <= cur_data;
           14: src_addr_o[23:16] <= cur_data;
           15: src_addr_o[31:24] <= cur_data;
           16: src_addr_o[39:32] <= cur_data;
           17: src_addr_o[47:40] <= cur_data;
           18: src_addr_o[55:48] <= cur_data;
           19: src_addr_o[63:56] <= cur_data;
           endcase
           if (addr_state == 3 & dst_addr_mode == 2) // end of short addr
             if (src_addr_mode == 0) begin // no src addr => done
               state <= STATE_MSDU;
               msdu_pos_o <= ~0;
             end else if (panidc) begin // src pan = dst pan, read src addr
               src_pan_o <= dst_pan_o;
               addr_state <= 12;
             end else // read src pan
               addr_state <= 10;
           else if ((addr_state == 13 & src_addr_mode == 2) | addr_state == 19) begin
             state <= STATE_MSDU;
             msdu_pos_o <= ~0;
           end else
             addr_state <= addr_state + 1;
         end
         STATE_MSDU: begin
           msdu_pos_o <= msdu_pos_o + 1;
           msdu_stb_o <= 1;
         end
         endcase
       end
       sym_state <= ~sym_state;
     end else
       msdu_stb_o <= 0;
     end
endmodule

