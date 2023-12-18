module keyboard
(
    input clk,
	input clrn,
	input ps2_clk,
	input ps2_data,
	output reg [31:0] cur_key
//	output [7:0] ascii_key,
//  output reg [6:0] h,
//	output reg [7:0] AN,
//	output dp,
//	output shift,
//	output ctrl,
//	output caps
	);

// add your definitions here
    wire [7:0] keydata;
    wire ready;
    wire overflow;
    reg nextdata_n = 0;
    reg release_f = 0;
    reg tri_f = 0;
    
    reg clk_1000HZ;

    

//----DO NOT CHANGE BEGIN----
//scancode to ascii conversion, will be initialized by the testbench
//scancode_ram myram(clk, cur_key, ascii_key, shift_state, caps_state, ctrl_state);
//PS2 interface, you may need to specify the inputs and outputs
ps2_keyboard mykey(clk, clrn, ps2_clk, ps2_data, keydata, ready, nextdata_n, overflow);
//bcd7seg
//bcd7seg dut1(.b(cur_key[3:0]), .h(h1));
//bcd7seg dut2(.b(cur_key[7:4]), .h(h2));
//bcd7seg dut3(.b(ascii_key[3:0]), .h(h3));
//bcd7seg dut4(.b(ascii_key[7:4]), .h(h4));
//bcd7seg dut5(.b(key_count[3:0]), .h(h5));
//bcd7seg dut6(.b(key_count[7:4]), .h(h6));
//    always @(posedge clk_1000HZ) begin
//        if(select == 4'b0101)
//            select <= 0;
//        else
//            select <= select+1;
//        case(select)
//            4'b0000: begin if(null) AN = 8'b11111111; else AN = 8'b11111110; h = h1; end
//            4'b0001: begin if(null) AN = 8'b11111111; else AN = 8'b11111101; h = h2; end
//            4'b0010: begin if(null) AN = 8'b11111111; else AN = 8'b11111011; h = h3; end
//            4'b0011: begin if(null) AN = 8'b11111111; else AN = 8'b11110111; h = h4; end
//            4'b0100: begin AN = 8'b11101111; h = h5; end
//            4'b0101: begin AN = 8'b11011111; h = h6; end
//        endcase
//        end
//---DO NOT CHANGE END-----

// add you code here
always@(posedge clk) begin
    if(clrn == 0) begin
        cur_key <= 0;
    end
    else begin
        if(nextdata_n == 0) nextdata_n <= 1;
        if(ready == 1) begin
            nextdata_n <= 0;
            
            if(cur_key == keydata) begin //keep
                cur_key <= cur_key;
            end
            
            else if(keydata == 8'he0) begin //triple
                tri_f <= 1;
                cur_key <= 0;
            end
                        
            else if(keydata == 8'hf0) begin //release
                release_f <= 1;
                cur_key <= 0;
            end
            
            else if(release_f == 1) begin
                    release_f <= 0;
                    if(tri_f == 1) begin
                        cur_key <= {24'h00E0F0, keydata};
                        tri_f <= 0;
                    end
                    else begin
                        cur_key <= {24'h0000F0, keydata};
                    end
            end
            
            else begin
                    if(tri_f == 1) begin
                        cur_key <= {24'h0000E0, keydata};
                        tri_f <= 0;
                    end
                    else begin
                        cur_key <= {24'h000000, keydata};
                    end
            end
            
        end
        else begin
            nextdata_n <= 0;
        end
    end
end

    //自定义时钟
    reg [31:0] count_clk = 0;
    always @(posedge clk)
    if(count_clk==49999)
    begin
        count_clk <=0;
        clk_1000HZ <= ~clk_1000HZ;
    end
    else
        count_clk <= count_clk+1;

endmodule
