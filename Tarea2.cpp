
// Program to read the file into an array line by line.
 
//Declare the necessary header files

#include  <stdio.h>
#include  <stdlib.h> 
#include  <iostream>
#include  <fstream>
#include  <string>
#include  <sstream>
#include  <thread>

#define tamano 100000 
 
using namespace std;

void hacerPing (std::string buffer){
	system (buffer.c_str());
}

int main(int argc, char *argv[])
{
    thread threads [argc-1];

    string archivo = argv[1];   // Nombre del archivo
    string paquete = argv[2];   // Cantidad de paquetes a enviar
    string linea;   // Leer cada linea del archivo
    int i=0;    // Variable para contar las lineas
    string ips[tamano];  // array para guardar cada linea
    ifstream mFile (archivo);   
    if(mFile.is_open()) 
    {
        while(!mFile.eof())
        {
            getline(mFile, linea);
            ips[i]=linea;
            i++;
        }
        mFile.close();
    }
    else
        cout<<"No fue posible abrir el archivo.\n";
        
        for(int j=0;j<i-1;j++)
    {
        string buffer("ping "); 
    	buffer +=ips[j];
    	buffer +=" -c ";
    	buffer +=paquete;
    	buffer +=" -q";
	hacerPing(buffer);
    }
    return 0;
}
