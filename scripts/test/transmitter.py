#!/usr/bin/env python3
"""
NRF24L01+PA+LNA loopback test - TRANSMITTER side
Run on the CONTROLLER RPi.

Hardware:
  CE_PIN  = 25 (BCM GPIO25)
  CSN_PIN = 0  (SPI0 CE0, GPIO8)
  MOSI=GPIO10, MISO=GPIO9, SCK=GPIO11

Usage:
  sudo python3 transmitter.py
"""

import sys
import time
import struct
from pyrf24 import RF24, RF24_PA_MIN, RF24_250KBPS

# ── Config (MUST match receiver.py exactly) ───────────────────────────────────
CE_PIN        = 25
CSN_PIN       = 0
CHANNEL       = 76
PA_LEVEL      = RF24_PA_MIN   # switch to RF24_PA_HIGH after bench test passes
DATA_RATE     = RF24_250KBPS
PAYLOAD_SIZE  = 32            # fixed, no dynamic payloads
PIPE_ADDR     = b"1CTRL"      # 5-byte address — matches C++ CTRL_ADDR
RETRY_DELAY   = 5             # 5 x 250us = 1.25ms between retransmits
RETRY_COUNT   = 15
SEND_INTERVAL = 1.0           # seconds between transmissions
# ─────────────────────────────────────────────────────────────────────────────


def build_payload(counter: int) -> bytes:
    packed  = struct.pack("<I", counter)
    padding = bytes([0xAA] * (PAYLOAD_SIZE - len(packed)))
    return packed + padding


def main():
    print("[TX] Initializing RF24...")
    radio = RF24(CE_PIN, CSN_PIN)

    if not radio.begin():
        print("[TX] FATAL: radio.begin() returned False.")
        print("     Check:")
        print("       1. SPI enabled: sudo raspi-config -> Interfaces -> SPI")
        print("       2. Device exists: ls /dev/spidev0.0")
        print("       3. Wiring: MOSI=GPIO10 MISO=GPIO9 SCK=GPIO11 CSN=GPIO8 CE=GPIO25")
        print("       4. VCC: NRF24+PA draws ~150mA — use dedicated 3.3V reg + decoupling caps.")
        sys.exit(1)

    print("[TX] radio.begin() OK — SPI established")

    radio.setChannel(CHANNEL)
    print(f"[TX] Channel: {CHANNEL} ({2400 + CHANNEL} MHz)")

    radio.setPALevel(PA_LEVEL)
    print(f"[TX] PA level: {PA_LEVEL}  (0=MIN 1=LOW 2=HIGH 3=MAX)")

    radio.setDataRate(DATA_RATE)
    print("[TX] Data rate: RF24_250KBPS")

    radio.setAutoAck(True)
    print("[TX] Auto-ack: ENABLED")

    radio.disableDynamicPayloads()
    radio.setPayloadSize(PAYLOAD_SIZE)
    print(f"[TX] Fixed payload size: {PAYLOAD_SIZE} bytes")

    radio.setRetries(RETRY_DELAY, RETRY_COUNT)
    print(f"[TX] Retries: delay={RETRY_DELAY}x250us, count={RETRY_COUNT}")

    radio.openWritingPipe(PIPE_ADDR)
    print(f"[TX] Writing pipe: {PIPE_ADDR}")

    radio.stopListening()
    print("[TX] PTX (transmit) mode active")

    print("\n===== REGISTER DUMP =====")
    radio.printDetails()
    print("=========================\n")

    # Noise check
    radio.startListening()
    time.sleep(0.001)
    noise = radio.testRPD()
    radio.stopListening()
    if noise:
        print(f"[TX] WARNING: high RF noise on channel {CHANNEL} (RPD=True).")
        print("     Try: sudo nmcli radio wifi off   or move module away from RPi.")
    else:
        print(f"[TX] Noise floor ch{CHANNEL}: OK (RPD clear)")

    print(f"\n[TX] Sending every {SEND_INTERVAL}s — Ctrl+C to stop\n")

    counter    = 0
    sent_total = 0
    ack_total  = 0

    try:
        while True:
            payload = build_payload(counter)
            t0      = time.monotonic()
            acked   = radio.write(payload)
            elapsed = (time.monotonic() - t0) * 1000

            sent_total += 1
            if acked:
                ack_total += 1

            arc  = radio.getARC()
            rate = ack_total / sent_total * 100

            if acked:
                print(f"[TX] #{counter:06d}  ACK=YES  retries={arc}  "
                      f"tx={elapsed:.1f}ms  success={rate:.1f}%")
            else:
                print(f"[TX] #{counter:06d}  ACK=NO   retries={arc}  "
                      f"tx={elapsed:.1f}ms  success={rate:.1f}%  <-- LOST")

            counter += 1
            time.sleep(SEND_INTERVAL)

    except KeyboardInterrupt:
        print(f"\n[TX] Done. sent={sent_total}  acked={ack_total}  "
              f"lost={sent_total - ack_total}  success={ack_total/max(sent_total,1)*100:.1f}%")


if __name__ == "__main__":
    main()
