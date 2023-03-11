`define STRINGIFY_BIOS(x) `"x/../software/bios/bios.hex`"

module z1top #(
    parameter BAUD_RATE = 115_200,
    parameter CPU_CLOCK_PERIOD = 20,
    parameter BIOS_MIF_HEX = `STRINGIFY_BIOS(`ABS_TOP)
) (
    input CLK_125MHZ_FPGA,
    input [3:0] BUTTONS,
    input [1:0] SWITCHES,
    output [5:0] LEDS,
    input  FPGA_SERIAL_RX,
    output FPGA_SERIAL_TX
);

    localparam CPU_CLOCK_FREQ = 1_000_000_000 / CPU_CLOCK_PERIOD;
    localparam RESET_PC = 32'h4000_0000;
    /* verilator lint_off REALCVT */
    localparam integer B_SAMPLE_CNT_MAX = 0.0005 * CPU_CLOCK_FREQ;
    localparam integer B_PULSE_CNT_MAX = 0.100 / 0.0005;
    /* lint_on */

    // Clocks
    wire cpu_clk, cpu_clk_locked;

    // Buttons after the button_parser
    wire [3:0] buttons_pressed;

    // Switches after the synchronizer
    wire [1:0] switches_sync;

    // Reset the CPU and all components on the cpu_clk if the reset button is
    // pushed or whenever the CPU clock PLL isn't locked
    wire cpu_reset;
    assign cpu_reset = buttons_pressed[0] || !cpu_clk_locked;

    // Use IOBs to drive/sense the UART serial lines
    wire cpu_tx, cpu_rx;
    (* IOB = "true" *) reg fpga_serial_tx_iob;
    (* IOB = "true" *) reg fpga_serial_rx_iob;
    assign FPGA_SERIAL_TX = fpga_serial_tx_iob;
    assign cpu_rx = fpga_serial_rx_iob;
    always @(posedge cpu_clk) begin
        fpga_serial_tx_iob <= cpu_tx;
        fpga_serial_rx_iob <= FPGA_SERIAL_RX;
    end

    // Clocking wizard IP from Vivado (wrapper of the PLLE module)
    // Generate CPU_CLOCK_FREQ clock from 125 MHz clock
    // PLL FREQ = (CLKFBOUT_MULT_F * 1000 / (CLKINx_PERIOD * DIVCLK_DIVIDE) must be within (800.000 MHz - 1600.000 MHz)
    // CLKOUTx_PERIOD = CLKINx_PERIOD x DIVCLK_DIVIDE x CLKOUT0_DIVIDE / CLKFBOUT_MULT_F
    clk_wiz #(
        .CLKIN1_PERIOD(8),
        .CLKFBOUT_MULT_F(8),
        .DIVCLK_DIVIDE(1),
        .CLKOUT0_DIVIDE(CPU_CLOCK_PERIOD)
    ) clk_wiz (
        .clk_out1(cpu_clk),       // output
        .reset(1'b0),             // input
        .locked(cpu_clk_locked),  // output
        .clk_in1(CLK_125MHZ_FPGA) // input
    );

    button_parser #(
        .WIDTH(4),
        .SAMPLE_CNT_MAX(B_SAMPLE_CNT_MAX),
        .PULSE_CNT_MAX(B_PULSE_CNT_MAX)
    ) bp (
        .clk(cpu_clk),
        .in(BUTTONS),
        .out(buttons_pressed)
    );

    synchronizer #(
        .WIDTH(2)
    ) switch_synchronizer (
        .clk(cpu_clk),
        .async_signal(SWITCHES),
        .sync_signal(switches_sync)
    );

    cpu #(
        .CPU_CLOCK_FREQ(CPU_CLOCK_FREQ),
        .RESET_PC(RESET_PC),
        .BAUD_RATE(BAUD_RATE),
        .BIOS_MIF_HEX(BIOS_MIF_HEX)
    ) cpu (
        .clk(cpu_clk),
        .rst(cpu_reset),
        .bp_enable(switches_sync[0]),
        .serial_out(cpu_tx),
        .serial_in(cpu_rx)
    );


endmodule
