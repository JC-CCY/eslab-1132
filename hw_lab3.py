from bluepy.btle import Peripheral, UUID, Scanner, DefaultDelegate

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)
        elif isNewData:
            print("Received new data from", dev.addr)

# 1. 掃描 BLE 設備
scanner = Scanner()
devices = scanner.scan(10.0)

n=0
addr = []
for dev in devices:
    print("%d: Device %s (%s), RSSI = %s dB" %(n,dev.addr,dev.addrType,dev.rssi))
    addr.append(dev.addr)
    n+=1
    for(adtype,desc,value) in dev.getScanData():
        print("%s = %s" %(desc,value))
    if n>10:
        break

# 2. 選擇設備
number = input("Enter your device number: ")
num = int(number)
device_addr = addr[num]

print(f"Connecting to {device_addr}...")
dev = Peripheral(device_addr, 'random')

print("Services...")
for svc in dev.services:
    print(str(svc))

try:
    service = dev.getServiceByUUID(UUID(0x1813))
    for ch in service.getCharacteristics():
        print(str(ch))
        cccd = ch.getDescriptors()[0].handle
        print(cccd)

    print("Enabling notifications (CCCD = 0x0002)...")
    dev.writeCharacteristic(60, b"\x02\x00", withResponse=False)

    print("CCCD successfully set to 0x0002.")

finally:
    dev.disconnect()
    print("Disconnected.")
