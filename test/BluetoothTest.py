import asyncio
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "0000FF00-0000-1000-8000-00805F9B34FB"
CHAR_UUID_WRITE = "0000FF01-0000-1000-8000-00805F9B34FB"
CHAR_UUID_NOTIFY = "0000FF02-0000-1000-8000-00805F9B34FB"

MAX_PACKET_SIZE = 20  # Tamanho máximo de um pacote BLE

def calculate_bcc(data):
    """Calcula o BCC (XOR de todos os bytes)."""
    bcc = 0
    for byte in data:
        bcc ^= byte
    return bcc

async def main():
    print("Procurando pelo dispositivo ESP32_BLE...")
    
    # Escanear dispositivos BLE
    devices = await BleakScanner.discover()
    esp32_address = None
    
    for device in devices:
        if device.name == "ESP32_BLE":
            esp32_address = device.address
            break
    
    if not esp32_address:
        print("Dispositivo ESP32_BLE não encontrado.")
        return
    
    print(f"Dispositivo ESP32_BLE encontrado: {esp32_address}")
    
    async with BleakClient(esp32_address) as client:
        if not client.is_connected:
            print("Falha ao conectar.")
            return
        
        print(f"Conectado ao dispositivo {esp32_address}")

        # Callback para notificações
        def notification_handler(sender, data):
            try:
                decoded_data = data.decode('utf-8')
                print(f"recebido: {decoded_data}")
            except UnicodeDecodeError:
                print(f"recebido (não-UTF-8): {data.hex()}")

        # Inscrever-se para notificações
        await client.start_notify(CHAR_UUID_NOTIFY, notification_handler)
        print(f"Inscrito nas notificações de {CHAR_UUID_NOTIFY}")

        # Loop para envio de comandos
        while True:
            comando = input("Digite o comando para enviar (ou 'sair' para encerrar): ")
            if comando.lower() == "sair":
                break

            comando_bytes = comando.encode('utf-8')

            if len(comando_bytes) <= MAX_PACKET_SIZE - 1:
                # Comando cabe em um único pacote, sem fragmentação
                await client.write_gatt_char(CHAR_UUID_WRITE, comando_bytes)
                print(f"envio: {comando_bytes.hex()} (utf8: {comando})")
            else:
                # Fragmentar e enviar comando longo
                total_length = len(comando_bytes)
                current_index = 0

                # Calcula o BCC de todo o comando antes de fragmentar
                bcc = calculate_bcc(comando_bytes)

                while current_index < total_length:
                    # Definir header para o fragmento
                    is_last_fragment = (current_index + (MAX_PACKET_SIZE - 2)) >= total_length
                    header = b'\x00' if is_last_fragment else b'\x01'

                    # Determinar limite do fragmento
                    fragment_end = current_index + (MAX_PACKET_SIZE - 2)
                    fragment = comando_bytes[current_index:fragment_end]
                    current_index = fragment_end

                    # Construir o pacote
                    packet = header + fragment

                    # Adicionar BCC apenas no último fragmento
                    if is_last_fragment:
                        packet += bytes([bcc])

                    # Enviar fragmento
                    await client.write_gatt_char(CHAR_UUID_WRITE, packet)
                    print(f"envio: {packet.hex()} (header: {header.hex()}{', bcc: ' + hex(bcc) if is_last_fragment else ''})")


        # Cancelar notificações antes de encerrar
        await client.stop_notify(CHAR_UUID_NOTIFY)
        print("Notificações canceladas.")

# Executar o programa principal
asyncio.run(main())
