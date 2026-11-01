; GBC infrared smoke test.
;
; Build two ROMs from this source:
;   rgbasm -o ir_sender.o ir_test.asm
;   rgbasm -DRECEIVER=1 -o ir_receiver.o ir_test.asm
; The sender pulses its IR LED and waits for the receiver's acknowledgement;
; the receiver waits for the sender, pulses back, and both write 1 to SRAM.
;
; Build:
;   make

DEF rRP EQU $FF56

SECTION "Header", ROM0[$100]
    nop
    jp Entry
    ds $134 - @, 0
    db "IRTEST"
    ds $143 - @, 0
    db $80                  ; CGB-compatible, native CGB mode
    ds $14D - @, 0
    ds $150 - @, 0

SECTION "Main", ROM0[$150]
Entry:
    di
    ld a, $0A
    ld [$0000], a
    xor a
    ld [$A000], a

    ; Read enable, local LED off.
    ld a, $C0
    ldh [rRP], a

IF DEF(RECEIVER)
.wait:
    ldh a, [rRP]
    bit 1, a
    jr nz, .wait

    ; Acknowledge with a 100 ms pulse train.
    ld d, 50
.ack:
    ld a, $C1
    ldh [rRP], a
    call Delay1ms
    ld a, $C0
    ldh [rRP], a
    call Delay1ms
    dec d
    jr nz, .ack

    ld a, 1
    ld [$A000], a
.done:
    jr .done
ELSE
    ; Pulse the LED and poll for the receiver during each off phase.
    ld d, 100
.loop:
    ld a, $C1
    ldh [rRP], a
    call Delay1ms
    ld a, $C0
    ldh [rRP], a

    ld e, 200
.poll:
    ldh a, [rRP]
    bit 1, a
    jr z, .seen
    dec e
    jr nz, .poll

    dec d
    jr nz, .loop
    jr .done

.seen:
    ld a, 1
    ld [$A000], a
.done:
    jr .done
ENDC

Delay1ms:
    ld bc, 150
.loop:
    dec bc
    ld a, b
    or c
    jr nz, .loop
    ret
