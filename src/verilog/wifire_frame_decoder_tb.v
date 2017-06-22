`timescale 10ns / 10ns

module wifire_frame_decoder_tb;
  `include "common.v"
  `include "receiver.v"
  
  initial $dumpfile("wifire_frame_decoder_tb.vcd");
  initial $dumpvars(0,wifire_frame_decoder_tb);
  
  wire [6:0] length;
  wire [15:0] frame_ctrl;
  wire [7:0] seqno;
  wire [15:0] dst_pan, src_pan;
  wire [63:0] dst_addr, src_addr;
  wire [7:0] msdu, msdu_pos;
  wire msdu_stb;
  wire valid_len, valid_frame_ctrl, valid_seqno, valid_addr, valid_msdu;
  
  wifire_frame_decoder decoder
  (.dsp_clk(clk),.reset(reset),
   .en(1),
   .rcv_sym_i(rcv_symbol),.rcv_sym_stb_i(rcv_sym_stb),.rcv_sfd_i(rcv_sfd),
   .rcv_running_i(rcv_running),
   .len_o(length), .frame_ctrl_o(frame_ctrl), .seqno_o(seqno),
   .dst_pan_o(dst_pan), .dst_addr_o(dst_addr),
   .src_pan_o(src_pan), .src_addr_o(src_addr),
   .msdu_o(msdu), .msdu_pos_o(msdu_pos),
   .valid_len_o(valid_len), .valid_frame_ctrl_o(valid_frame_ctrl),
   .valid_seqno_o(valid_seqno), .valid_addr_o(valid_addr),
   .valid_msdu_o(valid_msdu), .msdu_stb_o(msdu_stb));
  
  wire [1:0] dst_addr_mode = frame_ctrl[11:10];
  wire [1:0] src_addr_mode = frame_ctrl[15:14];
  wire panidc = frame_ctrl[6];
  
  task address_test;
    input panidc;
    input [1:0] dst_addr_mode;
    input [15:0] dst_pan;
    input [63:0] dst_addr;
    input [1:0] src_addr_mode;
    input [15:0] src_pan;
    input [63:0] src_addr;
    begin
      rcv_sim_sfd();
      rcv_sim_byte(17);
      rcv_sim_short({src_addr_mode, 2'b00, dst_addr_mode, 3'b000, panidc, 6'd0 });
      rcv_sim_byte(7'h5a);
      rcv_sim_address(1, dst_addr_mode, dst_pan, dst_addr);
      rcv_sim_address(~panidc, src_addr_mode, src_pan, src_addr);
    end
  endtask
  
  initial begin
    @(negedge reset);
    @(posedge clk);
    
    rcv_running <= 1;
    
    // first test: test length
    #1000
    rcv_sim_sfd();
    rcv_sim_byte(17);
    
    @(posedge clk);
    if (valid_len & length == 17)
      $display("%g: Length Test: passed", $time);
    else
      $display("%g: Length Test: FAILED", $time);
    
    // second test: reset on sfd
    @(posedge clk);
    rcv_sim_sfd();
    
    @(posedge clk);
    if (~valid_len)
      $display("%g: Reset on SFD Test: passed", $time);
    else
      $display("%g: Reset on SFD Test: FAILED", $time);
    
    rcv_sim_byte(17); // recover
    
    // third test: framectrl
    rcv_sim_short(16'h0803);
    
    @(posedge clk);
    if (valid_frame_ctrl & frame_ctrl == 16'h0803)
      $display("%g: Frame Control Test: passed", $time);
    else
      $display("%g: Frame Control Test: FAILED", $time);
    
    // 4th test: seqno
    rcv_sim_byte(8'h5a);
    
    @(posedge clk);
    if (valid_seqno & seqno == 8'h5a)
      $display("%g: Sequence Number Test: passed", $time);
    else
      $display("%g: Sequence Number Test: FAILED", $time);
    
    // 5th test: no addresses at all
    address_test(0, 0, 1'dx, 1'dx, 0, 1'dx, 1'dx);
    
    @(posedge clk);
    if (valid_addr & panidc == 0 & dst_addr_mode == 0 & src_addr_mode == 0)
      $display("%g: Address Test 1 (No Addresses): passed", $time);
    else
      $display("%g: Address Test 1 (No Addresses): FAILED", $time);
    
    // 6th test: dst addr (16), no src
    address_test(0, 2, 16'hdead, 16'hbeef, 0, 1'dx, 1'dx);
    
    @(posedge clk);
    if (valid_addr & panidc == 0 & dst_addr_mode == 2 & src_addr_mode == 0
      & dst_pan == 16'hdead & dst_addr[15:0] == 16'hbeef)
      $display("%g: Address Test 2 (16 Bit Dst, No Src): passed", $time);
    else
      $display("%g: Address Test 2 (16 Bit Dst, No Src): FAILED", $time);
    
    // 7th test: dst addr (64), src (16)
    address_test(0, 3, 16'hcafe, 64'hdeadbeefcafebabe, 2, 16'hbabe, 16'hca5e);
    
    @(posedge clk);
    if (valid_addr & panidc == 0 & dst_addr_mode == 3 & src_addr_mode == 2
      & dst_pan == 16'hcafe & dst_addr == 64'hdeadbeefcafebabe
      & src_pan == 16'hbabe & src_addr[15:0] == 16'hca5e)
      $display("%g: Address Test 3 (64 Bit Dst, 16 Bit Src): passed", $time);
    else
      $display("%g: Address Test 3 (64 Bit Dst, 16 Bit Src): FAILED", $time);
    
    // 8th test: dst addr (16), src (64), panidc
    address_test(1, 2, 16'hcafe, 16'hfefe, 3, 1'dx, 64'hcafebabedeadbeef);
    
    @(posedge clk);
    if (valid_addr & panidc == 1 & dst_addr_mode == 2 & src_addr_mode == 3
      & dst_pan == 16'hcafe & dst_addr[15:0] == 16'hfefe
      & src_pan == dst_pan & src_addr == 64'hcafebabedeadbeef)
      $display("%g: Address Test 4 (16 Bit Dst, 64 Bit Src, PanIDc): passed", $time);
    else
      $display("%g: Address Test 4 (16 Bit Dst, 64 Bit Src, PanIDc): FAILED", $time);
    
    // 9th test: no dst addr, src (16)
    address_test(0, 0, 1'dx, 1'dx, 2, 16'hcafe, 16'h50fa);
    
    @(posedge clk);
    if (valid_addr & panidc == 0 & dst_addr_mode == 0 & src_addr_mode == 2
      & src_pan == 16'hcafe & src_addr[15:0] == 16'h50fa)
      $display("%g: Address Test 5 (No Dst, 16 Bit Src): passed", $time);
    else
      $display("%g: Address Test 5 (No Dst, 16 Bit Src): FAILED", $time);
    
    // 10th test: msdu, byte 1
    address_test(0, 2, 16'hffff, 16'hffff, 0, 1'dx, 1'dx);
    rcv_sim_byte(7);
    
    @(posedge msdu_stb);
    if (valid_msdu & msdu_pos == 0 & msdu == 7)
      $display("%g: MSDU Byte 1 Test: passed", $time);
    else
      $display("%g: MSDU Byte 1 Test: FAILED", $time);
    
    // 10th test: msdu, byte 2
    rcv_sim_byte(8'hde);
    @(posedge msdu_stb);
    if (valid_msdu & msdu_pos == 1 & msdu == 8'hde)
      $display("%g: MSDU Byte 2 Test: passed", $time);
    else
      $display("%g: MSDU Byte 2 Test: FAILED", $time);
    
    #3000 $finish;
  end

endmodule

