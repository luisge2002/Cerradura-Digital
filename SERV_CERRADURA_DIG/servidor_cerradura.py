from flask import Flask
import requests
from datetime import datetime
import threading

app = Flask(__name__)

BOT_TOKEN = "8603340393:AAFl5dMuGtWC4NN8H8FUDJvoJjy-ej6uzAE"
CHAT_ID = "5704950242"


def enviar_telegram(mensaje):
    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"

    data = {
        "chat_id": CHAT_ID,
        "text": mensaje
    }

    try:
        r = requests.post(url, data=data, timeout=10)

        print("Respuesta Telegram:", r.status_code)
        print("Contenido:", r.text)

        if r.status_code == 200:
            print("Mensaje enviado:", mensaje)
        else:
            print("Telegram respondió con error")

    except Exception as e:
        print("Error enviando Telegram:", e)


def enviar_telegram_background(mensaje):
    hilo = threading.Thread(target=enviar_telegram, args=(mensaje,))
    hilo.daemon = True
    hilo.start()


@app.route("/puerta-abierta", methods=["GET"])
def puerta_abierta():
    hora = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    mensaje = f"🔓 Puerta abierta\nHora: {hora}"

    enviar_telegram_background(mensaje)

    return "OK - Puerta abierta recibida", 200


@app.route("/clave-incorrecta", methods=["GET"])
def clave_incorrecta():
    hora = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    mensaje = f"⚠️ Clave incorrecta\nHora: {hora}"

    enviar_telegram_background(mensaje)

    return "OK - Clave incorrecta recibida", 200


@app.route("/", methods=["GET"])
def inicio():
    return "Servidor de cerradura funcionando", 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False, use_reloader=False)