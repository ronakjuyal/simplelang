COMPUTER    = $(wildcard rtl/*.v)
LIBRARIES   = $(wildcard rtl/library/*.v)

build:
	iverilog -o computer -Wall \
		$(COMPUTER) \
		$(LIBRARIES) \
		rtl/tb/machine_tb.v

run: build
	vvp -n computer

clean:
ifeq ($(OS),Windows_NT)
	del /s /q computer 2>nul || exit 0
else
	rm -rf computer
endif

view:
	gtkwave machine.vcd gtkwave/config.gtkw

tests:
	bats tests/tests.bats

.PHONY: build run clean view tests
