#!/usr/bin/env python3
"""
combustion_thermometer_debug.py - Live BLE scanner for Combustion Inc Predictive Thermometers.

Prints all 8 probe temperatures (T1=tip, T8=ambient) for every thermometer
found nearby. Cross-reference against the Combustion App (Debug Mode) to
validate the BLE packet parser before flashing to the Pico 2 W.

Requirements:
    pip install bleak

Usage:
    python3 combustion_debug.py
    python3 combustion_debug.py --serial 12345678   # filter to one device
"""

import asyncio
import struct
import argparse
import time
from datetime import datetime
from bleak import BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

COMBUSTION_COMPANY_ID = 0x09C7
PRODUCT_TYPE_PROBE = 0x01
PROBE_COUNT = 8
MIN_PAYLOAD_LENGTH = 18  # 1 type + 4 serial + 13 temp bytes


def extract_13_bits(data: bytes, index: int) -> int:
    """Extract the Nth 13-bit value from a packed byte array (LSB-first)."""
    start_bit = index * 13
    start_byte = start_bit // 8
    bit_offset = start_bit % 8

    raw = data[start_byte] | (data[start_byte + 1] << 8)
    if bit_offset > 3:
        raw |= data[start_byte + 2] << 16

    return (raw >> bit_offset) & 0x1FFF


def parse_payload(payload: bytes) -> dict | None:
    """
    Parse manufacturer-specific payload (bytes after the 2-byte company ID).

    Returns a dict with 'serial' and 'temps' (list of 8 floats in °C),
    or None if the payload is not a recognised Combustion probe advertisement.
    """
    if len(payload) < MIN_PAYLOAD_LENGTH:
        return None
    if payload[0] != PRODUCT_TYPE_PROBE:
        return None

    serial = struct.unpack_from('<I', payload, 1)[0]
    temp_bytes = payload[5:18]  # 13 bytes = 8 × 13-bit packed values
    valid_bytes_reads = [b for b in temp_bytes if b != 0]
    is_valid = len(valid_bytes_reads) > 2

    if not is_valid:
        # print(f'  *** Ignoring invalid/empty payload for serial {serial} ***')
        return None  # device not yet measuring

    # Packet stores T8 (handle/ambient) at index 0, T1 (tip) at index 7.
    # Reverse so temps[0]=T1 and temps[7]=T8 (Ambient).
    temps = [(extract_13_bits(temp_bytes, PROBE_COUNT - 1 - i) * 0.05) - 20.0
             for i in range(PROBE_COUNT)]

    return {'serial': serial, 'temps': temps}


def format_row(result: dict, address: str) -> str:
    temps = result['temps']
    probe_str = '  '.join(f'T{i+1}:{t:6.2f}°C' for i, t in enumerate(temps))
    ambient = temps[-1]
    ts = datetime.now().strftime('%H:%M:%S')
    return (f'[{ts}] Serial {result["serial"]:>10d}  {address}  |  '
            f'{probe_str}  |  Ambient(T8): {ambient:6.2f}°C')


class Scanner:
    def __init__(self, serial_filter: int | None):
        self._serial_filter = serial_filter
        self._seen: set[str] = set()
        self._last_print: dict[str, float] = {}

    def callback(self, device: BLEDevice, advertisement_data: AdvertisementData):
        payload = advertisement_data.manufacturer_data.get(
            COMBUSTION_COMPANY_ID)
        if payload is None:
            return

        result = parse_payload(payload)
        if result is None:
            return

        if self._serial_filter is not None and result['serial'] != self._serial_filter:
            return

        if device.address not in self._seen:
            self._seen.add(device.address)
            print(
                f'  *** New device found: serial {result["serial"]} @ {device.address} ***')

        now = time.monotonic()
        if now - self._last_print.get(device.address, 0.0) < 1.0:
            return
        self._last_print[device.address] = now

        print("Payload:" + str(payload))
        print(format_row(result, device.address))

    async def run(self):
        print('Scanning for Combustion thermometers... (Ctrl+C to stop)\n')
        async with BleakScanner(self.callback):
            while True:
                await asyncio.sleep(1.0)


def main():
    parser = argparse.ArgumentParser(
        description='Combustion BLE debug scanner')
    parser.add_argument('--serial', type=int, default=None,
                        help='Only show output for this serial number')
    args = parser.parse_args()

    scanner = Scanner(serial_filter=args.serial)
    try:
        asyncio.run(scanner.run())
    except KeyboardInterrupt:
        print('\nStopped.')


if __name__ == '__main__':
    main()
