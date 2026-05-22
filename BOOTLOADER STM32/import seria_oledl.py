import serial
import time
from pathlib import Path

SOF = 0xAA

CMD_START_UPDATE = 0x01
CMD_WRITE_DATA   = 0x02
CMD_END_UPDATE   = 0x03

ACK  = 0x79
NACK = 0x1F

CHUNK_SIZE = 32
MAX_FW_SIZE = 16 * 1024   # vung temp: 0x08004000 -> 0x08007FFF


def calc_checksum(cmd: int, data: bytes) -> int:
    total = cmd + len(data)
    for b in data:
        total += b
    return total & 0xFF


def make_packet(cmd: int, data: bytes = b"") -> bytes:
    checksum = calc_checksum(cmd, data)
    return bytes([SOF, cmd, len(data)]) + data + bytes([checksum])


def wait_ack(ser: serial.Serial) -> bool:
    resp = ser.read(1)

    if not resp:
        print("Timeout: khong nhan duoc phan hoi")
        return False

    if resp[0] == ACK:
        print("ACK")
        return True

    if resp[0] == NACK:
        print("NACK")
        return False

    print(f"Phan hoi khac: 0x{resp[0]:02X}")
    return False


def send_packet(ser: serial.Serial, packet: bytes, expect_ack: bool = True) -> bool:
    ser.write(packet)
    ser.flush()

    if not expect_ack:
        return True

    return wait_ack(ser)


def send_firmware(port: str, bin_path: str, baudrate: int = 9600) -> None:
    firmware_path = Path(bin_path)

    if not firmware_path.exists():
        print(f"Loi: khong tim thay file firmware: {firmware_path}")
        return

    firmware = firmware_path.read_bytes()

    print(f"Mo cong {port} @ {baudrate}")
    print(f"Firmware: {firmware_path}")
    print(f"Kich thuoc firmware: {len(firmware)} bytes")

    if len(firmware) == 0:
        print("Loi: file firmware rong")
        return

    if len(firmware) > MAX_FW_SIZE:
        print("Loi: firmware qua lon, vuot qua vung temp 16KB")
        return

    ser = serial.Serial(
        port=port,
        baudrate=baudrate,
        timeout=2,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        xonxoff=False,
        rtscts=False,
        dsrdtr=False
    )

    try:
        ser.setDTR(False)
        ser.setRTS(False)

        time.sleep(1.0)
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        print("Gui START_UPDATE...")
        pkt = make_packet(CMD_START_UPDATE)
        if not send_packet(ser, pkt):
            print("Loi START_UPDATE")
            return

        offset = 0
        while offset < len(firmware):
            chunk = firmware[offset:offset + CHUNK_SIZE]
            pkt = make_packet(CMD_WRITE_DATA, chunk)

            print(f"Gui DATA offset={offset}, len={len(chunk)}")
            if not send_packet(ser, pkt):
                print(f"Loi tai offset {offset}")
                return

            offset += len(chunk)
            time.sleep(0.01)

        print("Gui END_UPDATE...")
        pkt = make_packet(CMD_END_UPDATE)
        if not send_packet(ser, pkt):
            print("Loi END_UPDATE")
            return

        print("Da gui xong firmware.")
        print("Neu app hop le, bootloader se jump sang app.")

    finally:
        ser.close()


if __name__ == "__main__":
    send_firmware(
        port="COM12",
        bin_path=r"D:\ARM\STM32\bin_test_oled\Objects\1.bin",
        baudrate=9600
    )