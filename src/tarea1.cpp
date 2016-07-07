#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
using namespace std;

typedef struct bmpFileHeader
{
  /* 2 bytes de identificación */
  unsigned int size;        /* Tamaño del archivo */
  unsigned short int resv1;       /* Reservado */
  unsigned short int resv2;       /* Reservado */
  unsigned int offset;      /* Offset hasta hasta los datos de imagen */
} bmpFileHeader;

typedef struct bmpInfoHeader
{
  unsigned int headersize;      /* Tamaño de la cabecera */
  unsigned int width;               /* Ancho */
  unsigned int height;          /* Alto */
  unsigned short int planes;                  /* Planos de color (Siempre 1) */
  unsigned short int bpp;             /* bits por pixel */
  unsigned int compress;        /* compresión */
  unsigned int imgsize;     /* tamaño de los datos de imagen */
  unsigned int bpmx;                /* Resolución X en bits por metro */
  unsigned int bpmy;                /* Resolución Y en bits por metro */
  unsigned int colors;              /* colors used en la paleta */
  unsigned int imxtcolors;      /* Colores importantes. 0 si son todos */
} bmpInfoHeader;


unsigned char* negative (unsigned short type,bmpFileHeader*header,bmpInfoHeader*bInfoHeader,unsigned char*imgdata, unsigned char* paleta,unsigned char* newPaleta )
{
	unsigned char * imgNegative=imgdata;
	newPaleta = paleta;

  if (paleta==NULL)
  {

		unsigned int iSize=bInfoHeader->imgsize;
		imgNegative=(unsigned char *) malloc(iSize);
		for (unsigned int i=0;i<iSize;i++)
		{
			imgNegative[i]=255 - imgdata[i];
		}
	}
	else
	{
		unsigned int tamPaleta = (1<<bInfoHeader->bpp)*4; //2^n
		newPaleta=(unsigned char *)malloc(tamPaleta);
		for (unsigned int i=0;i<tamPaleta;i++)
		{
			newPaleta[i]=255 - paleta[i];
		}

	}
	return imgNegative;

}

unsigned char* rotarMatrizCW (unsigned short type,bmpFileHeader*header,bmpInfoHeader*bInfoHeader,unsigned char *imgdata, unsigned char* paleta)
{
	unsigned char *imgRotada=NULL;
	unsigned int tamPaleta=0;

	unsigned int bpp = bInfoHeader->bpp;
	unsigned int columnas = bInfoHeader->width;
	unsigned int filas = bInfoHeader->height;

	unsigned int filaPadding = filas;
	unsigned int newFilas=filas*bpp;

	if (newFilas%32!=0)
	{
		newFilas+=32-newFilas%32;
	}
	filaPadding = newFilas/8;

	newFilas=newFilas/bpp;


	if (paleta!=NULL)
		tamPaleta = (1<<bInfoHeader->bpp)<<2;

		bInfoHeader->imgsize=filaPadding*columnas;
		header->size=54+tamPaleta+bInfoHeader->imgsize;
		imgRotada=(unsigned char *)calloc(filaPadding,columnas);


		unsigned int aux =bInfoHeader->width;
		bInfoHeader->width=bInfoHeader->height;
		bInfoHeader->height=aux;


		if (bpp<=8)
		{
			unsigned char bitMask=0xFF << (8-bpp); //mascara de imagen sin rotar
			unsigned char bitMAux=bitMask; //variable que usare para modificar el valor de bitMask
			unsigned int bytePartition = 8/bpp;
			unsigned int k=0;
			aux=(columnas*bpp);
			if (aux%32!=0)
			{

				aux=aux+32-aux%32;
			}
			else
			{

			}
			aux=aux/8;

			for (unsigned int i=0;i<columnas;i++)
			{
				for (unsigned int j=0;j<filas;j++)
				{

					imgRotada[i*filaPadding + j/bytePartition]|= imgdata[ (filas-j-1)*aux + /*aux-*/(i/bytePartition)/*-1*/ ] & bitMAux;
				}
				k++;
				if (k==bytePartition)
				{
					bitMAux=0xFF << (8-bpp);
					k=0;
				}
				else
				{
					bitMAux=bitMAux>>bpp;

				}

			}

		}
		else
		{
			aux=(columnas*bpp);
			if (aux%32!=0)
			{
				aux=aux+32-aux%32;
			}
			aux=aux/8;
			unsigned int numBytes = bpp/8;
			for (unsigned int i=0;i<columnas;i++)
			{
				for (unsigned int j=0;j<filas;j++)
				{

					for (unsigned int k=numBytes;k>0;k--)
					{

						imgRotada[i*filaPadding + (j*numBytes) + numBytes-k]=imgdata[(filas-j-1)*aux + i*numBytes + numBytes-k];

					}
				}
			}
		}

	return imgRotada;

}



unsigned char* espejoHorizontal (unsigned short type,bmpFileHeader*header,bmpInfoHeader*bInfoHeader,unsigned char *imgdata, unsigned char* paleta)
{
	unsigned int columnas = bInfoHeader->width;
	unsigned int filas = bInfoHeader->height;
	unsigned int bpp=bInfoHeader->bpp;
	unsigned int filaPadding= columnas*bpp;

	if (filaPadding%32!=0)
	{
		filaPadding+=32-filaPadding%32;
	}
	filaPadding/=8;
	unsigned char*newImg = (unsigned char*) calloc(filaPadding,columnas);
	columnas = bInfoHeader->width*bpp/8;

	for (unsigned int i=0;i<filas;i++)
	{
		for (unsigned int j=0;j<columnas;j++)
		{
			newImg[i*filaPadding + j]=imgdata[(filas-i-1)*filaPadding + j];
		}
	}

	return newImg;
}

unsigned char* espejoVertical (unsigned short type,bmpFileHeader*header,bmpInfoHeader*bInfoHeader,unsigned char *imgdata, unsigned char* paleta)
{
	unsigned int columnas = bInfoHeader->width;
	unsigned int filas = bInfoHeader->height;
	unsigned int bpp =bInfoHeader->bpp;
	unsigned int filaPadding= columnas*bpp;

	if (filaPadding%32!=0)
	{
		filaPadding+=32-filaPadding%32;
	}
	filaPadding=filaPadding/8;
	unsigned char* newImg= (unsigned char*) calloc(filaPadding,columnas);


	if (bpp<=8)
	{
		unsigned int bytePartition = 8/bpp;
		unsigned int aux = (columnas*bpp)/8;
		unsigned int k=0;
		unsigned char bitMask= 0xFF >> (8-bpp);
		unsigned char maskAux='\0';
		for (unsigned int i=0;i<filas;i++)
		{
			for (unsigned int j=0;j<columnas;j++)
			{

				if (j%bytePartition>=bytePartition/2)
				{
					maskAux=(imgdata[i*filaPadding + aux - j/bytePartition-1] & bitMask)>>(j%bytePartition - (bytePartition-(j%bytePartition)-1))*bpp;
				}
				else
				{
                    maskAux=(imgdata[i*filaPadding + aux - j/bytePartition-1] & bitMask)<<(8-bpp*((j%bytePartition)+1) - j%bytePartition);
				}
				newImg[i*filaPadding + j/bytePartition]|=maskAux;
				k++;
				if (k==bytePartition)
				{
					bitMask=0xFF >> (8-bpp);
					k=0;
				}
				else
				{
					bitMask=bitMask<<bpp;

				}
			}

		}
	}
	else
	{
		unsigned int numBytes=bpp/8;
		unsigned int aux = (columnas*bpp)/8;
		for (unsigned int i=0;i<filas;i++)
		{
			for (unsigned int j=0;j<columnas;j++)
			{
				for (unsigned int k=0;k<numBytes;k++)
				{
					newImg[i*filaPadding+(j*numBytes)+k]=imgdata[i*filaPadding + aux-(j*numBytes) -numBytes+k];
				}
			}
		}


	}



	return newImg;
}

unsigned char *LoadBMP(char *filename, bmpInfoHeader *bInfoHeader, int opc, int sentido, int numRotaciones)
{

	//adicional
	char salida[100]="salida.bmp";

	//f adicional

  FILE *f;
  bmpFileHeader header;     /* cabecera */
  unsigned int tamPaleta;
  unsigned char * paleta=NULL; /*datos de la paleta*/
  unsigned char *imgdata;   /* datos de imagen */
  unsigned char *imgRotada=NULL;
  unsigned char *newPaleta=NULL;
  unsigned short type;        /* 2 bytes identificativos */

  f=fopen (filename, "r");
  if (!f)
    return NULL;        /* Si no podemos leer, no hay imagen*/

  /* Leemos los dos primeros bytes */
  fread(&type, sizeof(short), 1, f);
  if (type !=0x4D42)        /* Comprobamos el formato */
    {
      fclose(f);
      return NULL;
    }


  /* Leemos la cabecera de fichero completa */
  fread(&header, sizeof(bmpFileHeader), 1, f);



  /* Leemos la cabecera de información completa */
  fread(bInfoHeader, sizeof(bmpInfoHeader), 1, f);


	if (bInfoHeader->bpp != 24)
	{
		tamPaleta = (1<< (bInfoHeader->bpp))*4; //se tienen 2^n colores de 3bytes cada
																												//uno
		paleta = (unsigned char*)malloc (tamPaleta*sizeof(char));
		newPaleta= (unsigned char*)malloc (tamPaleta*sizeof(char));
		fread(paleta,tamPaleta,1,f);
		newPaleta=paleta;
	}

  /* Reservamos memoria para la imagen, ¿cuánta?
     Tanto como indique imgsize */
  imgdata=(unsigned char*)malloc((bInfoHeader->imgsize)*sizeof(char));

  /* Nos situamos en el sitio donde empiezan los datos de imagen,
   nos lo indica el offset de la cabecera de fichero*/
  fseek(f, header.offset, SEEK_SET);


  /* Leemos los datos de imagen, tantos bytes como imgsize */
  fread(imgdata, bInfoHeader->imgsize,1, f);
  //fread(imgdata, tamFila,bInfoHeader->height, f);

  /* Cerramos */
  fclose(f);



		if (opc==0)
		{
			imgRotada=negative(type,&header,bInfoHeader,imgdata,paleta,newPaleta);
		}
		else if (opc==1)
		{
			if (sentido==0)
			{
				imgRotada=rotarMatrizCW(type,&header,bInfoHeader,imgdata,paleta);
				for (int i=0;i<3-numRotaciones;i++)
				{
					imgRotada=rotarMatrizCW(type,&header,bInfoHeader,imgRotada,paleta);

				}
			}
			else if (sentido==1)
			{
				imgRotada=rotarMatrizCW(type,&header,bInfoHeader,imgdata,paleta);
				for (int i=0;i<numRotaciones-1;++i)
				{
					imgRotada=rotarMatrizCW(type,&header,bInfoHeader,imgRotada,paleta);

				}
			}
		}
		else if (opc==2)
		{
			imgRotada=espejoHorizontal(type,&header,bInfoHeader,imgdata,paleta);
		}
		else if (opc==3)
		{
			imgRotada=espejoVertical(type,&header,bInfoHeader,imgdata,paleta);
		}
		else
		{
			printf ("opcion invalida\n");
		}

	f=fopen(salida,"wb");
	fwrite(&type,2,1,f);
  fwrite(&header,sizeof(bmpFileHeader),1,f);
  fwrite(bInfoHeader,sizeof(bmpInfoHeader),1,f);
  if (paleta!=NULL)
  {
		fwrite(newPaleta,tamPaleta,1,f);
	}
	fwrite(imgRotada,bInfoHeader->imgsize,1,f);

	fclose(f);
  /* Devolvemos la imagen */

  return imgdata;
}



int main() {

	bmpInfoHeader *bInfoHeader = (bmpInfoHeader*) malloc(1*sizeof(bmpInfoHeader));
	char filename[200]="lagochungara.bmp";
	int opc=-1;
	int sentido=-1;
	int numRotaciones=0;
	do
	{
		printf ("Ingrese el nombre de archivo.bmp\n");
		scanf("%s",filename);
		printf ("indique que opcion que desea realizar: 0 para convertir a negativo, 1 para rotar la imagen\n");
		printf ("2 para espejo horizontal, 3 para espejo vertical\n");
		scanf("%d",&opc);
		if (opc==1)
		{
			printf ("Indique en que sentido (0: horario, 1:antihorario) y cuantos grados desea rotar la imagen: ");
			printf ("90 180 o 270 grados ");
			printf ("separados por un espacio\n");
			scanf("%d%d",&sentido,&numRotaciones);
			numRotaciones=numRotaciones/90;
		}

		LoadBMP (filename,bInfoHeader,opc,sentido,numRotaciones);
		printf ("desea continuar? 0:si, 1:no\n");
		scanf("%d",&opc);
		if (opc==1)
			break;
	}
	while (opc!=-1);

	free (bInfoHeader);




}
