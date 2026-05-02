srec_cat.exe accelX -Binary -Crop 0x00 0x7568 -o accelX.c -C-Array accelX -INClude -Output_Word
srec_cat.exe accelY -Binary -Crop 0x00 0x7568 -o accelY.c -C-Array accelY -INClude -Output_Word
srec_cat.exe accelZ -Binary -Crop 0x00 0x7568 -o accelZ.c -C-Array accelZ -INClude -Output_Word

srec_cat.exe gyroX -Binary -Crop 0x00 0x7568 -o gyroX.c -C-Array gyroX -INClude -Output_Word
srec_cat.exe gyroY -Binary -Crop 0x00 0x7568 -o gyroY.c -C-Array gyroY -INClude -Output_Word
srec_cat.exe gyroZ -Binary -Crop 0x00 0x7568 -o gyroZ.c -C-Array gyroZ -INClude -Output_Word
