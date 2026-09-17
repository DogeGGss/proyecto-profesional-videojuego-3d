# CATCH'EM

El juego de la mancha, en 3D. Proyecto desarrollado en **Unreal Engine 4.27**.

🌐 **[Sitio del juego](https://dogeggss.github.io/proyecto-profesional-videojuego-3d/)** — capturas, tabla de récords y descargas
🎮 **[Descargar para Windows](https://github.com/DogeGGss/proyecto-profesional-videojuego-3d/releases/download/v0.7.0-test-uni/CATCH.EM.rar)** · **[Descargar para Android](https://github.com/DogeGGss/proyecto-profesional-videojuego-3d/releases/download/v0.5.0-test-android-uni/CATCH.EM.apk)**

## Modos de juego

**Mancha y evasor** — multijugador de 2 jugadores en red local (sin servidores). Uno crea la partida, el otro se une. Rondas cronometradas de 30 segundos, marcador al mejor de cinco, detección de toque por proximidad.

**El circuito** — un jugador, contrarreloj. Recorrido de obstáculos (péndulos, barredoras, plataformas móviles, pinchos, portales) sin condición de derrota: si te caés, reaparecés en el punto de partida. El mejor tiempo se guarda entre partidas.

## Controles

| Acción | Windows | Android |
|---|---|---|
| Moverse | `W` `A` `S` `D` | Joystick izquierdo |
| Mirar | Mouse | Arrastrar |
| Saltar | `Espacio` | Botón en pantalla |
| Pausa / salir | `Esc` | Botón en pantalla |

En Windows los controles se pueden reasignar desde Opciones. En Android son fijos.

## Requisitos

| | Mínimos | Recomendados |
|---|---|---|
| Objetivo | 30 FPS, calidad baja, 1920×1080 | 60 FPS, calidad alta, 1920×1080 |

Probado en un Ryzen 5 2400G con gráficos integrados (58 FPS en calidad alta) y en un Ryzen 9 7900 con RX 9070 XT (543 FPS en calidad alta).

## Estructura del repositorio

```
4.27/MyProject/     Proyecto de Unreal Engine 4.27 (código fuente, contenido, configuración)
docs/               Sitio del juego (GitHub Pages)
gestion-proyecto/   Documentación de gestión por sprint (decisiones técnicas, backlog, riesgos, retrospectivas)
```

## Desarrollo

Requiere **Unreal Engine 4.27**. Abrir `4.27/MyProject/MyProject.uproject` desde el Epic Games Launcher o Unreal Editor.

Los assets binarios (`.uasset`, `.umap`) se versionan con Git LFS — asegurate de tener [Git LFS](https://git-lfs.com/) instalado antes de clonar.
