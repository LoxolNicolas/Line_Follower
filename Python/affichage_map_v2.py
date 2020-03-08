import sys
import serial

from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QApplication, QMainWindow, QMenu, QVBoxLayout, QSizePolicy, QMessageBox, QWidget, QPushButton
from PyQt5.QtGui import QIcon


from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import matplotlib.pyplot as plt

import numpy as np

ser = serial.Serial(
        port='COM23',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=0)

def cart2pol(x, y):
    rho = np.sqrt(x**2 + y**2)
    phi = np.arctan2(y, x)
    return(rho, phi)

def pol2cart(rho, phi):
    x = rho * np.cos(phi)
    y = rho * np.sin(phi)
    return(x, y)

class App(QMainWindow):

    def __init__(self):
        super().__init__()
        self.title = 'PyQt5 matplotlib example - pythonspot.com'
        self.width = 1280
        self.height = 720
        self.left = 50
        self.top = 50
        self.initUI()
        
        self.timer = QTimer(self)
        self.timer_interval = 1./20. * 1000.
        
        self.timer.timeout.connect(self.loop)
                   
        self.timer.start(self.timer_interval) 
        
        self.serLine = []

    def initUI(self):
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)

        self.m = PlotCanvas(self, width=11.80, height=7.2)
        self.m.move(0,0)

        button = QPushButton('PyQt5 button', self)
        button.setToolTip('This s an example button')
        button.move(1180,0)
        button.resize(100,100)
        button.clicked.connect(self.click)

        self.show()
        
    def click(self):
        self.m.tabX = []
        self.m.tabY = []
        self.m.tabX_ech = []
        self.m.tabY_ech = []
        self.m.tab_courbure = []
    
    def loop(self):
        if(ser.in_waiting > 0):
            while(ser.in_waiting > 0):  # si des caractères ont été recus
                try:
                    car = ser.read().decode("ascii")  # on decode le caractère
                    if(car != '\n'):  # si le caractère ne terminait la ligne
                        self.serLine.append(car)  # on ajoute le caractère à la ligne lue
                    else:                        # sinon, la ligne est terminée, on traite les données de la ligne
                        strl = ''.join(self.serLine)  # on transforme la ligne en string
                        self.serLine = []  # la ligne est remise à zéro pour recevoir une nouvelle ligne
        
                        elems = strl.split(" ")  # on crée un tableau contenant les différentes informations de la ligne
                        
                        if(elems[0] == 'C'):
                            self.m.tabX.append(float(elems[1]))
                            self.m.tabY.append(float(elems[2]))
                        elif(elems[0] == 'D'):
                            self.m.tabX_ech.append(float(elems[1]))
                            self.m.tabY_ech.append(float(elems[2]))
                            self.m.tab_courbure.append(float(elems[3]))
                            print(strl)
                        else:
                            print(strl)
                except:
                    print("serial read error")
            
            self.m.plot()


class PlotCanvas(FigureCanvas):

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)

        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                QSizePolicy.Expanding,
                QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)
        self.ax = self.figure.add_subplot(111)
        
        self.tabX = []
        self.tabY = []
        self.tabX_ech = []
        self.tabY_ech = []
        self.tab_courbure = []
        
        self.plot()


    def plot(self):
        self.ax.clear()
        self.ax.plot(self.tabX, self.tabY, 'b-')
        self.ax.plot(self.tabX_ech, self.tabY_ech, 'ro')
        self.ax.axis("equal") #Repere orthonorme
        self.draw()
        

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())