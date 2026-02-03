import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart


def notificar_usuario(destino, assunto, corpo):
    smtp = smtplib.SMTP("smtp.gmail.com", 587)
    smtp.ehlo()
    smtp.starttls()

    email_origem = "gcmc@softex.cin.ufpe.br"
    senha_app = "xbxb wsop fbal lbfm"

    smtp.login(email_origem, senha_app)

    msg = MIMEMultipart()
    msg["From"] = email_origem
    msg["To"] = destino
    msg["Subject"] = assunto

    msg.attach(MIMEText(corpo, "plain"))

    smtp.sendmail(email_origem, destino, msg.as_string())
    smtp.quit()