`timescale 10ns / 10ns

module wifire_tb;
  `include "common.v"
  `include "receiver.v"
  
  initial $dumpfile("wifire_tb.vcd");
  initial $dumpvars(0,wifire_tb);
  
  wire irq_header, irq_msdu;
  
  wifire #(.BASE(16)) wifire
  (.dsp_clk(clk),.reset(reset),
   .set_stb(set_stb),.set_addr(set_addr),.set_data(set_data),
   .wb_clk_i(wb_clk),.wb_rst_i(wb_rst),.wb_we_i(wb_we),.wb_stb_i(wb_stb),
   .wb_adr_i(wb_adr),.wb_dat_i(wb_dat_o),.wb_dat_o(wb_dat_i),.wb_ack_o(wb_ack),
   .rcv_sym_i(rcv_symbol),.rcv_sym_stb_i(rcv_sym_stb),.rcv_sfd_i(rcv_sfd),
   .rcv_running_i(rcv_running), .rcv_power_level_i(rcv_power_level),
   .irq_header_o(irq_header), .irq_msdu_o(irq_msdu));
  
  task wifire_enable;
    begin
      write_setting(17, 1);
    end
  endtask
  
  reg [31:0] data;
  
  initial begin
    @(negedge reset);
    @(posedge clk);
    
    wifire_enable();
    rcv_running <= 1;
    rcv_power_level <= 32'hdeadbeef;
    
    // first test: test wishbone output
    rcv_sim_beaconreq(8'h5a);
    
    readmem(0, data);
    if (data[7:0] == 10)
      $display("%g: Wishbone Test 1 (Length): passed", $time);
    else
      $display("%g: Wishbone Test 1 (Length): FAILED", $time);
    
    readmem(1, data);
    if (data[15:0] == 16'h0803)
      $display("%g: Wishbone Test 2 (Frame Control): passed", $time);
    else
      $display("%g: Wishbone Test 2 (Frame Control): FAILED", $time);
    
    readmem(2, data);
    if (data[7:0] == 8'h5a)
      $display("%g: Wishbone Test 3 (Sequence Number): passed", $time);
    else
      $display("%g: Wishbone Test 3 (Sequence Number): FAILED", $time);
    
    readmem(3, data);
    if (data[15:0] == 16'hffff)
      $display("%g: Wishbone Test 4 (Dst Pan): passed", $time);
    else
      $display("%g: Wishbone Test 4 (Dst Pan): FAILED", $time);
    
    readmem(5, data);
    if (data[15:0] == 16'hffff)
      $display("%g: Wishbone Test 5 (Dst Addr): passed", $time);
    else
      $display("%g: Wishbone Test 5 (Dst Addr): FAILED", $time);
    
    readmem(14, data);
    if (data == 32'hdeadbeef)
      $display("%g: Wishbone Test 6 (Powerlevel): passed", $time);
    else
      $display("%g: Wishbone Test 6 (Powerlevel): FAILED", $time);
    
    readmem(1<<5 + 0, data);
    if (data[7:0] == { 8'd7 })
      $display("%g: Wishbone Test 7 (MSDU 0): passed", $time);
    else
      $display("%g: Wishbone Test 7 (MSDU 0): FAILED", $time);
    
    rcv_sim_byte(8'h11);
    rcv_sim_short(16'hdead);
    
    readmem(1<<5 + 0, data);
    if (data == { 16'hdead, 8'h11, 8'd7 })
      $display("%g: Wishbone Test 8 (MSDU 0-3): passed", $time);
    else
      $display("%g: Wishbone Test 8 (MSDU 0-3): FAILED", $time);
    
    #3000
    
    // clear wifire
    write_setting(16, 1);
    @(posedge clk);
    
    // write header
    rcv_sim_sfd();
    rcv_sim_byte(30); // length
    rcv_sim_short({2'd2, 2'b00, 2'd2, 3'b000, 1'd1, 3'd0, 3'd1 });
    rcv_sim_byte(7'h5a);
    rcv_sim_address(1, 2, 16'h0022, 16'h5a5a);
    rcv_sim_address(0, 2, 16'h0022, 16'hdead);
    
    @(posedge clk);
    @(posedge clk);
    // check for irq
    if (irq_header)
      $display("%g: Header IRQ: passed", $time);
    else
      $display("%g: Header IRQ: FAILED", $time);
    
    write_setting(18, 1);
    
    // write data
    rcv_sim_byte(10);
    @(posedge clk);
    @(posedge clk);
    // check for irq
    if (irq_msdu)
      $display("%g: MSDU IRQ 1: passed", $time);
    else
      $display("%g: MSDU IRQ 1: FAILED", $time);
    
    write_setting(18, 1);
    write_setting(19, 1);
    @(posedge clk);
    rcv_sim_byte(8'ha3);
    rcv_sim_byte(8'ha1);
    rcv_sim_byte(8'h11);
    
    @(posedge clk);
    @(posedge clk);
    // check for irq
    if (irq_msdu)
      $display("%g: MSDU IRQ 2: passed", $time);
    else
      $display("%g: MSDU IRQ 2: FAILED", $time);
    
    rcv_sim_sfd();
    @(posedge clk);
    @(posedge clk);
    if (~irq_msdu)
      $display("%g: MSDU IRQ 3: passed", $time);
    else
      $display("%g: MSDU IRQ 3: FAILED", $time);
    
    #3000 $finish;
   end
   
endmodule
