#!/usr/bin/env python3
"""
NRF24L01+PA+LNA loopback test - RECEIVER side
Run on the DRONE RPi.

Hardware:
  CE_PIN  = 25 (BCM GPIO25)
  CSN_PIN = 0  (SPI0 CE0, GPIO8)
  MOSI=GPIO10, MISO=GPIO9, SCK=GPIO11

Usage:
  sudo python3 receiver.py
"""

import sys
import time
import struct
from pyrf24 import RF24, RF24_PA_MIN, RF24_250KBPS

# ── Config (MUST match transmitter.py exactly) ────────────────────────────────
CE_PIN       = 25
CSN_PIN      = 0
CHANNEL      = 76
PA_LEVEL     = RF24_PA_MIN    # switch to RF24_PA_HIGH after bench test passes
DATA_RATE    = RF24_250KBPS
PAYLOAD_SIZE = 32             # fixed, no dynamic payloads
PIPE_ADDR    = b"1CTRL"       # 5-byte address — matches C++ CTRL_ADDR
RETRY_DELAY  = 5
RETRY_COUNT  = 15
# ─────────────────────────────────────────────────────────────────────────────


def decode_payload(raw: bytes) -> tuple[int, bool]:
    counter    = struct.unpack("<I", raw[:4])[0]
    padding_ok = all(b == 0xAA for b in raw[4:])
    return counter, padding_ok


def main():
    print("[RX] Initializing RF24...")
    radio = RF24(CE_PIN, CSN_PIN)

    if not radio.begin():
        print("[RX] FATAL: radio.begin() returned False.")
        print("     Check:")
        print("       1. SPI enabled: sudo raspi-config -> Interfaces -> SPI")
        print("       2. Device exists: ls /dev/spidev0.0")
        print("       3. Wiring: MOSI=GPIO10 MISO=GPIO9 SCK=GPIO11 CSN=GPIO8 CE=GPIO25")
        print("       4. VCC: NRF24+PA draws ~150mA — use dedicated 3.3V reg + decoupling caps.")
        sys.exit(1)

    print("[RX] radio.begin() OK — SPI established")

    radio.set_channel(CHANNEL)
    print(f"[RX] Channel: {CHANNEL} ({2400 + CHANNEL} MHz)")

    radio.set_pa_level(PA_LEVEL)
    print(f"[RX] PA level: {PA_LEVEL}  (0=MIN 1=LOW 2=HIGH 3=MAX)")

    radio.set_data_rate(DATA_RATE)
    print("[RX] Data rate: RF24_250KBPS")

    radio.set_auto_ack(True)          # ENABLED — receiver sends ACKs automatically
    print("[RX] Auto-ack: ENABLED")

    radio.dynamic_payloads = False
    radio.payload_length = PAYLOAD_SIZE
    print(f"[RX] Fixed payload size: {PAYLOAD_SIZE} bytes")

    radio.set_retries(RETRY_DELAY, RETRY_COUNT)

    # Open pipe 1 — NOT pipe 0. Pipe 0 is reserved internally for auto-ack TX address.
    radio.open_reading_pipe(1, PIPE_ADDR)
    print(f"[RX] Reading pipe 1: {PIPE_ADDR}")

    radio.start_listening()           # PRX mode
    print("[RX] PRX (receive) mode active")

    print("\n===== REGISTER DUMP =====")
    radio.print_details()
    print("=========================\n")

    print("[RX] Listening... Ctrl+C to stop\n")

    received_total = 0
    dropped_total  = 0
    last_counter   = None
    last_status_t  = time.monotonic()

    try:
        while True:
            if radio.available():
                raw = radio.read(PAYLOAD_SIZE)
                received_total += 1

                # RPD latches on received signal power — read immediately after receive
                rssi_ok = radio.rpd

                counter, padding_ok = decode_payload(raw)

                if last_counter is not None:
                    gap = counter - last_counter - 1
                    if gap > 0:
                        dropped_total += gap
                        print(f"[RX] WARNING: {gap} packet(s) dropped "
                              f"(expected #{last_counter+1}, got #{counter})")

                last_counter  = counter
                last_status_t = time.monotonic()

                integrity = "OK" if padding_ok else "CORRUPT"
                print(f"[RX] #{counter:06d}  rssi_ok={rssi_ok}  "
                      f"payload={integrity}  "
                      f"rx={received_total}  dropped={dropped_total}")

                if not padding_ok:
                    print(f"[RX]   raw: {raw.hex()}")

            else:
                # Print status every 5s while waiting
                if time.monotonic() - last_status_t > 5.0:
                    print(f"[RX] Waiting... (received {received_total} so far)")
                    last_status_t = time.monotonic()
                time.sleep(0.001)

    except KeyboardInterrupt:
        print(f"\n[RX] Done. received={received_total}  dropped={dropped_total}")


if __name__ == "__main__":
    main()
