import matplotlib.pyplot as plt
import collections
import sys
import numpy as np
 

def update_diz(tmp,dic): #popola il dizionario di elementi che hanno il tempo come chiave e come valori il conteggio delle connessioni per banda
	if int(tmp[0][:-1]) in dic.keys(): #se è presente la chiave nella lista delle chiavi non la crea, ma aggiorna soltanto il valore 
		dic[int(tmp[0][:-1])] += 1 # "tmp[0][:-1]" prende il tempo della simulazione togliendo la s(secondi) 
	else:
		dic[int(tmp[0][:-1])] = 1


f= open(sys.argv[-1],"r") 

banda = [{},{},{},{}] #crea una lista di quattro dizionari relativa alle bande in analisi: 8Mb,20Mb,35Mb,50Mb 
mbnd=f.readline() #legge la prima riga del file passato come argomento da shell. La prima riga mostra la banda effettiva del peer locale, mentre le
# nelle righe successive alla prima sono presenti le bande effettive dei peer remoti.  
mbnd=mbnd.strip() #toglie gli spazi di inizio e fine della riga
myband=int(mbnd[:-3]) #toglie la stringa "bps" e converte il valore in intero.
f.close()

f=open(sys.argv[-1],"r")
for x in f: #questo ciclo serve a interpretare il fenomeno della clusterizzazione dei peer remoti
	tmp = x.split() #fa uno split della riga prendendo lo spazio come carattere delimitatore. La riga del file è di questo tipo: 20s 1.1.6.217 38000000bps
	if tmp[0][len(tmp[0])-3:] != "bps": #controlla se sta leggendo la prima riga del file
		hisband=int(tmp[2][:-3])
		if abs(hisband - myband) <= 7000000: #if |tmp2 - myband| <= 7Mb then giallo
			update_diz(tmp,banda[0])
					
		elif abs(hisband - myband) > 7000000 and abs(hisband - myband) <= 15000000: #if |tmp2 - myband| <= 15Mb && |tmp2 - myband| > 7Mb then verde
			update_diz(tmp,banda[1])

		elif abs(hisband - myband) > 15000000 and abs(hisband - myband) <=30000000: #if |tmp2 - myband| <= 30Mb && |tmp2 - myband| > 15Mb then blu
			update_diz(tmp,banda[2])
		
		else:
			update_diz(tmp,banda[3]) #else rosso

namefile=f.name
f.close()

for i in xrange(0,4):
	banda[i] = collections.OrderedDict(sorted(banda[i].items())) #ordina gli elementi dei dizionari per chiave

# Le quattro istruzioni successive fanno le seguenti operazioni:

# 1)banda.values() prende la lista dei valori del dizionario "banda[x]" con 0<=x<=3;
# 2) ogni valore della lista viene sommato (o sottratto) per una quantità piccola. Questo passaggio serve a non sovrappore i mark 
	#delle bande durante la rappresentazione;
# 3) i valori risultanti vengono assegnati a un'altra lista chiamata "valueBandax";
valueBanda8 = [x+0.08 for x in banda[0].values()] 
valueBanda20 = [x+0.01 for x in banda[1].values()]
valueBanda35 = [x-0.05 for x in banda[2].values()]
valueBanda50 = [x-0.11 for x in banda[3].values()]

plt.rcParams.update({'font.size': 28}) #viene impostato il font per il plot
plt.figure(figsize=(20,15)) #la dimensione della figura che contiene il plot

##Le quattro istruzioni a seguire servono a plottare i valori nel grafico. La funzione plot() prende come argomenti:

# 1) la lista delle chiavi (secondi), 
# 2) il numero di connessioni per banda ottenuto dalla funzione "update_diz"
# 3) il tipo di mark da rappresentare
# 4) la dimensione del mark
# 5) con "markevery=8" vengono rappresentati i mark a step di 8. Questo argormento ci consente di non ottenere i grafici molto densi e 
# 	a dare una facile interpretazione di ciò che si sta rappresentando

plt.plot(banda[0].keys(),valueBanda8,'y-o', markersize=8, markevery=8)  #8Mb
plt.plot(banda[1].keys(),valueBanda20,'g-o', markersize=8, markevery=8) #20Mb
plt.plot(banda[2].keys(),valueBanda35,'b-o', markersize=8, markevery=8) #35Mb
plt.plot(banda[3].keys(),valueBanda50,'r-o', markersize=8, markevery=8) #50Mb

plt.yticks([0,1,2,3,4,5,6]) #rappresenta i valori della lista [0,1,2,3,4,5,6] nell'asse delle ordinate
plt.xlabel("secondi")
plt.ylabel("n. connessioni")
plt.legend(['|hisband-myband|<7Mb','|hisband-myband|<15Mb','|hisband-myband|<30Mb','|hisband-myband|<50Mb'])
plt.title(sys.argv[-1][:-4]+'banda effettiva '+str(mbnd))
plt.savefig(namefile[:-4]+'.png')
plt.show() # mostra il grafico

