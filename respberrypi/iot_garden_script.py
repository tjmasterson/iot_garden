from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTShadowClient
import logging
import time
import json
import struct
import smbus


# Shadow JSON schema:
#
# Name: Bot
# {
#	"state": {
#		"desired":{
#			"property":<INT VALUE>
#		}
#	}
# }

# This is the address we setup in the Arduino Program
ARDUINO_ADDRESS = 0x08
TEMP_INDEX = 0
EC_METER_INDEX = 1
D_OXYGEN_INDEX = 2

host = 'abe6cwrtvd51t.iot.us-east-1.amazonaws.com'
rootCAPath = '/opt/devicesdk/CA.pem'
certificatePath = '/opt/devicesdk/99a27b730e-certificate.pem.crt'
privateKeyPath = '/opt/devicesdk/99a27b730e-private.pem.key'
port = 443
useWebsocket = TRUE
thingName = 'GardenRaspberryPi'
# clientId = args.clientId

# Custom Shadow callback
def customShadowCallback_Update(payload, responseStatus, token):
    # payload is a JSON string ready to be parsed using json.loads(...)
    # in both Py2.x and Py3.x
    if responseStatus == "timeout":
        print("Update request " + token + " time out!")
    if responseStatus == "accepted":
        payloadDict = json.loads(payload)
        print("~~~~~~~~~~~~~~~~~~~~~~~")
        print("Update request with token: " + token + " accepted!")
        print("property: " + str(payloadDict["state"]["desired"]["property"]))
        print("~~~~~~~~~~~~~~~~~~~~~~~\n\n")
    if responseStatus == "rejected":
        print("Update request " + token + " rejected!")

def customShadowCallback_Delete(payload, responseStatus, token):
    if responseStatus == "timeout":
        print("Delete request " + token + " time out!")
    if responseStatus == "accepted":
        print("~~~~~~~~~~~~~~~~~~~~~~~")
        print("Delete request with token: " + token + " accepted!")
        print("~~~~~~~~~~~~~~~~~~~~~~~\n\n")
    if responseStatus == "rejected":
        print("Delete request " + token + " rejected!")

# Configure logging
logger = logging.getLogger("AWSIoTPythonSDK.core")
logger.setLevel(logging.DEBUG)
streamHandler = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
streamHandler.setFormatter(formatter)
logger.addHandler(streamHandler)

# Init AWSIoTMQTTShadowClient
myAWSIoTMQTTShadowClient = None
if useWebsocket:
    myAWSIoTMQTTShadowClient = AWSIoTMQTTShadowClient(clientId, useWebsocket=True)
    myAWSIoTMQTTShadowClient.configureEndpoint(host, port)
    myAWSIoTMQTTShadowClient.configureCredentials(rootCAPath)

# AWSIoTMQTTShadowClient configuration
myAWSIoTMQTTShadowClient.configureAutoReconnectBackoffTime(1, 32, 20)
myAWSIoTMQTTShadowClient.configureConnectDisconnectTimeout(10)  # 10 sec
myAWSIoTMQTTShadowClient.configureMQTTOperationTimeout(5)  # 5 sec

# Connect to AWS IoT
myAWSIoTMQTTShadowClient.connect()

# Create a deviceShadow with persistent subscription
deviceShadowHandler = myAWSIoTMQTTShadowClient.createShadowHandlerWithName(thingName, True)

# Delete shadow JSON doc
deviceShadowHandler.shadowDelete(customShadowCallback_Delete, 5)

# for RPI version 1, use "bus = smbus.SMBus(0)"
bus = smbus.SMBus(1)

def buildJSONPayload(sensor_data):
    JSONPayload = {
        'state': {
            'temp': '{:10.2f}'.format(get_float(sensor_data, TEMP_INDEX))
            'ec_meter': 4
            'dissolved_oxygen': 5
        }
    }
    return json.dumbs(JSONPayload)

def get_data():
    return bus.read_i2c_block_data(ARDUINO_ADDRESS, 0);

def get_float(data, index):
    bytes = data[4*index:(index+1)*4]
    byte_array = bytearray(bytes)
    return struct.unpack('f', byte_array)[0]

while True:
    try:
        data = get_data()
        json_payload = buildJSONPayload(data)
        deviceShadowHandler.shadowUpdate(json_payload, customShadowCallback_Update, 5)
        time.sleep(2)
    except Exception as e:
        print(e)
        continue
    time.sleep(2);



