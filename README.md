
# NetRate

## Licencia: GNU

### Autor:

Cristian Solervicéns.

---

# Nota:
Acabo de dejar funcionando este programa que encontré entre mis cachureos de hace quién sabe cuantos años.
En su momento me fue de bastante utilidad, hoy es un cachureo entretenido de cuando programaba bastante en C, cuando WindowsXP era lo máximo, y cuando Visual Basic 6 era el rey.

Me dió algun trabajo porque estoy un poco oxidado con "C" pero logré compilarlo en el WSL de Windows con Ubuntu 24.02 (y bajo windows también debería compilar con Dev-C++) !!!

```
gcc net-rate.c -o net-rate
```

## Descripción:

Es una utilidad Cliente-Servidor linux/windows de línea de comando, que permite
comprobar de forma simple y confiable e independiente la tasa de transferencia a
través de la red entre dos computadores (o la misma) mediante la transferencia de un buffer
de datos.


## Modo de uso:

   Ejecute NetRate como SERVIDOR (opción -l) en el Computador A y NetRate como
   Cliente (sin opcion -l) en el Computador B, representando ambos los extremos
   de la red que se desean medir.
   Por omisión opera con protocolo TCP, bajo el puerto 5001 con paquete de
   1024 Kb.

Servidor "A":

   Copie el programa en alguna carpeta de las máquinas "A" y "B". Si es una
   carpeta que está en el path srá más simple de usar, pues no se deberá cambiar
   al directorio en que copió el programa. (Jajajaja, esto es de la época de DOS)
   
   Abra una ventana DOS en "A", ejecute "NetRate -l". Para ver la ayuda en línea,
   escriba: NetRate -h
   
   Si desea usar un puerto específico: NetRate -l -e <puerto>
   Para finalizar el programa presione Ctrl+C

Cliente "B":

   Abra una ventana DOS en "B", para ver la ayuda en línea,
   ejecute NetRate -h
   
   Una línea de comando típica es de la forma:
   NetRate -n <servidor> -e <puerto> -s <kb-a-enviar>
   
   Donde <servidor> es el nombre de la máquina "A" o su IP
         <puerto>   el puerto en que el ServidorTx está escuchando en "A"
         <kb-a-enviar> KiloBytes de datos a enviar al Servidor
         
   Ejecutado NetRate, desplegará el número de KBytes enviados al Servidor
   y el tiempo en milisegundos empleado para ello.

## Nota2:

También, si usas la opción -f file_name en cliente y servidor el programa puede
transferir un archivo de una máquina a otra.