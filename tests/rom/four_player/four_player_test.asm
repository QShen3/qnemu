; Four-player DMG-07 protocol smoke test.
;
; Four copies of this ROM run with a DMG-07 adapter.  Each player pings,
; requests the transmission phase, sends its player id, and waits for a packet
; containing 1,2,3,4.  On success it writes 1 to SRAM at $A000.
;
; Build:
;   rgbasm -o four_player_test.o four_player_test.asm
;   rgblink -o four_player_test.gb four_player_test.o
;   rgbfix -v -p 0 -m mbc1+ram+battery -r 2 -t LINK4 four_player_test.gb

DEF rSB EQU $FF01
DEF rSC EQU $FF02
DEF rIF EQU $FF0F
DEF rIE EQU $FFFF

SECTION "SerialVector", ROM0[$58]
    jp SerialISR

SECTION "Header", ROM0[$100]
    nop
    jp Entry
    ds $150 - @, 0

SECTION "Vars", WRAM0
Mode:        db
PingState:   db
PingCount:   db
RequestXfer: db
CcCount:     db
XferCount:   db
PlayerId:    db
PassFlag:    db
RxBuffer:    ds 4

SECTION "Main", ROM0[$150]
Entry:
    di
    ; Enable MBC1 cartridge RAM.
    ld a, $0A
    ld [$0000], a

    xor a
    ld [Mode], a
    ld [PingState], a
    ld [PingCount], a
    ld [RequestXfer], a
    ld [CcCount], a
    ld [XferCount], a
    ld [PlayerId], a
    ld [PassFlag], a
    ld [RxBuffer], a
    ld [RxBuffer + 1], a
    ld [RxBuffer + 2], a
    ld [RxBuffer + 3], a

    ld a, $08
    ldh [rIE], a
    ldh [rIF], a

    ; External clock, waiting for the DMG-07 to clock the first byte.
    xor a
    ldh [rSB], a
    ld a, $80
    ldh [rSC], a
    ei

Wait:
    ld a, [PassFlag]
    and a
    jr z, Wait
    ld a, 1
    ld [$A000], a
Done:
    jr Done

SerialISR:
    push af
    push bc
    push de
    push hl

    ldh a, [rSB]
    ld b, a

    ld a, [Mode]
    cp 2
    jp z, .xfer

    ; Ping phase.
    ld a, b
    cp $CC
    jr nz, .ping
    ld a, [CcCount]
    inc a
    ld [CcCount], a
    cp 4
    jr nz, .ccReply
    ld a, 2
    ld [Mode], a
    xor a
    ld [XferCount], a
.ccReply:
    xor a
    ldh [rSB], a
    ld a, $80
    ldh [rSC], a
    jp .done

.ping:
    ld a, b
    cp $FE
    jr nz, .pingAfterHeader
    ld a, [PingCount]
    inc a
    ld [PingCount], a
    cp 4
    jr c, .pingHeaderReply
    ld a, 1
    ld [RequestXfer], a
.pingHeaderReply:
    ld a, 1
    ld [PingState], a
    ld a, [RequestXfer]
    and a
    jr nz, .replyAA
    ld a, $88
    jr .writeReply

.pingAfterHeader:
    ld a, [PingState]
    cp 1
    jr nz, .pingState2
    ld a, b
    and 7
    ld [PlayerId], a
    ld a, 2
    ld [PingState], a
    ld a, [RequestXfer]
    and a
    jr nz, .replyAA
    ld a, $88
    jr .writeReply

.pingState2:
    cp 2
    jr nz, .pingState3
    ld a, 3
    ld [PingState], a
    ld a, [RequestXfer]
    and a
    jr nz, .replyAA
    ld a, $10
    jr .writeReply

.pingState3:
    cp 3
    jr nz, .replyZero
    ld a, 4
    ld [PingState], a
    ld a, [RequestXfer]
    and a
    jr nz, .replyAA
    ld a, $01
    jr .writeReply

.replyAA:
    ld a, $AA
    jr .writeReply
.replyZero:
    xor a

.writeReply:
    ldh [rSB], a
    ld a, $80
    ldh [rSC], a

    ; The transmission counter is only advanced by real transfer-phase bytes.
    ld a, [Mode]
    cp 2
    jr nz, .done
    ld a, [XferCount]
    inc a
    ld [XferCount], a
    cp 4
    jr nz, .done
    xor a
    ld [XferCount], a
    ld a, [RxBuffer]
    cp 1
    jr nz, .done
    ld a, [RxBuffer + 1]
    cp 2
    jr nz, .done
    ld a, [RxBuffer + 2]
    cp 3
    jr nz, .done
    ld a, [RxBuffer + 3]
    cp 4
    jr nz, .done
    ld a, 1
    ld [PassFlag], a
    jp .done

.xfer:
    ; Store the received byte for the packet check.
    ld a, [XferCount]
    ld c, a
    ld hl, RxBuffer
    add a, l
    ld l, a
    ld a, b
    ld [hl], a

    ; Send the player id on the first byte of the packet, then padding.
    ld a, c
    and a
    jr nz, .xferReplyZero
    ld a, [PlayerId]
    jr .writeReply
.xferReplyZero:
    xor a
    jr .writeReply

.done:
    pop hl
    pop de
    pop bc
    pop af
    reti
