import serial
import time
import os
import subprocess

def openSerialPort(portName="COM5", baudRate=115200):
    ser = serial.Serial(portName, baudRate, timeout=5, write_timeout=1)
    ser.dtr = False
    ser.rts = False
    time.sleep(1)
    return ser

def sendCommand(serialPort, commandDescription, commandData):
    print(f"Sending {commandDescription}...")
    commandWithNewline = commandData + b"\n"
    serialPort.reset_input_buffer()
    time.sleep(0.5)
    serialPort.write(commandWithNewline)
    serialPort.flush()
    time.sleep(2)
    response = serialPort.read_all().decode()
    print("Response:", response[:-2])
    return response[:-2]

def main():
    try:
        ser = openSerialPort("COM5", 115200)
    except Exception as e:
        print("Failed to open serial port:", e)
        return

    scriptCommand = [
        r"C:\Program Files\Git\bin\bash.exe",
        "./CertificatesGenerator.sh",
        "--RSA2048",
        "--all",
        "--deviceId",
        "1"
    ]

    subprocess.run(scriptCommand, check=True)

    sendCommand(ser, "memory clean", b"c@")
    sendCommand(ser, "select RSA2048", b"r2048")
    caPath = os.path.join("CA", "CA.der")
    if os.path.isfile(caPath):
        with open(caPath, "rb") as fileHandle:
            caCertData = fileHandle.read()
            hexData = ("cc0" + caCertData.hex()).encode("utf-8")
        sendCommand(ser, "CA certificate", hexData)
    else:
        print(f"Failed to open CA cert file: {caPath} not found")

    keyPath = os.path.join("DEVICE", "1", "DEVICE_private_key.der")
    if os.path.isfile(keyPath):
        with open(keyPath, "rb") as fileHandle:
            rsaKeyPairData = fileHandle.read()
            hexData = ("ck" + rsaKeyPairData.hex()).encode("utf-8")
        sendCommand(ser, "device RSA key", hexData)
    else:
        print(f"Failed to open device RSA key file: {keyPath} not found")

    deviceCertPath = os.path.join("DEVICE", "1", "DEVICE.der")
    if os.path.isfile(deviceCertPath):
        with open(deviceCertPath, "rb") as fileHandle:
            deviceCertData = fileHandle.read()
            hexData = ("cc1" + deviceCertData.hex()).encode("utf-8")
        sendCommand(ser, "device certificate", hexData)
    else:
        print(f"Failed to open device cert file: {deviceCertPath} not found")

    apiPath = os.path.join("API", "API.der")
    if os.path.isfile(apiPath):
        with open(apiPath, "rb") as fileHandle:
            apiCertData = fileHandle.read()
            hexData = ("cc2" + apiCertData.hex()).encode("utf-8")
        sendCommand(ser, "API certificate", hexData)
    else:
        print(f"Failed to open API cert file: {apiPath} not found")

    sigPath = os.path.join("TestData", "1", "buffer_encrypted_api.sig")
    cipherPath = os.path.join("TestData", "1", "buffer_encrypted.der")
    bufferPath = os.path.join("TestData", "1", "buffer.bin")
    if os.path.isfile(sigPath) and os.path.isfile(cipherPath)and os.path.isfile(bufferPath):
        with open(sigPath, "rb") as sigFile:
            sigData = sigFile.read()
        with open(cipherPath, "rb") as cipherFile:
            cipherData = cipherFile.read()
        
        sigLenHex = "{:04X}".format(len(sigData))
        commandStr = "o" + sigLenHex + sigData.hex() + cipherData.hex()
        answer = sendCommand(ser, "Open message", commandStr.encode("utf-8"))
        with open(bufferPath, "rb") as bufferFile:
            expected_answer = bufferFile.read().decode()
        if(answer != expected_answer):
            print("Error while receiving decrypted buffer")
            print(f"Expected: {expected_answer}")
            print(f"Received: {answer}")
        else:
            print("Received expected answer")

    else:
        print("Failed to open signature and/or ciphertext and/or open buffer file in TestData/1/")

    ser.close()

if __name__ == "__main__":
    main()
