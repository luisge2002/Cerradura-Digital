# Cerradura-Digital
# Cerradura Digital Inteligente con ESP8266

Prototipo de cerradura digital inteligente desarrollado con ESP8266, teclado matricial 4x4 mediante expansor PCF8574, relé de apertura, buzzer de alerta, LED indicador, control remoto por Blynk y notificaciones por Telegram a través de un servidor local en PC.

## Descripción del proyecto

Este proyecto busca replicar el funcionamiento básico de una cerradura digital comercial, incorporando mejoras orientadas a conectividad, monitoreo y seguridad electrónica.

El sistema permite abrir una puerta mediante una clave numérica ingresada en un teclado 4x4. Si la clave es correcta, el ESP8266 activa un relé mediante un pulso en bajo para accionar la cerradura eléctrica. Si la clave es incorrecta, se activa un buzzer con un patrón de error.

Además, el sistema permite apertura remota desde Blynk y envía eventos a un servidor local ejecutado en una PC. Este servidor se encarga de enviar notificaciones por Telegram, evitando que el ESP8266 tenga que manejar directamente la comunicación HTTPS con Telegram.

## Funciones principales

- Ingreso de clave mediante teclado matricial 4x4.
- Lectura del teclado mediante expansor I2C PCF8574.
- Apertura de cerradura mediante relé activo en bajo.
- Buzzer de alerta ante clave incorrecta.
- LED indicador por cada pulsación de tecla.
- Apertura remota desde Blynk.
- Servidor local en PC para recibir eventos del ESP8266.
- Notificaciones por Telegram para apertura e intentos fallidos.
- Reconexión automática a WiFi y Blynk.
- Funcionamiento local del teclado aunque no haya internet.

## Arquitectura general

```text
Usuario
  ↓
Teclado 4x4
  ↓
PCF8574 por I2C
  ↓
ESP8266
  ├── Relé → Cerradura eléctrica
  ├── Buzzer → Alerta por clave incorrecta
  ├── LED → Indicador de pulsación
  ├── Blynk → Apertura remota
  └── Servidor local en PC → Telegram
