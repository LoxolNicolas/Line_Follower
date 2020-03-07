#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import os.path

# Prend en paramatre un tableau de tableau 
def Affiche_Moustache(data): 
    mini = data[0][0]
    maxi = data[0][0]
    
    for tab in data:
        mini2 = min(tab)
        mini = min(mini, mini2)
        maxi2 = max(tab)
        maxi = max(maxi, maxi2)
        
    mini -= (maxi - mini) * 0.03
    maxi += (maxi - mini) * 0.03
    
    mini = np.floor(mini*10)/10
    maxi = np.ceil(maxi*10)/10
    
    print(mini, maxi)
    
    
    format_box = 110
    nb = format_box + 1  # Pour subplot
    
    ###########
    amax = 3000
    vmax = 1000
    
    
    colors = ['lightblue', 'lightgreen', 'pink']
    colors2 = ['blue', 'green', 'red']
    
    pink_patch = mpatches.Patch(color='lightblue', label='0.333 m/s')
    lightgreen_patch = mpatches.Patch(color='lightgreen', label='0.666 m/s')
    lightblue_patch = mpatches.Patch(color='pink', label='1 m/s')
    plt.legend(title = 'Vitesses', loc = 'lower left', handles=[lightblue_patch, lightgreen_patch, pink_patch])

    ###########
        
    plt.xlabel("Erreur de mesure de distance du capteur en fonction de la vitesse et de l'accélération (%)")
    plt.gca().xaxis.set_tick_params(direction='in', pad=10)  # Modication des ticks en x
    ## plt.gca().yaxis.set_ticks([0])  # Non affichage des ticks en y
    plt.gca().xaxis.grid(True)
    plt.xlim([mini, maxi])
    plt.xticks(np.arange(mini , maxi , (maxi - mini) / 10.000000001 ))  # Nombre de tick en x (le pas vas surment changer)
    
    for num, tab in enumerate(data):
        #plt.subplot(nb)
        
        x = [num] * len(tab)
        if((num + 1) % 3 == 0):
            plt.axhline(num + 0.5, color = "black")
        
        accel = amax * ((((nb - format_box - 1) // 3) + 1) / 3)
        vitesse = vmax * ((((nb - format_box - 1) % 3) + 1) / 3)
        
        plt.plot(tab, x, marker = 'o', color = colors2[num % 3],alpha=0.2, zorder=2)
        
        var = plt.boxplot(tab, positions = [num],
                          vert=False,
                          labels = ["acceleration : " + str(int(accel)/1000.0) + " m/s²"] if (num%3 == 1) else [""],
                          widths = [0.8],
                          patch_artist=True,  # fill with color
                          zorder=1
                          )
        
        var['boxes'][0].set_facecolor(colors[num % 3])
        
        
        #plt.subplot(nb+1)

        print("Mediane : " + str(np.median(tab[1:])))
        print("1er quartile : " + str(np.quantile(tab[1:], 0.25)))
        print("3eme quartile : " + str(np.quantile(tab[1:], 0.75)))
        print("Min : " + str(min(tab[1:])))
        print("Max : " + str(max(tab[1:])))

        nb += 1
    #plt.subplots_adjust(hspace=0.2, wspace=0.2)  # Espacement entre les plots
    plt.show()


def main():
    
    dataFile = "2data.npy"
    ndata = []  # this will be our non numpy list, way more easy to use
    
    data = np.load(dataFile);
    
    print(data)
    
    for vitesse in range(len(data)):
        for acc in range(len(data[vitesse])):
            
            ndata.append([])  # a new set of measures needs to be appended to the list
    
    for vitesse in range(len(data)):
        for acc in range(len(data[vitesse])):
            
            #ndata.append([])  # a new set of measures needs to be appended to the list
            
            # next we fill this new set with the title + data
            
            ## ndata[-1].append( data[vitesse][acc][0].decode() )  # we need to decode the title from binary data
            
            for mesure in range(0, len(data[vitesse][acc])):
                ndata[acc * len(data) + vitesse].append( ((float(data[vitesse][acc][mesure])) - 130.0) / 130.0 * 100 ) # we convert binary data into floats
#                        
    print(ndata)
    Affiche_Moustache(ndata)


if __name__ == "__main__":
    # execute only if run as a script
    main()
