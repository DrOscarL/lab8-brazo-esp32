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

