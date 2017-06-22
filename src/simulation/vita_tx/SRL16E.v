
module SRL16E(Q, A0, A1, A2, A3, CE, CLK, D);
	parameter INIT=16'h0000;
	input  A0, A1, A2, A3, CE, CLK, D;
	output Q;
	reg [15:0] data;
	assign Q=data[{A3, A2, A1, A0}];
	initial
		data=INIT;
	always @(posedge CLK)
		if (CE == 1'b1) data[15:0] <= {data[14:0], D};
endmodule

