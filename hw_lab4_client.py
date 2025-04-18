from bluepy.btle import Peripheral,UUID,Scanner,DefaultDelegate
import struct

class AccDelegate(DefaultDelegate):
    def __init__(self):
        super().__init__()

    def handleNotification(self, cHandle, data):
        try:
            x, y, z = struct.unpack('<hhh', data)
            print(f"acc: X: {x}, Y: {y}, Z: {z}")
        except:
            print("fail to receive data.")

scanner = Scanner()
devices = scanner.scan(10.0)
for dev in devices:
    for(adtype,desc,value) in dev.getScanData():
        if desc == "Complete Local Name" and value == "BlueNRG":
            target_mac = dev.addr
            
print("Connecting...")
print(target_mac)
dev = Peripheral(target_mac,"random")
dev.setDelegate(AccDelegate())

service = dev.getServiceByUUID(UUID("00000000000111e119ab40002a5d5c51b"))
print("Enabling notifications (CCCD = 0x0002)...")
char2 = dev.getCharacteristics(uuid=UUID("00000000000111e119ab40002a5d5c51b"))[0]
cccd = char2.getDescriptors()[0]
cccd.write(b"\x01\x00",withResponse = True)

char = dev.getCharacteristics(uuid=UUID("00000000000111e119ab40002a5d5c51b"))[1]
value = struct.pack("B",0x02) #You can modify this value
char.write(value,withResponse = True)
print("Successfully write frequency into stm32. Use Ctrl+C to end.")

while(1):
    if dev.waitForNotifications(50.0):
        continue