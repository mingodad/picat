MFLAGS=-DM64BITS
#MFLAGS=-DM32BITS
#MFLAGS=-m32 -DM32BITS

#CPP    = clang-16-env clang++ -DGC -DGCC -DPICAT -DSAT -DMAPLESAT $(MFLAGS) -DFANN_NO_DLL -DFANN
#CC     = clang-16-env clang -std=c99 -D_XOPEN_SOURCE=500 -DNSIG=256 -DGC -DGCC -DPICAT -DSAT -DMAPLESAT $(MFLAGS) -DFANN_NO_DLL -DFANN

#CPP    = emsdk-env emcc -DGC -DGCC -DPICAT -DSAT -DMAPLESAT $(MFLAGS) -DFANN_NO_DLL -DFANN
#CC     = emsdk-env emcc -DGC -DGCC -DPICAT -DSAT -DMAPLESAT $(MFLAGS) -DFANN_NO_DLL -DFANN

CPP    = g++ -DGC -DGCC -DPICAT -DSAT -DMAPLESAT $(MFLAGS) -DFANN_NO_DLL -DFANN
CC     = gcc -DGC -DGCC -DPICAT -DSAT -DMAPLESAT $(MFLAGS) -DFANN_NO_DLL -DFANN

#CC     = cc

#CFLAGS = -O3 -Dunix -DGC -DBSD             #SunOS
CFLAGS = $(MFLAGS) -O3 -fno-strict-aliasing -finline-functions -fomit-frame-pointer -Dunix -DLINUX -DPOSIX #Linux,#HP
#CFLAGS = -g -DNOINLINE -fno-strict-aliasing -Dunix -DLINUX -DPOSIX #Linux,#HP
#CFLAGS = -O3 -Dunix -DGC -DPOSIX           #HP
#CFLAGS = -O3 -Dunix -DGC                    #Solaris,SonyNews
#CFLAGS = -O3 -DWIN32 -DGC                  #Windows-PC
#CFLAGS = -O3 -DDARWIN                      #MacOS X, Darwin

#LGLFLAG = -Wall -DNLGLOG -DNLGLSTATS -DNDEBUG -DNLGLPICOSAT
#ESPRESSO_FLAGS = $(MFLAGS) -O3 -I. -Iespresso
#KISSAT_FLAGS = $(MFLAGS) -O3 -W -Wall -DNEMBEDDED -DNDEBUG -DNMETRICS -DQUIET -DNSTATISTICS  -DNPROOFS

LFLAGS = $(MFLAGS) -lm  -lpthread

include common.mak
