#!/bin/bash

Usage() {
    echo "Usage:"
    echo "  $0 --all --deviceId <deviceId>   : Generate CA, API, and DEVICE certificates"
    echo "  $0 --deviceId <deviceId>         : Generate only DEVICE certificate (requires existing CA/API)"
    exit 1
}

# Default flags
doAll=false
deviceId=""

# Parse parameters
if [ $# -eq 0 ]; then
    Usage
fi

if [ "$1" == "--all" ]; then
    doAll=true
    shift
fi

if [ "$1" == "--deviceId" ]; then
    shift
    deviceId=$1
    shift
else
    Usage
fi

if [ -z "$deviceId" ]; then
    echo "Error: deviceId not specified."
    Usage
fi

# Variables
caDir="CA"
apiDir="API"
deviceName="DEVICE"
confFolder="CertificatesConf"
testDir="TestData/$deviceId"

CreateCA() {
    echo "Creating CA directory..."
    mkdir -p "$caDir"

    echo "Generating CA private key in DER format..."
    openssl genrsa -out "$caDir/${caDir}_key_priv.pem" 1024
    if [ $? -ne 0 ]; then
        echo "Error generating the CA private key."
        exit 1
    fi
    openssl rsa -in "$caDir/${caDir}_key_priv.pem" -outform der -out "$caDir/${caDir}_key_priv.der"
    rm "$caDir/${caDir}_key_priv.pem"

    echo "Generating CA certificate in DER format..."
    openssl req -new -x509 -days 3650 -key "$caDir/${caDir}_key_priv.der" -out "$caDir/${caDir}.pem" -config "$confFolder/ca_openssl.cnf"
    if [ $? -ne 0 ]; then
        echo "Error generating the CA certificate."
        exit 1
    fi
    openssl x509 -in "$caDir/${caDir}.pem" -outform der -out "$caDir/${caDir}.der"
    rm "$caDir/${caDir}.pem"

    echo "Successfully generated CA certificate in DER format."
}

CreateAPI() {
    echo "Creating API directory..."
    mkdir -p "$apiDir"

    echo "Generating API private key in DER format..."
    openssl genrsa -out "$apiDir/api_private_key.pem" 1024
    if [ $? -ne 0 ]; then
        echo "Error generating the API private key."
        exit 1
    fi
    openssl rsa -in "$apiDir/api_private_key.pem" -outform der -out "$apiDir/api_private_key.der"
    rm "$apiDir/api_private_key.pem"

    echo "Generating API CSR..."
    openssl req -new -key "$apiDir/api_private_key.der" -out "$apiDir/${apiDir}.csr" -config "$confFolder/${apiDir}_openssl.cnf"
    if [ $? -ne 0 ]; then
        echo "Error generating the API CSR."
        exit 1
    fi
    openssl req -in "$apiDir/${apiDir}.csr" -outform der -out "$apiDir/${apiDir}_csr.der"
    rm "$apiDir/${apiDir}.csr"

    echo "Signing the API certificate with the CA in DER format..."
    openssl x509 -req -in "$apiDir/${apiDir}_csr.der" -CA "$caDir/${caDir}.der" -CAkey "$caDir/${caDir}_key_priv.der" -CAcreateserial -out "$apiDir/${apiDir}.der" -days 3650 -sha256 -extfile "$confFolder/${apiDir}_openssl.cnf" -extensions v3_ca
    if [ $? -ne 0 ]; then
        echo "Error signing the API certificate."
        exit 1
    fi

    echo "Successfully generated API certificate in DER format."
}

CreateDevice() {
    # Make sure DEVICE directory exists
    if [ ! -d "$deviceName" ]; then
        mkdir "$deviceName"
    fi

    # Remove existing folder for this deviceId if it exists
    if [ -d "$deviceName/$deviceId" ]; then
        rm -rf "$deviceName/$deviceId"
    fi

    mkdir "$deviceName/$deviceId"

    templateFile="$confFolder/${deviceName}_template_openssl.cnf"
    outputFile="$deviceName/$deviceId/${deviceName}_openssl.cnf"

    if [ ! -f "$templateFile" ]; then
        echo "Error: Template file '$templateFile' not found."
        exit 1
    fi

    # Replace placeholder in template
    sed "s/CN = XXXXXXXXXX/CN = $deviceId/" "$templateFile" > "$outputFile"
    echo "Configuration file generated: $outputFile"

    echo "Processing $deviceName in folder $deviceName/$deviceId..."

    # Generate private key (PEM), then convert to DER
    openssl genrsa -out "$deviceName/$deviceId/${deviceName}_private_key.pem" 1024
    if [ $? -ne 0 ]; then
        echo "Error generating RSA private key."
        exit 1
    fi
    openssl rsa -in "$deviceName/$deviceId/${deviceName}_private_key.pem" -outform der -out "$deviceName/$deviceId/${deviceName}_private_key.der"

    # Extract public key in DER
    openssl rsa -in "$deviceName/$deviceId/${deviceName}_private_key.der" -pubout -outform der -out "$deviceName/$deviceId/${deviceName}_key_pub.der"

    # Generate CSR (PEM), convert to DER
    openssl req -new -key "$deviceName/$deviceId/${deviceName}_private_key.pem" -out "$deviceName/$deviceId/${deviceName}.csr" -config "$outputFile"
    if [ $? -ne 0 ]; then
        echo "Error generating the device CSR."
        exit 1
    fi
    openssl req -in "$deviceName/$deviceId/${deviceName}.csr" -outform der -out "$deviceName/$deviceId/${deviceName}_csr.der"
    rm "$deviceName/$deviceId/${deviceName}.csr"

    # Sign device certificate
    openssl x509 -req -in "$deviceName/$deviceId/${deviceName}_csr.der" -CA "$caDir/${caDir}.der" -CAkey "$caDir/${caDir}_key_priv.der" -CAcreateserial -out "$deviceName/$deviceId/${deviceName}.crt" -days 1000 -sha256 -extfile "$outputFile" -extensions v3_ca
    if [ $? -ne 0 ]; then
        echo "Error signing the device certificate."
        exit 1
    fi

    # Convert device certificate to DER
    openssl x509 -in "$deviceName/$deviceId/${deviceName}.crt" -outform der -out "$deviceName/$deviceId/${deviceName}.der"
    rm "$deviceName/$deviceId/${deviceName}.crt"

    # Generate a new CSR for demonstration, in DER format
    openssl req -new -key "$deviceName/$deviceId/${deviceName}_private_key.pem" -out "$deviceName/$deviceId/csr.pem" -config "$confFolder/csr.cnf"
    if [ $? -ne 0 ]; then
        echo "Error generating the new CSR with PEM key."
        exit 1
    fi
    openssl req -in "$deviceName/$deviceId/csr.pem" -outform der -out "$deviceName/$deviceId/csr.der"
    rm "$deviceName/$deviceId/csr.pem"

    echo "$deviceName successfully processed in DER format."

    # Generate test signatures
    echo "Generating signatures for testing..."
    mkdir -p "$testDir"

    buffer="a1b2c3d4e5f60718293a4b5c6d7e8f90123456789abcdef0a1b2c3d4e5f60718"
    echo -n "$buffer" > "$testDir/buffer.bin"

    openssl pkeyutl -encrypt -in "$testDir/buffer.bin" -pubin -inkey "$deviceName/$deviceId/${deviceName}_key_pub.der" -out "$testDir/buffer_encrypted.der"
    if [ $? -ne 0 ]; then
        echo "Error encrypting the buffer."
        exit 1
    fi

    openssl rsa -in "$apiDir/api_private_key.der" -inform DER -out "$apiDir/api_private_key.pem" -outform PEM
    if [ $? -ne 0 ]; then
        echo "Error converting API private key to PEM."
        exit 1
    fi

    openssl dgst -sha256 -sign "$apiDir/api_private_key.pem" -out "$testDir/buffer_encrypted_api.sig" "$testDir/buffer_encrypted.der"
    if [ $? -ne 0 ]; then
        echo "Error signing the buffer with the API key."
        exit 1
    fi

    rm "$apiDir/api_private_key.pem"

    echo "Test signatures successfully generated in DER format."
    echo "Done."
}

if [ "$doAll" = true ]; then
    echo "Running full generation (CA, API, DEVICE)..."
    CreateCA
    CreateAPI
    CreateDevice
else
    echo "Generating only DEVICE certificate. Existing CA and API are required."
    # Check for CA
    if [ ! -f "$caDir/${caDir}.der" ] || [ ! -f "$caDir/${caDir}_key_priv.der" ]; then
        echo "Error: CA certificate or private key not found. Please run with '--all --deviceId <deviceId>' first."
        exit 1
    fi
    # Check for API
    if [ ! -f "$apiDir/${apiDir}.der" ] || [ ! -f "$apiDir/api_private_key.der" ]; then
        echo "Error: API certificate or private key not found. Please run with '--all --deviceId <deviceId>' first."
        exit 1
    fi
    CreateDevice
fi
