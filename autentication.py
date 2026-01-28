import json

with open('data/users.json', 'r', encoding='utf-8') as f:
    users = json.load(f)


def autenticar_usuario(password):

    if not password:
        return { "status" : "failure", "message": "Senha não fornecida" }

    for user in users["data"]:
        if user["password"] == password:
            return { "status" : "success", "message": "Autenticação bem-sucedida", "username" : user["name"] }

    return { "status" : "failure", "message": "Senha inválida" }
