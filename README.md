# Instrucciones de instalación y ejecución

Dentro del entregable se puede ver un archivo Makefile, donde el usuario deberá situarse en el directorio del mismo para luego correr los comandos make clean y make all. Al ejecutar este último, se crearán 3 archivos: application, worker y view; los cuales se pueden ejecutar de 3 formas distintas:

1).

` ./application path `

Se ejecutará el proceso padre enviando los archivos dentro de path. No se mostrará ningún resultado por consola, en cambio estos resultados se podrán ver dentro del archivo output_application como se mencionó anteriormente ejecutando el comando cat output_application lo veremos en consola.

2).

` ./application path | ./view `

Realiza lo mismo que en el caso anterior (ejecutar el proceso padre) y ejecutando el proceso view, que se conectará a un espacio de memoria compartida para leer los resultados del proceso application y mostrarlos por consola a medida que se resuelvan los archivos.

3).

` ./worker [ENTER] `
` file [CTRL+D] `

Ejecuta el proceso hijo/esclavo y procesa este file/s que se envíe/n dentro de la ejecución. El [CTRL+D] corresponde al EOF de la shell para terminar la ejecución del proceso. 