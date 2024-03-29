`timescale 10ns/1ns
module Keyboard
(
    input clk,
    input clrn,
	input ps2_clk,
	input ps2_data,
	input en,
	output reg [31:0] cur_key
	);

// add your definitions here
    wire [7:0] keydata;
    wire ready;
    wire overflow;
    wire reset;
    reg nextdata_n = 0;
    reg release_f = 0;
    reg tri_f = 0;

    assign reset = clrn == 0 ? 1 : overflow;

//PS2 interface
ps2_keyboard mykey(clk, ~reset, ps2_clk, ps2_data, keydata, ready, nextdata_n, overflow);

// add you code here
always@(posedge clk) begin
    if(clrn == 0) begin
        cur_key <= 32'h00000000;
    end
    else begin
        if(en == 0) begin
            cur_key <= cur_key;
            //cur_key <= 32'h0000001c;
            nextdata_n <= 1;
        end
        else if(nextdata_n == 0) nextdata_n <= 1;
        else if(ready == 1) begin
            nextdata_n <= 0;
            
            if(cur_key[7:0] == keydata) begin //keep
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
            cur_key <= 0;
        end
    end
end

endmodule
