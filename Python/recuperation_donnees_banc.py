import serial
import math
import pprint
import numpy
import os.path

driver = serial.Serial(
        port='COM22',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=0)


# LOXOL

sensor = serial.Serial(
        port='COM19',  # changer
        baudrate=115200,  # pas changer
        parity=serial.PARITY_NONE,  # pas changer
        stopbits=serial.STOPBITS_ONE,  # pas changer
        bytesize=serial.EIGHTBITS,  # pas changer
        timeout=0)  # pas changer

print("connected to: " + driver.portstr + " and " + sensor.portstr)


while True:

    oldDistance = 0
    distance = 0
    datas = []

    accelerationStep = 0
    vitesseStep = 0

    timer_start = 0
    distance_start = 0

    driver_line = []  # LOXOL necessaire
    sensor_line = []

    show = False

    conti = True

    while conti:

        # LOXOL

        if(driver.in_waiting > 0):  # si des caractères ont été recus
            car = driver.read().decode("ascii")  # on decode le caractère
            if(car != '\n'):  # si le caractère ne terminait la ligne
                driver_line.append(car)  # on ajoute le caractère à la ligne lue
            else:                        # sinon, la ligne est terminée, on traite les données de la ligne
                strl = ''.join(driver_line)  # on transforme la ligne en string
                driver_line = []  # la ligne est remise à zéro pour recevoir une nouvelle ligne

                elems = strl.split(" ")  # on crée un tableau contenant les différentes informations de la ligne

                if(elems[0] == "acceleration"):  # le premier élément du tableau/ligne est utilisé comme information du traitement à appliquer
                    datas.append([])
                    accelerationStep = int(elems[1])

                elif(elems[0] == "vitesse"):
                    datas[accelerationStep].append([])
                    vitesseStep = int(elems[1])

                elif(elems[0] == "mesure"):
                    print("mesure " + elems[1])
                    datas[accelerationStep][vitesseStep].append(0)
                    mesure = int(elems[1])

                elif(elems[0] == "endMesure"):
                    print(str(accelerationStep) + " " + str(vitesseStep) + " " + str(mesure))
                    datas[accelerationStep][vitesseStep][mesure] = distance - oldDistance
                    print("measure " + str(distance - oldDistance))
                    oldDistance = distance

                elif(elems[0] == "startMesure"):
                    oldDistance = distance

                elif(elems[0] == "stop"):
                    conti = False

        if(sensor.in_waiting > 0):
            car = sensor.read().decode("ascii")
            sensor_line.append(car)
            if(car == '\n'):
                strl = ''.join(sensor_line)
                sensor_line = []

                elems = strl.split(" ")

                distance = float(elems[0])

    print("data")
    print(datas)

    baseDataFile = 'data.npy'
    
    i = 0
    while os.path.exists(str(i) + baseDataFile):
        i+=1
        
    numpy.save(str(i) + baseDataFile, datas)

    result = []

    for accelerationStep, vitesses in enumerate(datas):
        result.append([])
        for vitesseStep, mesures in enumerate(vitesses):
            result[accelerationStep].append([])

            result[accelerationStep][vitesseStep].append(0)
            result[accelerationStep][vitesseStep].append(0)

            moyenne = 0

            for mesure in mesures:
                moyenne += mesure

            moyenne = moyenne / len(mesures)

            ecartType = 0

            for mesure in mesures:
                ecartType += pow(mesure - moyenne, 2)

            ecartType = math.sqrt(ecartType)

            print("0: " + str(accelerationStep) + "/" + str(len(result)))
            print("1: " + str(vitesseStep) + "/" + str(len(result[accelerationStep])))
            print("2: 0,1/" + str(len(result[accelerationStep][vitesseStep])))

            result[accelerationStep][vitesseStep][0] = moyenne
            result[accelerationStep][vitesseStep][1] = ecartType
    pprint.pprint(result)


print("serial close")

driver.close()
sensor.close()
