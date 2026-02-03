import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import os
from dotenv import load_dotenv

load_dotenv()

def notificar_usuario(destino, assunto, corpo):
    smtp = smtplib.SMTP("smtp.gmail.com", 587)
    smtp.ehlo()
    smtp.starttls()

    email_origem = os.getenv("EMAIL")
    senha_app = os.getenv("PASSCODE_EMAIL")

    smtp.login(email_origem, senha_app)

    msg = MIMEMultipart()
    msg["From"] = email_origem
    msg["To"] = destino
    msg["Subject"] = assunto

    msg.attach(MIMEText(corpo, "plain"))

    smtp.sendmail(email_origem, destino, msg.as_string())
    smtp.quit()