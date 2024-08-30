import socket
import sys

DEFAULT_BUFLEN = 512

def mandar_mensaje(sock, mensaje):
    try:
        sock.sendall(mensaje.encode())
    except socket.error as e:
        print(f"send failed with error: {e}")
        sock.close()
        sys.exit(1)

def recibir_mensaje(sock, mensaje):
    try:
        recvbuf = sock.recv(DEFAULT_BUFLEN).decode()
        if recvbuf:
            print(recvbuf)
            return 1
    except socket.error as e:
        print(f"recv failed with error: {e}")
        return 0
    print(mensaje)
    return 0

def main():
    if len(sys.argv) != 3:
        print(f"usage: {sys.argv[0]} server-name port")
        sys.exit(1)

    server_name = sys.argv[1]
    port = int(sys.argv[2])

    try:
        # Crear un socket para conectarse al servidor
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((server_name, port))

            opcion = None
            while opcion != '3':
                print("\n1. Generar usuario.")
                print("\n2. Generar contrasena.")
                print("\n3. Cerrar sesion.")
                opcion = input("\n\nElija una opcion: ")

                if opcion == '1':
                    while True:
                        longitud = input("Indique la longitud que desea (5-15): ")
                        if longitud == "volver":
                            break
                        if not longitud.isdigit():
                            print("Error: La longitud debe ser un numero.")
                            continue

                        msj = f"a-{longitud}"
                        mandar_mensaje(s, msj)
                        salida = "Cliente desconectado por inactividad\n"
                        if recibir_mensaje(s, salida) == 0:
                            break

                elif opcion == '2':
                    while True:
                        longitud = input("Indique la longitud que desea (8-49): ")
                        if longitud == "volver":
                            break
                        if not longitud.isdigit():
                            print("Error: La longitud debe ser un numero.")
                            continue

                        msj = f"b-{longitud}"
                        mandar_mensaje(s, msj)
                        salida = "Cliente desconectado por inactividad\n"
                        if recibir_mensaje(s, salida) == 0:
                            break

                elif opcion == '3':
                    s.shutdown(socket.SHUT_SEND)

                else:
                    print("\nLa opción elegida es inválida\n")

    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
