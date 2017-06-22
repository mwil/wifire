  // clock and reset
  reg clk = 0;
  reg reset = 1;
  
  initial #1000 reset = 0;
  always #50 clk = ~clk;
  
  // settings bus
  reg         set_stb = 0;
  reg [7:0]   set_addr;
  reg [31:0]  set_data;
  
  task write_setting;
    input [7:0] addr;
    input [31:0] data;
    begin
      set_stb <= 0;
      @(posedge clk);
      set_addr <= addr;
      set_data <= data;
      set_stb  <= 1;
      @(posedge clk);
      set_stb <= 0;
    end
  endtask
  
  // wishbone
  reg wb_clk = 0;
  wire wb_rst = reset;
  reg wb_we = 0;
  reg wb_stb = 0;
  wire wb_ack;
  reg [15:0] wb_adr;
  reg [31:0] wb_dat_o;
  wire [31:0] wb_dat_i;
  
  always @(posedge clk)
    wb_clk <= ~wb_clk;
  
  task readmem;
    input [13:0] addr;
    output [31:0] out;
    begin
     @(posedge wb_clk);
     wb_adr <= {addr,2'd0};
     wb_stb <= 1;
     @(posedge wb_ack);
     out = wb_dat_i;
     wb_stb <= 0;
    end
  endtask

