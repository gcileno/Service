import json

with open('data/users.json', 'r', encoding='utf-8') as f:
    users = json.load(f)


def autenticar_usuario(password):

    if not password:
        return { "status" : "failure", "code" : 400, "message": "Senha não fornecida" }

    for user in users["data"]:
        if user["password"] == password:
            return { "status" : "success", "code" : 200,"message": "Autenticação bem-sucedida", "username" : user["name"] }

    return { "status" : "failure", "code" : 401 ,"message": "Senha inválida" }
