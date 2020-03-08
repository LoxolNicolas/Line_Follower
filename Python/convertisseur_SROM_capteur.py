# -*- coding: utf-8 -*-
"""
Created on Sun Mar  8 15:20:03 2020

@author: Felix
"""

inputFileName = 'SROM_input.txt'
outputFileName = 'SROM_output.txt'

inputFile = open(inputFileName, 'r')
outputFile = open(outputFileName, 'w')

outputFile.write("{\n")

for (count, line) in enumerate(inputFile):
    outputFile.write("0x" + line[0:2] + ", ")
    if((count+1) % 16 == 0):
        outputFile.write("\n")

inputFile.close()
outputFile.close()

print("done")
