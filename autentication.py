import json

with open('data/users.json', 'r', encoding='utf-8') as f:
    users = json.load(f)


def autenticar_usuario(password):

    if not password:
        return f"FAIL"

    for user in users["data"]:
        if user["password"] == password:
            return f"OK|{user['name']}|{user['email']}|{user['username']}"

    return f"FAIL"
