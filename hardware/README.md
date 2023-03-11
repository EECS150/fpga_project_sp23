# Testing Framework

## Simulation
### iverilog
```bash
make sim/cpu_tb.fst
# logfile
cat sim/cpu_tb.log
# waveform
gtkwave sim/cpu_tb.fst &
```

### VCS
```bash
make sim/cpu_tb.vpd
# logfile
cat sim/cpu_tb.log
# waveform
dve -vpd sim/cpu_tb.vpd &
```

###  Clean
- Clean simulation outputs: `make clean-sim`
- Forcefully re-run a testbench: `make -B sim/cpu_tb.fst`

## CAD Flow
- Lint RTL with verilator: `make lint`
- Open Vivado GUI: `make vivado`
- Elaborate and open circuit in GUI: `make elaborate`
- Synthesis: `make synth`
    - Log file: `build/synth/synth.log`
- Place, Route, Bitstream Generation (Implementation): `make impl`
    - Log file: `build/impl/impl.log`
- Program FPGA: `make program` or `make remote` if you're using programming via hardware server
- Force program FPGA (don't rebuild bitstream if RTL changed): `make program-force` or `make remote-force`
- Clean CAD flow outputs: `make clean-build`
