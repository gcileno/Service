from socket import socket, AF_INET, SOCK_STREAM
from service import criar_socket, parse_requisicao, tratar_rotas, resposta_http, receber_requisicao

def main():

    user_logado = {}

    servidor = criar_socket()

    while True:
        conn, addr = servidor.accept()
        print("Conexão de:", addr)

        data = receber_requisicao(conn)

        if not data:
            conn.close()
            continue

        print("Requisição recebida:\n", data)

        metodo, rota, body = parse_requisicao(data)

        conteudo = tratar_rotas(metodo, rota, body, user_logado)
        resposta = resposta_http(conteudo)

        conn.sendall(resposta)
        conn.close()


if __name__ == "__main__":
    main()