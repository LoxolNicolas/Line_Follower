import serial
import matplotlib.pyplot as plt
import numpy as np

driver = serial.Serial(
        port = 'COM10',
        baudrate = 115200,
        parity = serial.PARITY_NONE,
        stopbits = serial.STOPBITS_ONE,
        bytesize = serial.EIGHTBITS,
        timeout = 0)

print("connexion a: " + driver.portstr)

continuer = True
distance_circuit = 0.0

while continuer:

    tabX = []
    tabY = []
    tabX_ech = []
    tabY_ech = []
    tab_courbure = []

    buffer_caractere = []
    
    while continuer:
        if(driver.in_waiting > 0):  # si des caractères ont été recus
            caractere = driver.read().decode("ascii")  # on decode le caractère
            if(caractere != '\n'):  # si le caractère ne terminait la ligne
                buffer_caractere.append(caractere)  # on ajoute le caractère à la ligne lue
            else:                        # sinon, la ligne est terminée, on traite les données de la ligne
                strl = ''.join(buffer_caractere)  # on transforme la ligne en string
                buffer_caractere = []  # la ligne est remise à zéro pour recevoir une nouvelle ligne
                element = strl.split(" ")  # on crée un tableau contenant les différentes informations de la ligne

                if(element[0] == "stop"):
                    continuer = False
                elif(len(element) >= 5 and element[3] == "C" or len(element) >= 5 and element[4] == "D"):
                    tabX.append(float(element[0]))
                    tabY.append(float(element[1]))
                    if(element[4] == "D"):
                        tabX_ech.append(float(element[0]))
                        tabY_ech.append(float(element[1]))
                        tab_courbure.append(float(element[3]))

    
    plt.plot(tabX, tabY)
    plt.plot(tabX_ech, tabY_ech, 'x')
    plt.xlabel("x en cm")
    plt.ylabel("y en cm")
    plt.axis("equal") #Repere orthonorme
    plt.title("Carte du circuit parcourue par le robot")
    plt.show()
    
    np.savetxt("courbure.txt")
        
print("serial close")

driver.close()