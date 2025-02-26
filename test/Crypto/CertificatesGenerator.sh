#!/bin/bash

usage() {
    echo "Uso: $0 <hardware id>"
    exit 1
}

if [ "$#" -lt 1 ]; then
    usage
fi

hardware_id=$1
name="DEVICE"
ca_dir="CA"
api_dir="API"
conf_folder="CertificatesConf"
test_dir="TestData/$hardware_id"

create_ca() {
    echo "Criando diretório da CA..."
    mkdir -p "$ca_dir"

    echo "Gerando chave privada da CA em DER..."
    openssl genrsa -out "$ca_dir/${ca_dir}_key_priv.pem" 1024
    if [ $? -ne 0 ]; then
        echo "Erro ao gerar a chave privada da CA."
        exit 1
    fi
    openssl rsa -in "$ca_dir/${ca_dir}_key_priv.pem" -outform der -out "$ca_dir/${ca_dir}_key_priv.der"
    rm "$ca_dir/${ca_dir}_key_priv.pem"

    echo "Gerando certificado da CA em DER..."
    openssl req -new -x509 -days 3650 -key "$ca_dir/${ca_dir}_key_priv.der" -out "$ca_dir/${ca_dir}.pem" -config "$conf_folder/ca_openssl.cnf"
    if [ $? -ne 0 ]; then
        echo "Erro ao gerar o certificado da CA."
        exit 1
    fi
    openssl x509 -in "$ca_dir/${ca_dir}.pem" -outform der -out "$ca_dir/${ca_dir}.der"
    rm "$ca_dir/${ca_dir}.pem"

    echo "Certificado da CA criado com sucesso em formato DER."
}

create_api() {
    echo "Criando diretório da API..."
    mkdir -p "$api_dir"

    echo "Gerando chave privada da API em DER..."
    openssl genrsa -out "$api_dir/api_private_key.pem" 1024
    if [ $? -ne 0 ]; then
        echo "Erro ao gerar a chave privada da API."
        exit 1
    fi
    openssl rsa -in "$api_dir/api_private_key.pem" -outform der -out "$api_dir/api_private_key.der"
    rm "$api_dir/api_private_key.pem"

    echo "Gerando CSR da API..."
    openssl req -new -key "$api_dir/api_private_key.der" -out "$api_dir/${api_dir}.csr" -config "$conf_folder/${api_dir}_openssl.cnf"
    if [ $? -ne 0 ]; then
        echo "Erro ao gerar o CSR da API."
        exit 1
    fi
    openssl req -in "$api_dir/${api_dir}.csr" -outform der -out "$api_dir/${api_dir}_csr.der"
    rm "$api_dir/${api_dir}.csr"

    echo "Assinando o certificado da API com a CA em DER..."
    openssl x509 -req -in "$api_dir/${api_dir}_csr.der" -CA "$ca_dir/${ca_dir}.der" -CAkey "$ca_dir/${ca_dir}_key_priv.der" -CAcreateserial -out "$api_dir/${api_dir}.der" -days 3650 -sha256 -extfile "$conf_folder/${api_dir}_openssl.cnf" -extensions v3_ca
    if [ $? -ne 0 ]; then
        echo "Erro ao assinar o certificado da API."
        exit 1
    fi

    echo "Certificado da API criado com sucesso em formato DER."
}

if [ ! -f "$ca_dir/${ca_dir}.der" ] || [ ! -f "$ca_dir/${ca_dir}_key_priv.der" ]; then
    echo "Certificado ou chave privada da CA não encontrado."
    create_ca
else
    echo "Certificado da CA encontrado."
fi

if [ ! -f "$api_dir/${api_dir}.der" ] || [ ! -f "$api_dir/api_private_key.der" ]; then
    echo "Certificado ou chave privada da API não encontrado."
    create_api
else
    echo "Certificado da API encontrado."
fi

if [ ! -d "$name" ]; then
    mkdir "$name"
fi

if [ -d "$name/$hardware_id" ]; then
    rm -rf "$name/$hardware_id"
fi

mkdir "$name/$hardware_id"

template_file="$conf_folder/${name}_template_openssl.cnf"
output_file="$name/$hardware_id/${name}_openssl.cnf"

if [ ! -f "$template_file" ]; then
    echo "Erro: O arquivo de template '$template_file' não foi encontrado."
    exit 1
fi

sed "s/CN = XXXXXXXXXX/CN = $hardware_id/" "$template_file" > "$output_file"

echo "Arquivo de configuração gerado: $output_file"

echo "Processando $name na pasta $name/$hardware_id..."

openssl genrsa -out "$name/$hardware_id/${name}_private_key.pem" 1024
if [ $? -ne 0 ]; then
    echo "Erro ao gerar a chave RSA."
    exit 1
fi
openssl rsa -in "$name/$hardware_id/${name}_private_key.pem" -outform der -out "$name/$hardware_id/${name}_private_key.der"

openssl rsa -in "$name/$hardware_id/${name}_private_key.der" -pubout -outform der -out "$name/$hardware_id/${name}_key_pub.der"

openssl req -new -key "$name/$hardware_id/${name}_private_key.pem" -out "$name/$hardware_id/${name}.csr" -config "$output_file"
if [ $? -ne 0 ]; then
    echo "Erro ao gerar o CSR do dispositivo."
    exit 1
fi
openssl req -in "$name/$hardware_id/${name}.csr" -outform der -out "$name/$hardware_id/${name}_csr.der"
rm "$name/$hardware_id/${name}.csr"

openssl x509 -req -in "$name/$hardware_id/${name}_csr.der" -CA "$ca_dir/${ca_dir}.der" -CAkey "$ca_dir/${ca_dir}_key_priv.der" -CAcreateserial -out "$name/$hardware_id/${name}.crt" -days 1000 -sha256 -extfile "$output_file" -extensions v3_ca
if [ $? -ne 0 ]; then
    echo "Erro ao assinar o certificado do dispositivo."
    exit 1
fi

openssl x509 -in "$name/$hardware_id/${name}.crt" -outform der -out "$name/$hardware_id/${name}.der"
rm "$name/$hardware_id/${name}.crt"

openssl req -new -key "$name/$hardware_id/${name}_private_key.pem" -out "$name/$hardware_id/csr.pem" -config "$conf_folder/csr.cnf"
if [ $? -ne 0 ]; then
    echo "Erro ao gerar a nova CSR com a chave PEM."
    exit 1
fi
openssl req -in "$name/$hardware_id/csr.pem" -outform der -out "$name/$hardware_id/csr.der"
rm "$name/$hardware_id/csr.pem"

echo "$name processado com sucesso em formato DER."

echo "Gerando assinaturas para teste..."

mkdir -p "$test_dir"

buffer="a1b2c3d4e5f60718293a4b5c6d7e8f90123456789abcdef0a1b2c3d4e5f60718"

echo "$buffer" | xxd -r -p > "$test_dir/buffer.bin"

openssl pkeyutl -encrypt -in "$test_dir/buffer.bin" -pubin -inkey "$name/$hardware_id/${name}_key_pub.der" -out "$test_dir/buffer_encrypted.der"
if [ $? -ne 0 ]; then
    echo "Erro ao encriptar o buffer."
    exit 1
fi

openssl dgst -sha256 -sign "$api_dir/api_private_key.der" -out "$test_dir/buffer_encrypted_api.sig" "$test_dir/buffer_encrypted.der"
if [ $? -ne 0 ]; then
    echo "Erro ao assinar o buffer com a chave da API."
    exit 1
fi

echo "Assinaturas para teste geradas com sucesso em formato DER."
