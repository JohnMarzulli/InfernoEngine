#!/usr/bin/env python3
"""
combustion_thermometer_debug.py - Live BLE scanner for Combustion Inc devices.

Prints temperature readings for every CPT (Predictive Thermometer) and
GGG (Giant Grill Gauge) found nearby. Both devices share company ID 0x09C7
and are distinguished by the product type byte in the manufacturer payload.

Cross-reference against the Combustion App (Debug Mode) to validate the BLE
packet parser before flashing to the Pico 2 W.

Requirements:
    pip install bleak

Usage:
    python3 combustion_thermometer_debug.py
    python3 combustion_thermometer_debug.py --serial 12345678
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

# Product types (payload byte 0)
PRODUCT_TYPE_CPT = 0x01   # Predictive Thermometer
PRODUCT_TYPE_GGG = 0x03   # Giant Grill Gauge

PRODUCT_NAMES = {
    PRODUCT_TYPE_CPT: 'CPT',
    PRODUCT_TYPE_GGG: 'GGG',
}

# CPT: 1 type + 4 serial + 13 temps
CPT_MIN_PAYLOAD = 18
# GGG: 1 type + 10 serial + 2 temp + 1 status + 1 reserved + 4 alarms
GGG_MIN_PAYLOAD = 19

PROBE_COUNT = 8


# ---------------------------------------------------------------------------
# CPT parsing
# ---------------------------------------------------------------------------

def extract_13_bits(data: bytes, index: int) -> int:
    """Extract the Nth 13-bit value from a packed byte array (LSB-first)."""
    start_bit = index * 13
    start_byte = start_bit // 8
    bit_offset = start_bit % 8

    raw = data[start_byte] | (data[start_byte + 1] << 8)
    if bit_offset > 3:
        raw |= data[start_byte + 2] << 16

    return (raw >> bit_offset) & 0x1FFF


def parse_cpt(payload: bytes) -> dict | None:
    """
    Parse a CPT (Predictive Thermometer) payload.
    Returns dict with 'serial' (int) and 'temps' (8 floats in °C), or None.
    """
    if len(payload) < CPT_MIN_PAYLOAD or payload[0] != PRODUCT_TYPE_CPT:
        return None

    serial = struct.unpack_from('<I', payload, 1)[0]
    temp_bytes = payload[5:18]

    if sum(1 for b in temp_bytes if b != 0) <= 2:
        return None  # not yet measuring

    # Packet stores T8 first, T1 last — reverse so index 0=T1, index 7=T8.
    temps = [(extract_13_bits(temp_bytes, PROBE_COUNT - 1 - i) * 0.05) - 20.0
             for i in range(PROBE_COUNT)]

    return {'product_type': PRODUCT_TYPE_CPT, 'serial': serial, 'temps': temps}


def format_cpt(result: dict, address: str) -> str:
    temps = result['temps']
    probe_str = '  '.join(f'T{i+1}:{t:6.2f}°C' for i, t in enumerate(temps))
    ts = datetime.now().strftime('%H:%M:%S')
    return (f'[{ts}] CPT  Serial {result["serial"]:>10d}  {address}  |  '
            f'{probe_str}  |  Ambient(T8): {temps[-1]:6.2f}°C')


# ---------------------------------------------------------------------------
# GGG parsing
# ---------------------------------------------------------------------------

def decode_alarm(raw16: int) -> dict:
    """
    Decode a 16-bit GGG alarm field.

    Bit layout (spec is 1-indexed; 0-indexed here):
      bit 0   Set       — alarm threshold has been configured
      bit 1   Tripped   — temperature crossed the threshold
      bit 2   Alarming  — alarm is actively sounding
      bits 3-15  13-bit temperature threshold
                 encoding: threshold_C = (raw13 * 0.1) - 20.0
    """
    return {
        'set':       bool(raw16 & 0x0001),
        'tripped':   bool(raw16 & 0x0002),
        'alarming':  bool(raw16 & 0x0004),
        'threshold': ((raw16 >> 3) * 0.1) - 20.0,
    }


def parse_ggg(payload: bytes) -> dict | None:
    """
    Parse a GGG (Giant Grill Gauge) payload.

    Payload layout (after company ID):
      [0]      product type (0x03)
      [1..10]  serial number (10-byte alphanumeric)
      [11..12] raw temperature: lower 13 bits of uint16 LE
                 encoding: temp_C = (raw13 * 0.1) - 20.0
      [13]     status flags: bit0=sensor present, bit1=overheating, bit2=low battery
      [14]     reserved (observed as 0x64=100, possibly battery %)
      [15..16] high alarm (uint16 LE) — see decode_alarm()
      [17..18] low alarm  (uint16 LE) — see decode_alarm()

    Returns dict or None.
    """
    if len(payload) < GGG_MIN_PAYLOAD or payload[0] != PRODUCT_TYPE_GGG:
        return None

    serial = payload[1:11].decode('ascii', errors='replace')

    raw16 = struct.unpack_from('<H', payload, 11)[0]
    raw13 = raw16 & 0x1FFF
    temperature = (raw13 * 0.1) - 20.0

    flags = payload[13]
    sensor_present = bool(flags & 0x01)
    overheating = bool(flags & 0x02)
    low_battery = bool(flags & 0x04)

    high_alarm = decode_alarm(struct.unpack_from('<H', payload, 15)[0])
    low_alarm = decode_alarm(struct.unpack_from('<H', payload, 17)[0])

    return {
        'product_type':  PRODUCT_TYPE_GGG,
        'serial':        serial,
        'temperature':   temperature,
        'sensor_present': sensor_present,
        'overheating':   overheating,
        'low_battery':   low_battery,
        'high_alarm':    high_alarm,
        'low_alarm':     low_alarm,
    }


def format_alarm(label: str, alarm: dict) -> str:
    flags = ''.join([
        ' Set' if alarm['set'] else '',
        ' Tripped' if alarm['tripped'] else '',
        ' Alarming' if alarm['alarming'] else '',
    ]).strip() or 'Off'
    return f'{label}: {alarm["threshold"]:6.2f}°C [{flags}]'


def format_ggg(result: dict, address: str) -> str:
    ts = datetime.now().strftime('%H:%M:%S')

    if not result['sensor_present']:
        temp_str = 'No probe attached'
    else:
        temp_str = f'Pit: {result["temperature"]:6.2f}°C'
        if result['overheating']:
            temp_str += '  [OVERHEATING]'

    device_flags = '  '.join(filter(None, [
        'Low battery' if result['low_battery'] else '',
    ]))
    flags_str = f'  {device_flags}' if device_flags else ''

    alarm_str = (f'  |  {format_alarm("High", result["high_alarm"])}'
                 f'  |  {format_alarm("Low", result["low_alarm"])}')

    return (f'[{ts}] GGG  Serial {result["serial"]}  {address}  |  '
            f'{temp_str}{flags_str}{alarm_str}')


# ---------------------------------------------------------------------------
# Dispatch
# ---------------------------------------------------------------------------

def parse_payload(payload: bytes) -> dict | None:
    if not payload:
        return None
    product_type = payload[0]
    if product_type == PRODUCT_TYPE_CPT:
        return parse_cpt(payload)
    if product_type == PRODUCT_TYPE_GGG:
        return parse_ggg(payload)
    return None


def format_row(result: dict, address: str) -> str:
    if result['product_type'] == PRODUCT_TYPE_CPT:
        return format_cpt(result, address)
    return format_ggg(result, address)


# ---------------------------------------------------------------------------
# Scanner
# ---------------------------------------------------------------------------

class Scanner:
    def __init__(self, serial_filter: int | None):
        self._serial_filter = serial_filter
        self._seen:       set[str] = set()
        self._last_print: dict[str, float] = {}

    def callback(self, device: BLEDevice, advertisement_data: AdvertisementData):
        payload = advertisement_data.manufacturer_data.get(
            COMBUSTION_COMPANY_ID)
        if payload is None:
            return

        result = parse_payload(payload)
        if result is None:
            return

        # Serial filter applies to CPT only (GGG serial is a string).
        if (self._serial_filter is not None
                and result['product_type'] == PRODUCT_TYPE_CPT
                and result['serial'] != self._serial_filter):
            return

        if device.address not in self._seen:
            self._seen.add(device.address)
            name = PRODUCT_NAMES.get(result['product_type'],
                                     f'0x{result["product_type"]:02X}')
            print(
                f'  *** New device: {name}  serial {result["serial"]}  @ {device.address} ***')

        now = time.monotonic()
        if now - self._last_print.get(device.address, 0.0) < 1.0:
            return
        self._last_print[device.address] = now

        print('Payload: ' + payload.hex(' '))
        print(format_row(result, device.address))

    async def run(self):
        print('Scanning for Combustion devices... (Ctrl+C to stop)\n')
        async with BleakScanner(self.callback):
            while True:
                await asyncio.sleep(1.0)


def main():
    parser = argparse.ArgumentParser(
        description='Combustion BLE debug scanner')
    parser.add_argument('--serial', type=int, default=None,
                        help='Filter CPT output to this serial number')
    args = parser.parse_args()

    scanner = Scanner(serial_filter=args.serial)
    try:
        asyncio.run(scanner.run())
    except KeyboardInterrupt:
        print('\nStopped.')


if __name__ == '__main__':
    main()
