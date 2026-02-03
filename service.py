import socket
import sys
import time
import json
from autentication import autenticar_usuario

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

def receber_requisicao(conn):
    data = ""

    # lê até chegar o fim do header
    while "\r\n\r\n" not in data:
        data += conn.recv(1024).decode("utf-8")

    header, _, rest = data.partition("\r\n\r\n")

    # pega o Content-Length
    content_length = 0
    for linha in header.split("\r\n"):
        if linha.lower().startswith("content-length"):
            content_length = int(linha.split(":")[1].strip())

    # lê o body inteiro
    while len(rest) < content_length:
        rest += conn.recv(1024).decode("utf-8")

    return header + "\r\n\r\n" + rest

def parse_requisicao(data):
    cabecalho, _, corpo = data.partition("\r\n\r\n")

    linhas = cabecalho.split("\r\n")
    metodo, rota, _ = linhas[0].split()

    body = corpo.strip()
    print("Corpo da requisição:", body)

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


def tratar_rotas(metodo, rota, body, user_logado):
    if metodo == "GET":

        if rota == "/":
            return "<h1>Servidor ESP32 OK</h1>"

        elif rota == "/status":
            return "STATUS: OK"
        
        elif rota == "/alarme/on":
            print('enviar email com notificação para usuario logado')
            
        elif rota == "/led/on":
            print("LED LIGADO (GET)")
            return "LED ON"

        elif rota == "/led/off":
            print("LED DESLIGADO (GET)")
            return "LED OFF"

        else:
            return "Rota GET inválida"

    elif metodo == "POST":

        if rota == "/autentication":
            aut = autenticar_usuario(body, user_logado)
            print(user_logado)
            return aut

    return "Método não suportado"



