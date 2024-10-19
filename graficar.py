import cv2
import matplotlib.pyplot as plt
import numpy as np

image_path = '/ImagenesBio/DRIVE/26_training.tif'

image = cv2.imread(image_path)

if image is None:
    print("No se pudo cargar la imagen.")
else:
    print("La imagen se cargó correctamente.")

    image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # Cargar los puntos desde el archivo de texto de coordenadas
    puntos_file = 'coordenadas_maximas.txt'
    with open(puntos_file, 'r') as file:
        puntos = [tuple(map(int, line.split())) for line in file]

    # Dibujar los puntos sobre la imagen
    for punto in puntos:
        cv2.circle(image_rgb, punto, 5, (255, 0, 0), -1) 

    # Cargar los coeficientes de la parábola desde el archivo de texto
    coeficientes_file = 'ultimos_coeficientes.txt'
    with open(coeficientes_file, 'r') as file:
        coeficientes = [float(line.strip()) for line in file]

    # Generar puntos para graficar la parábola
    y_values = np.linspace(0, image_rgb.shape[0], 1000)
    x_values = coeficientes[0] * y_values**2 + coeficientes[1] * y_values + coeficientes[2]

    # Limitar los valores de x para que estén dentro de los límites de la imagen
    x_values = np.clip(x_values, 0, image_rgb.shape[1] - 1)

    # Dibujar la parábola sobre la imagen
    for i in range(len(y_values)):
        cv2.circle(image_rgb, (int(x_values[i]), int(y_values[i])), 1, (0, 255, 0), -1)

    # Mostrar la imagen con los puntos y la parábola
    plt.imshow(image_rgb)
    plt.axis('off')
    plt.show()
