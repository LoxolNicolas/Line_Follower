import serial
import matplotlib.pyplot as plt

driver = serial.Serial(
        port = 'COM10',
        baudrate = 115200,
        parity = serial.PARITY_NONE,
        stopbits = serial.STOPBITS_ONE,
        bytesize = serial.EIGHTBITS,
        timeout = 0)

print("connexion a: " + driver.portstr)

continuer = True

while continuer:

    tabX = []
    tabY = []

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
                else:
                    tabX.append(float(element[0]))
                    tabY.append(float(element[1]))
                    print(float(element[2]))
    
    plt.plot(tabX, tabY)
    plt.xlabel("x en cm")
    plt.ylabel("y en cm")
    plt.axis("equal") #Repere orthonorme
    plt.title("Carte du circuit parcourue par le robot")
    plt.show()
        
print("serial close")

driver.close()