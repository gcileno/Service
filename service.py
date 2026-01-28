import socket
import sys
import time

HOST = ''
PORT = 80

def obter_ip_local():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    finally:
        s.close()
    return ip


def criar_socket():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.bind((HOST, PORT))
    except socket.error as e:
        print("Erro ao bindar:", e)
        sys.exit()

    s.listen(1)

    ip = obter_ip_local()

    print("Servidor HTTP rodando!")
    print(f"Acesse no navegador: http://{ip}:{PORT}/")

    return s



def parse_requisicao(data):
    linhas = data.split("\r\n")

    # Primeira linha: GET /rota HTTP/1.1
    metodo, rota, _ = linhas[0].split()

    body = ""
    if metodo == "POST":
        body = linhas[-1]  # corpo simples (ESP32 geralmente manda assim)

    return metodo, rota, body


def resposta_http(conteudo, status="200 OK"):
    resposta = (
        f"HTTP/1.1 {status}\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        f"{conteudo}"
    )
    return resposta.encode("utf-8")


def tratar_rotas(metodo, rota, body):
    if metodo == "GET":

        if rota == "/":
            return "<h1>Servidor ESP32 OK</h1>"

        elif rota == "/status":
            return "STATUS: OK"

        elif rota == "/led/on":
            print("LED LIGADO (GET)")
            return "LED ON"

        elif rota == "/led/off":
            print("LED DESLIGADO (GET)")
            return "LED OFF"

        else:
            return "Rota GET inválida"

    elif metodo == "POST":

        if rota == "/led":
            print("POST recebido:", body)

            if body == "on":
                print("LED LIGADO (POST)")
                return "LED ON"

            elif body == "off":
                print("LED DESLIGADO (POST)")
                return "LED OFF"

            else:
                return "Comando inválido"

        elif rota == "/autentication":
            return "Rota POST inválida"

    return "Método não suportado"



