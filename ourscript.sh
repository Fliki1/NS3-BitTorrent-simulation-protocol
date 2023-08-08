#!/bin/bash
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
CLIENTIP=$1;
PEERSET=$2;
COMPLETE=$3;
mkdir -p "$SCRIPTPATH/bash-file";	# creo una directory a parte
FILE1=$SCRIPTPATH/$CLIENTIP;
FILE2=$SCRIPTPATH/$PEERSET;
COMPLETEFILE=$SCRIPTPATH/$COMPLETE;

sort="bash-file/sort.txt"
sort -k 5 -n $FILE1 > $sort #ordina con sort

subsort="bash-file/subsort.txt"
awk '{print $1,$2,$3,$4, substr($5,0,length($5)-2),$6}' $sort >$subsort # salvata il sort con "sub dei bps"

# Creiamo i file in base alla banda dei client
awk ' { if ($5 >= 1 && $5 <= 8000000) print $0 }' $subsort > "bash-file/8Mb.txt"
awk ' { if ($5 >= 8000001 && $5 <= 20000000) print $0 }' $subsort > "bash-file/20Mb.txt"
awk ' { if ($5 >= 20000001 && $5 <= 35000000) print $0 }' $subsort > "bash-file/35Mb.txt"
awk ' { if ($5 >= 35000001 && $5 <= 50000000) print $0 }' $subsort > "bash-file/50Mb.txt"

# Otteniamo gli ip per ciuascuna banda, visto il problema con l'ip 1.1.6.157 riscontrato, prendiamo l'ip random tra questi shuf
ip8=$(awk '{print $3}' <<< "$(shuf -n 1 "bash-file/8Mb.txt")")
ip20=$(awk '{print $3}' <<< "$(shuf -n 1 "bash-file/20Mb.txt")")
ip35=$(awk '{print $3}' <<< "$(shuf -n 1 "bash-file/35Mb.txt")")
ip50=$(awk '{print $3}' <<< "$(shuf -n 1 "bash-file/50Mb.txt")")

# Per veridica lo stampo lo stesso - capitano casi in cui non è presente nulla
echo "8 mega: "$ip8
#echo $(grep -w "Ip: $ip8" "$FILE2")
grep -w "Ip: $ip8" "$FILE2" > "bash-file/grep8Mb.txt" #"bash-file/8Mb-$ip8.txt"

echo "20 mega: "$ip20
#echo $(grep -w "Ip: $ip20" "$FILE2")
grep -w "Ip: $ip20" "$FILE2" > "bash-file/grep20Mb.txt"

echo "35 mega: "$ip35
#echo $(grep -w "Ip: $ip35" "$FILE2")
grep -w "Ip: $ip35" "$FILE2" > "bash-file/grep35Mb.txt"

echo "50 mega: "$ip50
#echo $(grep -w "Ip: $ip50" "$FILE2")
grep -w "Ip: $ip50" "$FILE2" > "bash-file/grep50Mb.txt"

# Passaggio indietro, capire la banda associata dei peer suoi associati [dopotutto ci basta capire in che range dei 4 si trovano]
# Array che setta l'intervallo di tempo desiderato da analizzare 
max=( $(tail -1 $FILE2) )

# ip -> nodeid  nodeip-> tempo di fine
# nodeip
id_8=$(awk -v ip="$ip8" '$2==ip {print $1}' RESULTS-active-clients.dat)
id_20=$(awk -v ip="$ip20" '$2==ip {print $1}' RESULTS-active-clients.dat)
id_35=$(awk -v ip="$ip35" '$2==ip {print $1}' RESULTS-active-clients.dat)
id_50=$(awk -v ip="$ip50" '$2==ip {print $1}' RESULTS-active-clients.dat)

#ms
sec_8=$(awk -v ip="$id_8" '$2==ip {print $1}' $COMPLETEFILE)
sec_20=$(awk -v ip="$id_20" '$2==ip {print $1}' $COMPLETEFILE)
sec_35=$(awk -v ip="$id_35" '$2==ip {print $1}' $COMPLETEFILE)
sec_50=$(awk -v ip="$id_50" '$2==ip {print $1}' $COMPLETEFILE)

echo $ip8" ha terminato a "$sec_8
echo $ip20" ha terminato a "$sec_20
echo $ip35" ha terminato a "$sec_35
echo $ip50" ha terminato a "$sec_50
#tempo -ms:
sec8=( ${sec_8:0:-3} )
sec20=( ${sec_20:0:-3} )
sec35=( ${sec_35:0:-3} )
sec50=( ${sec_50:0:-3} )

#conversione ms-secondi
sec8=$(( sec8 / 1000 ))
sec20=$(( sec20 / 1000 ))
sec35=$(( sec35 / 1000 ))
sec50=$(( sec50 / 1000 ))

#########
awk -v ip="$ip8" '$3==ip { print $5 >> "bash-file/8Mb-"ip".txt" }' $FILE1
awk -v ip="$ip20" '$3==ip { print $5 >> "bash-file/20Mb-"ip".txt" }' $FILE1
awk -v ip="$ip35" '$3==ip { print $5 >> "bash-file/35Mb-"ip".txt" }' $FILE1
awk -v ip="$ip50" '$3==ip { print $5 >> "bash-file/50Mb-"ip".txt" }' $FILE1

echo "Popolo il file di connessioni di "$ip8"..."
for (( i = 20; i <= $sec8; i=i+5 )); # 8Mb
do
	awk -v event="$i"s -v ip="$ip8" '$1==event { print event" " $6 >> "bash-file/8Mb-"ip".txt" }' bash-file/grep8Mb.txt 
done

echo "Popolo il file di connessioni di "$ip20"..."
for (( i = 20; i <= $sec20; i=i+5 )); #20Mb
do 
	awk -v event="$i"s -v ip="$ip20" '$1==event { print event" " $6 >> "bash-file/20Mb-"ip".txt" }' bash-file/grep20Mb.txt
done

echo "Popolo il file di connessioni di "$ip35"..."
for (( i = 20; i <= $sec35; i=i+5 )); #35Mb
do
	awk -v event="$i"s -v ip="$ip35" '$1==event { print event" " $6 >> "bash-file/35Mb-"ip".txt" }' bash-file/grep35Mb.txt	
done

echo "Popolo il file di connessioni di "$ip50"..."
for (( i = 20; i <= $sec50; i=i+5 )); #50Mb
do
	awk -v event="$i"s -v ip="$ip50" '$1==event { print event" " $6 >> "bash-file/50Mb-"ip".txt" }' bash-file/grep50Mb.txt	
done

#########


if [[ -e "bash-file/8Mb-"$ip8".txt" ]]; then # Controllo di esistenza del suddetto file
	while read line
	do
  		array=($line)	# Tratto la linea del file come un array puro
  		temp=($(grep -w "Ip: ${array[1]}" "$FILE1")) # grep di tutta la riga -> array
  		echo ${array[0]} " " ${array[1]} " "  ${temp[4]} >> "bash-file/tmp.txt"	# Salvo tutto su un tmp

	done < "bash-file/8Mb-"$ip8".txt"

	mv -f "bash-file/tmp.txt" "bash-file/8Mb-"$ip8".txt"	# che poi elimino risalvando sul file
fi
##########################################################
if [[ -e "bash-file/20Mb-"$ip20".txt" ]]; then # 20
	while read line
	do
  		array=($line)	
  		temp=($(grep -w "Ip: ${array[1]}" "$FILE1"))
  		echo ${array[0]} " " ${array[1]} " "  ${temp[4]} >> "bash-file/tmp.txt"	
	done < "bash-file/20Mb-"$ip20".txt"

	mv -f "bash-file/tmp.txt" "bash-file/20Mb-"$ip20".txt"	
fi
##########################################################
if [[ -e "bash-file/35Mb-"$ip35".txt" ]]; then # 35
	while read line
	do
  		array=($line)	
  		temp=($(grep -w "Ip: ${array[1]}" "$FILE1"))
  		echo ${array[0]} " " ${array[1]} " "  ${temp[4]} >> "bash-file/tmp.txt"	
	done < "bash-file/35Mb-"$ip35".txt"

	mv -f "bash-file/tmp.txt" "bash-file/35Mb-"$ip35".txt"	
fi
##########################################################
if [[ -e "bash-file/50Mb-"$ip50".txt" ]]; then # 50
	while read line
	do
  		array=($line)	
  		temp=($(grep -w "Ip: ${array[1]}" "$FILE1"))
  		echo ${array[0]} " " ${array[1]} " "  ${temp[4]} >> "bash-file/tmp.txt"	
	done < "bash-file/50Mb-"$ip50".txt"

	mv -f "bash-file/tmp.txt" "bash-file/50Mb-"$ip50".txt"	
fi
##########################################################
echo "Terminato!"
