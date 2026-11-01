ROM_SENDER := ir_sender.gb
ROM_RECEIVER := ir_receiver.gb

all: $(ROM_SENDER) $(ROM_RECEIVER)

$(ROM_SENDER): ir_test.asm
	rgbasm -o ir_sender.o ir_test.asm
	rgblink -o $@ ir_sender.o
	rgbfix -v -p 0 -m mbc1+ram+battery -r 2 $@

$(ROM_RECEIVER): ir_test.asm
	rgbasm -DRECEIVER=1 -o ir_receiver.o ir_test.asm
	rgblink -o $@ ir_receiver.o
	rgbfix -v -p 0 -m mbc1+ram+battery -r 2 $@

clean:
	rm -f ir_sender.o ir_receiver.o

.PHONY: all clean
