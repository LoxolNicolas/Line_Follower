#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
import numpy as np
import os.path

# Prend en paramatre un tableau de tableau 
def Affiche_Moustache(data): 
    mini = data[0][1]
    maxi = data[0][1]
    
    for tab in data:
        mini2 = min(tab[1:])
        mini = min(mini, mini2)
        maxi2 = max(tab[1:])
        maxi = max(maxi, maxi2)
        
    mini -= (maxi - mini) * 0.05
    maxi += (maxi - mini) * 0.05
    
    print(mini, maxi)
    
    
    format_box = 410
    nb = format_box + 1  # Pour subplot
    for tab in data:
        plt.subplot(nb)
        
        titre = tab[0]
        
        plt.boxplot(tab[1:], vert=0)  # Affichage verticale
        x = np.random.normal(1, 0.00, size=len(tab)- 1)
        
        plt.plot(tab[1:], x, 'bo',alpha=0.2)
        
        
        plt.title(titre)
        plt.xlabel("Erreur de mesure du capteur sur un trajet rectiligne de 130cm, 40 echantillons, lot " + str(nb - format_box))
        plt.gca().xaxis.set_tick_params(direction='in', pad=10)  # Modication des ticks en x
        plt.gca().yaxis.set_ticks([0])  # Non affichage des ticks en y
        plt.xlim([mini, maxi])
        plt.xticks(np.arange(mini , maxi , (maxi - mini) / 5.000000001 ))  # Nombre de tick en x (le pas vas surment change)
        
        #plt.subplot(nb+1)

        print("Mediane : " + str(np.median(tab[1:])))
        print("1er quartile : " + str(np.quantile(tab[1:], 0.25)))
        print("3eme quartile : " + str(np.quantile(tab[1:], 0.75)))
        print("Min : " + str(min(tab[1:])))
        print("Max : " + str(max(tab[1:])))

        nb += 1
    plt.subplots_adjust(hspace=0.9, wspace=0.5)  # Espacement entre les plots
    plt.show()


def main():
    
    baseDataFile = "data.npy"
    ndata = [['2 lots combinés, 80 echantillons']]  # this will be our non numpy list, way more easy to use
    
    i = 1
    
    while os.path.exists(str(i) + baseDataFile):
        if(i == 1):
            data = np.load(str(i) + baseDataFile)[0][0][1:]
        else:
            data = np.concatenate( (data, np.load(str(i) + baseDataFile)[0][0][1:]) )
        i += 1
        print(str(i) + baseDataFile)
    print(data)
        
    # data is now a binary numpy array, we want to convert it to title and values
    
            
    for mesure in range(len(data)):
        ndata[0].append( float(data[mesure]) - 130 ) # we convert binary data into floats
#                        
    
    # Affiche_Moustache(ndata)

    
    baseDataFile = "data.npy"
    
    i = 1
    
    while os.path.exists(str(i) + baseDataFile):
        if(i == 1):
            data = np.load(str(i) + baseDataFile)
        else:
            data = np.concatenate( (data, np.load(str(i) + baseDataFile)) )
        i += 1
        print(str(i) + baseDataFile)
        
    # data is now a binary numpy array, we want to convert it to title and values
    
    # ndata = []  # this will be our non numpy list, way more easy to use
    
    vMax = len(data)
    for vitesse in range(vMax):
        for acc in range(len(data[vitesse])):
            
            ndata.append([])  # a new set of measures needs to be appended to the list
            
            # next we fill this new set with the title + data
            
            ndata[-1].append( data[vitesse][acc][0].decode() )  # we need to decode the title from binary data
            
            for mesure in range(1, len(data[vitesse][acc])):
                ndata[-1].append( float(data[vitesse][acc][mesure]) - 130 ) # we convert binary data into floats
#                        
    print(ndata)
    Affiche_Moustache(ndata)


if __name__ == "__main__":
    # execute only if run as a script
    main()
