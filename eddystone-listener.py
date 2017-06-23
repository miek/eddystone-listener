import time
from beacontools import BeaconScanner, EddystoneEIDFrame, EddystoneEncryptedTLMFrame
from binascii import hexlify

# TODO: use a more suitable data structure that'll remove old entries
eids = {}

def callback(bt_addr, rssi, packet, additional_info):
    if isinstance(packet, EddystoneEIDFrame):
        eids[bt_addr] = packet.eid
    else:
        print("<%s, %d> %s %s %s" %
              (bt_addr, rssi, packet, additional_info, hexlify(eids[bt_addr])))

# scan for all EID frames of beacons
scanner = BeaconScanner(callback, 
    packet_filter=[EddystoneEIDFrame, EddystoneEncryptedTLMFrame],
)

scanner.start()
time.sleep(10)
scanner.stop()
