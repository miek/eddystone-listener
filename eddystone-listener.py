import time
from beacontools import BeaconScanner, EddystoneEIDFrame

def callback(bt_addr, rssi, packet, additional_info):
    print("<%s, %d> %s %s" % (bt_addr, rssi, packet, additional_info))

# scan for all EID frames of beacons
scanner = BeaconScanner(callback, 
    packet_filter=EddystoneEIDFrame
)

scanner.start()
time.sleep(10)
scanner.stop()
