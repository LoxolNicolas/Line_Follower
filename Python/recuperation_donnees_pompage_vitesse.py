import serial
import matplotlib.pyplot as plt
import numpy as np
import keyboard


ser = serial.Serial(
        port = 'COM10',
        baudrate = 115200,
        parity = serial.PARITY_NONE,
        stopbits = serial.STOPBITS_ONE,
        bytesize = serial.EIGHTBITS,
        timeout = 0)

print("connexion a: " + ser.portstr)


buffer_caractere = []

times = []
speeds = []
outputs = []

while True:  # data acquisition loop
    if(ser.in_waiting > 0):  # si des caractères ont été recus
        caractere = ser.read().decode("ascii")  # on decode le caractère
        
        if(caractere != '\n'):  # si le caractère ne terminait la ligne
            buffer_caractere.append(caractere)  # on ajoute le caractère à la ligne lue
        else:                        # sinon, la ligne est terminée, on traite les données de la ligne
            strl = ''.join(buffer_caractere)  # on transforme la ligne en string
            buffer_caractere = []  # la ligne est remise à zéro pour recevoir une nouvelle ligne
            elements = strl.split(" ")  # on crée un tableau contenant les différentes informations de la ligne
            
            if(len(elements) == 3):
                times.append(float(elements[0]))
                speeds.append(float(elements[1]))
                outputs.append(float(elements[2]) * 100)
            
    if keyboard.is_pressed('q'):  # if exit key is pressed
        break;                    # get out of the acquisition loop

npArr = [times, speeds, outputs]

consigne = 120

tabCons = [consigne, consigne]

np.save("pompage_vitesse", npArr)

plt.plot(times, speeds, "-b", label="vitesse (cm/s)")
plt.plot(times, outputs, "-r", label="puissance de sortie (%)")
plt.plot([times[0], times[-1]], tabCons, "-g", label="consigne")
plt.xlabel("temps (s)")
plt.legend(loc="upper left")
plt.title("Mise en pompage du correcteur de régulation de la vitesse (Kp = 0.053)")
plt.show()

        
print("serial close")

ser.close()