`timescale 1ns/1ns
`include "mem_path.vh"

// This testbench is used to estimate the CPI using a smaller version of benchmark.
// Test ends when the CSR is set to a non-zero value, which is also used as a reference checksum.

module small_tb();
  reg clk, rst;
  parameter CPU_CLOCK_PERIOD = 20;
  parameter CPU_CLOCK_FREQ   = 1_000_000_000 / CPU_CLOCK_PERIOD;
  localparam BAUD_RATE       = 10_000_000;
  localparam BAUD_PERIOD     = 1_000_000_000 / BAUD_RATE; // 8680.55 ns

  localparam TIMEOUT_CYCLE = 100_000;

  initial clk = 0;
  always #(CPU_CLOCK_PERIOD/2) clk = ~clk;

  reg bp_enable = 1'b0;
  wire serial_out;

  cpu # (
    .CPU_CLOCK_FREQ(CPU_CLOCK_FREQ),
    .RESET_PC(32'h1000_0000),
    .BAUD_RATE(BAUD_RATE)
  ) cpu (
    .clk(clk),
    .rst(rst),
    .bp_enable(bp_enable),
    .serial_in(1'b1),
    .serial_out(serial_out)
  );

  reg [31:0] cycle;
  always @(posedge clk) begin
    if (rst === 1)
      cycle <= 0;
    else
      cycle <= cycle + 1;
  end

  // Host off-chip UART <-- FPGA on-chip UART (transmitter)
  // Display the received characters and store the result
  integer j;
  reg [9:0] char_out;
  reg [8*16-1:0] result;
  initial begin
    forever begin
      // Wait until serial_out is LOW (start of transaction)
      while (serial_out === 1) begin
        @(posedge clk);
      end
      for (j = 0; j < 10; j = j + 1) begin
        char_out[j] = serial_out;
        #(BAUD_PERIOD);
      end
      if ((char_out[8:1] >= 8'd32 && char_out[8:1] <= 8'd126) || char_out[8:1] === 8'd10 || char_out[8:1] === 8'd13)
        $write("%c", char_out[8:1]);
      if(result[8*16-1:8*8] !== "Result: ")
        result = {result, char_out[8:1]};
    end
  end

  reg [255:0] MIF_FILE;
  string hex_file, test_name;
  reg [8*8-1:0] expected;
  initial begin
    if (!$value$plusargs("hex_file=%s", hex_file)) begin
      $display("Must supply hex_file!");
      $fatal();
    end

    if (!$value$plusargs("test_name=%s", test_name)) begin
      $display("Must supply test_name!");
      $fatal();
    end

    `ifndef IVERILOG
      $vcdpluson;
    `endif
    `ifdef IVERILOG
      $dumpfile({test_name, ".fst"});
      $dumpvars(0, small_tb);
    `endif

    #1;
    $readmemh(hex_file, `IMEM_PATH.mem, 0, 16384-1);
    $readmemh(hex_file, `DMEM_PATH.mem, 0, 16384-1);

    rst = 1;

    // Hold reset for a while
    repeat (10) @(posedge clk);

    @(negedge clk);
    rst = 0;

    // Delay for some time
    repeat (10) @(posedge clk);

    // Wait until csr is updated
    while (`CSR_PATH === 0)
      @(posedge clk);

    $sformat(expected, "%h", `CSR_PATH);
     
    repeat (1000) @(posedge clk);
    if (expected === result[8*8-1:0]) begin
      $display("[%d sim. cycles] %s PASSED!", cycle, test_name);
    end else begin
      $display("[%d sim. cycles] %s FAILED! got %s, expected %s", 
               cycle, test_name, result[8*8-1:0], expected);
    end

    repeat (100) @(posedge clk);
    $finish();
  end

  initial begin
    repeat (TIMEOUT_CYCLE) @(posedge clk);
    $display("Timeout!");
    $finish();
  end

endmodule
