
  // receiver
  reg rcv_sfd = 0;
  reg [3:0] rcv_symbol;
  reg rcv_sym_stb = 0;
  reg rcv_running = 0;
  reg [31:0] rcv_power_level = 0;
  
  task rcv_sim_sfd;
    begin
      rcv_sfd <= 1;
      #100;
      @(posedge clk);
      rcv_sfd <= 0;
    end
  endtask
  
  task rcv_sim_symbol;
    input [3:0] symbol_i;
    begin
      @(posedge clk);
      rcv_symbol <= symbol_i;
      #300;
      @(posedge clk);
      rcv_sym_stb <= 1;
      #100;
      @(posedge clk);
      rcv_sym_stb <= 0;
    end
  endtask
  
  task rcv_sim_byte;
    input [7:0] data_i;
    begin
     rcv_sim_symbol(data_i[3:0]);
     rcv_sim_symbol(data_i[7:4]);
    end
  endtask
  
  task rcv_sim_short;
    input [15:0] data_i;
    begin
     rcv_sim_byte(data_i[7:0]);
     rcv_sim_byte(data_i[15:8]);
    end
  endtask
  
  task rcv_sim_beaconreq;
    input [7:0] seqno;
    begin
     rcv_sim_sfd();
     @(posedge clk);
     rcv_sim_byte(10); // length
     // control frame, 16 bit dest, frame version 0, no src addr
     rcv_sim_short(16'h0803);
     rcv_sim_byte(seqno);
     rcv_sim_short(16'hffff);
     rcv_sim_short(16'hffff);
     rcv_sim_byte(8'h7);
    end
  endtask
  
  task rcv_sim_address;
    input sendpan;
    input [1:0] mode;
    input [15:0] pan;
    input [63:0] addr;
    begin
      if (mode != 0) begin
        if (sendpan)
          rcv_sim_short(pan);
        rcv_sim_byte(addr[7:0]);
        rcv_sim_byte(addr[15:8]);
        if (mode == 3) begin
          rcv_sim_byte(addr[23:16]);
          rcv_sim_byte(addr[31:24]);
          rcv_sim_byte(addr[39:32]);
          rcv_sim_byte(addr[47:40]);
          rcv_sim_byte(addr[55:48]);
          rcv_sim_byte(addr[63:56]);
        end
      end
    end
  endtask
