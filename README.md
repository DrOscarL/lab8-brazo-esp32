# Lab 8 · Ejercicio 3 — Teleoperación de un manipulador robótico desde un ESP32 simulado en Velxio

```
[Velxio: ESP32 + 3 potenciómetros + LCD 20x4]
        │  WiFi emulado (SSID "Velxio-GUEST")
        ▼  MQTT TCP 1883
   broker.hivemq.com  ──── MQTT sobre WebSocket (wss 8884) ────►  [GitHub Pages: brazo 3D en Three.js]
        ▲                                                                  │
        └──────────────── tópico .../cmd (texto al LCD) ◄──────────────────┘
```

## Estructura

| Ruta | Contenido |
|---|---|
| `velxio/lab8_ej3_brazo_mqtt.ino` | Sketch para el ESP32 en Velxio |
| `docs/index.html` | Visualizador web (se publica con GitHub Pages) |

## Publicación en GitHub Pages

1. Crear un repositorio público (por ejemplo `DrOscarL/lab8-brazo-esp32`) y subir este contenido.
2. *Settings → Pages → Build and deployment*: Source = *Deploy from a branch*, Branch = `main`, carpeta `/docs`.
3. La página queda en `https://droscarl.github.io/lab8-brazo-esp32/?g=grupo1`.

## Tópicos MQTT

| Tópico | Dirección | Contenido |
|---|---|---|
| `udla/lab8/<GRUPO>/brazo/estado` | ESP32 → web | `{"q":[q1,q2,q3],"p":[x,y,z],"n":"<ull>","t_us":"<ll>"}` |
| `udla/lab8/<GRUPO>/brazo/cmd` | web → ESP32 | Texto de hasta 20 caracteres para la fila 4 del LCD |

## Modelo del manipulador (3 GDL: base, hombro, codo)

$$x = \cos q_1\,[L_2\cos q_2 + L_3\cos(q_2+q_3)],\quad
y = \sin q_1\,[L_2\cos q_2 + L_3\cos(q_2+q_3)],\quad
z = L_1 + L_2\sin q_2 + L_3\sin(q_2+q_3)$$

donde $q_1$ es el ángulo de la base respecto del eje $z$, $q_2$ el ángulo del hombro medido desde el plano horizontal, $q_3$ el ángulo del codo relativo al eslabón 2, y $L_1 = 0{,}10$ m, $L_2 = 0{,}12$ m, $L_3 = 0{,}10$ m son la altura de la base y las longitudes de los eslabones; $(x, y, z)$ es la posición del efector final en metros.

---

## Texto propuesto para la guía

**3) Teleoperación de un manipulador robótico mediante ESP32 y MQTT**

En este ejercicio el ESP32 simulado en Velxio se conecta a Internet mediante WiFi y controla, en tiempo real, un manipulador robótico de 3 grados de libertad visualizado en una página web. Se utilizan los tipos de datos de 8 bytes estudiados en los ejercicios 1 y 2: `double` para el cálculo de la cinemática directa y `long long` / `unsigned long long` para el contador de mensajes y la marca de tiempo en microsegundos.

3a) Cree un nuevo proyecto en Velxio con un ESP32 DevKit V1. Conecte tres potenciómetros a los pines GPIO34, GPIO35 y GPIO32, y el LCD 20x4 I2C a GPIO21 (SDA) y GPIO22 (SCL). Explique por qué, con WiFi activo, deben usarse pines del ADC1.

3b) Copie el código entregado por el profesor, reemplace el valor de `GRUPO` por el identificador de su grupo, agregue las librerías PubSubClient y LiquidCrystal I2C, y ejecute la simulación. Compruebe en el Monitor Serial la conexión WiFi, la conexión al broker MQTT y los valores de ángulos y posición.

3c) Abra la página del visualizador con el identificador de su grupo. Mueva los potenciómetros y verifique que el brazo 3D sigue los movimientos. Registre el valor de ‖p<sub>ESP32</sub> − p<sub>web</sub>‖ y explique por qué es del orden de 10⁻¹⁰ o menor.

3d) Explique por qué el ESP32 envía el contador y la marca de tiempo como texto y no como número en el mensaje JSON (considere el máximo entero exacto que representa JavaScript, 2⁵³ − 1, frente al máximo de `long long`, 2⁶³ − 1).

3e) Envíe un mensaje desde la página web y compruebe que aparece en la fila 4 del LCD.

CUANDO TENGA TODO ESTO FUNCIONANDO 100% OK, MUÉSTRELO FUNCIONANDO AL PROFESOR.

Guarde el Proyecto funcionando y suba el Link Velxio (la URL) al Campus Virtual, un solo Proyecto por cada Grupo.
