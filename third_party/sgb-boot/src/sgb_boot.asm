; qnemu Super Game Boy boot ROM (clean-room, MIT)
;
; The SGB's GB-side boot ROM does not display or verify anything: it forwards
; the cartridge header to the SNES-side SGB BIOS through the standard packet
; protocol, then unmaps itself.  qnemu HLEs the SNES side in GbSgb.
;
; Packet format (Pan Docs, SGB command packet):
;   start: 00 -> 30
;   magic (F1,F3,F5,F7,F9,FB), checksum, 14 data bytes, done: 20 -> 30
;   4 VBlanks between packets
; Header source: $0104-$014F, padded with zeroes.

DEF rJOYP EQU $FF00
DEF rIF   EQU $FF0F
DEF rLCDC EQU $FF40
DEF rBGP  EQU $FF47
DEF rNR52 EQU $FF26
DEF rNR51 EQU $FF25
DEF rNR50 EQU $FF24
DEF rIE   EQU $FFFF
DEF rBANK EQU $FF50
DEF hCommand EQU $FF80

SECTION "BootCode", ROM0[$0000]
Start:
    ld sp, $FFFE

    ; Initialize the APU to the SGB post-boot register values.
    ld a, $80
    ldh [rNR52], a
    ld a, $80
    ldh [$FF10], a
    ld a, $BF
    ldh [$FF11], a
    ld a, $F3
    ldh [$FF12], a
    ld a, $FF
    ldh [$FF13], a
    xor a
    ldh [$FF14], a
    ld a, $3F
    ldh [$FF16], a
    xor a
    ldh [$FF17], a
    ld a, $FF
    ldh [$FF18], a
    xor a
    ldh [$FF19], a
    ld a, $7F
    ldh [$FF1A], a
    ld a, $FF
    ldh [$FF1B], a
    ld a, $9F
    ldh [$FF1C], a
    ld a, $FF
    ldh [$FF1D], a
    xor a
    ldh [$FF1E], a
    ld a, $FF
    ldh [$FF20], a
    xor a
    ldh [$FF21], a
    ldh [$FF22], a
    xor a
    ldh [$FF23], a
    ld a, $77
    ldh [$FF24], a
    ld a, $F3
    ldh [$FF25], a
    ld a, $F0
    ldh [$FF26], a

    ld a, $FC
    ldh [rBGP], a
    ld a, $91
    ldh [rLCDC], a

    ld c, $00          ; JOYP
    ld a, $F1
    ldh [hCommand], a
    ld hl, $0104       ; cartridge header start

.sendCommand:
    xor a
    ldh [c], a
    ld a, $30
    ldh [c], a

    ldh a, [hCommand]
    call SendByte

    push hl
    ld b, 14
    ld d, 0
.checksumLoop:
    call ReadHeaderByte
    add d
    ld d, a
    dec b
    jr nz, .checksumLoop
    ld a, d
    call SendByte
    pop hl

    ld b, 14
.sendLoop:
    call ReadHeaderByte
    call SendByte
    dec b
    jr nz, .sendLoop

    ld a, $20
    ldh [c], a
    ld a, $30
    ldh [c], a

    ; Wait four VBlanks (IE=VBlank, IF cleared before HALT).
    ld e, 4
    ld a, 1
    ldh [rIE], a
.waitLoop:
    xor a
    ldh [rIF], a
    halt
    nop
    dec e
    jr nz, .waitLoop
    xor a
    ldh [rIE], a

    ldh a, [hCommand]
    add 2
    ldh [hCommand], a
    ld a, $58
    cp l
    jr nz, .sendCommand

    ; Final SGB register set.
    ld b, $00
    ld c, $14
    ld d, $00
    ld e, $00
    ld hl, $C060
    ld a, $01
    or a               ; A=1, F=0
    jp BootGame

ReadHeaderByte:
    ld a, $4F
    cp l
    jr c, .zero
    ld a, [hli]
    ret
.zero:
    inc hl
    xor a
    ret

SendByte:
    ld e, a
    ld d, 8
.loop:
    ld a, $10
    rr e
    jr c, .zeroBit
    add a              ; $20 for a one bit
.zeroBit:
    ldh [c], a
    ld a, $30
    ldh [c], a
    dec d
    ret z
    jr .loop

IF @ > $00FE
    FAIL "SGB boot ROM overflow"
ENDC
    ds $00FE - @, 0

SECTION "BootGame", ROM0[$00FE]
BootGame:
    ldh [rBANK], a
