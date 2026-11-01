ROM := four_player_test.gb

all: $(ROM)

$(ROM): four_player_test.asm
	rgbasm -o four_player_test.o four_player_test.asm
	rgblink -o $@ four_player_test.o
	rgbfix -v -p 0 -m mbc1+ram+battery -r 2 -t LINK4 $@

clean:
	rm -f four_player_test.o

.PHONY: all clean
