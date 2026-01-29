import json

with open('data/users.json', 'r', encoding='utf-8') as f:
    users = json.load(f)


def notification_alarme(user):

    if not user:
        return { "status" : "failure", "code" : 400, "message": "Usuário não fornecido" }
    
    for u in users["data"]:
        if u["name"] == user:
            # Aqui você pode adicionar a lógica para enviar a notificação
            print(f"Notificação de alarme disparado enviada para o usuário: {user}")
            return { "status" : "success", "code" : 200,"message": f"Notificação enviada para {user}" }