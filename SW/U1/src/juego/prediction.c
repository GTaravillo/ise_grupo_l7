#include "prediction.h"
//#include "tablero.h"
#include <string.h>

void predictPosition(AJD_TableroPtr tablero, AJD_CasillaPtr origen, uint8_t predict[64]);
void predPeon(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero);
void predCaballo(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero);
void predDiagonal(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero);
void predVertHorz(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero);
void predRey(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero);
void vectorProcess(uint8_t pos[2], uint8_t vector[2], uint8_t pred[4], uint8_t block[4]);
void predProcess(uint8_t pred[4], uint8_t predict[64]);

void predictPosition(AJD_TableroPtr tablero, AJD_CasillaPtr origen, uint8_t predict[64]){
	AJD_Pieza pieza = origen->pieza;
	memset(predict,0,8*8*sizeof(uint8_t));
	switch (pieza){
		case PEON1...PEON8:
			predPeon(predict, origen, tablero);
			break;
		case CABALLO1:
		case CABALLO2:
			predCaballo(predict, origen, tablero);
			break;
		case ALFIL1...ALFIL2:
			predDiagonal(predict, origen, tablero);
			break;
		case TORRE1...TORRE2:
			predVertHorz(predict, origen, tablero);
			break;
		case DAMA:
			predDiagonal(predict, origen, tablero);
			predVertHorz(predict, origen, tablero);
			break;
		case REY:
			predRey(predict, origen, tablero);
			break;
		default:
			predict[0] = 255;
			break;
	}
}

void predPeon(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero){
	AJD_idCasilla id = origin->id;
	uint8_t i = 0;
	if(origin->color_pieza == NEGRO){
		if(id < 16 && id >= 8){ //|| (origin->color_pieza == BLANCO && id < 6*8+8 && id >= 6*8)){
			for(i = 0; i<2; i++){
				if(tablero->casilla[id+8*(i+1)].pieza == NONE){
					predict[id+8*(i+1)] = 1;
				}else{
					break;
				}
			}
		}else{
			if(tablero->casilla[id+8].pieza == NONE){
				predict[id+8] = 1;
			}
		}
		predict[id+7] = (tablero->casilla[id+7].pieza != NONE) && (id%8 != 0) ? 1 : 0;
		predict[id+9] = (tablero->casilla[id+9].pieza != NONE) && (id%8 != 7) ? 1 : 0;

	}else if(origin->color_pieza == BLANCO){
		if(id < 6*8+8 && id >= 6*8){
			for(i = 0; i<2; i++){
				if(tablero->casilla[id-8*(i+1)].pieza == NONE){
					predict[id-8*(i+1)] = 1;
				}else{
					break;
				}
			}
		}else{
			if(tablero->casilla[id-8].pieza == NONE){
				predict[id-8] = 1;
			}
		}
		predict[id-7] = (tablero->casilla[id-7].pieza != NONE) && (id%8 != 0) ? 1 : 0;
		predict[id-9] = (tablero->casilla[id-9].pieza != NONE) && (id%8 != 7) ? 1 : 0;
	}
}

void predCaballo(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero){
	uint8_t vector[2] = {1, 2};
	uint8_t pred[4];
	uint8_t block[4] = {0, 0, 0, 0};
	//uint8_t result[8];
	AJD_idCasilla id = origin->id;
	uint8_t pos[2] = {origin->id % 8, origin->id / 8};
	//uint8_t i;
	vectorProcess(pos, vector, pred, block);
	predProcess(pred, predict);
	vector[0] = 2;
	vector[1] = 1;
	memset(block, 0, 4*sizeof(uint8_t));
	vectorProcess(pos, vector, pred, block);
	predProcess(pred, predict);
	
}

void predDiagonal(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero){
	uint8_t vector[2] = {1, 1};
	uint8_t pred[4];
	uint8_t block[4] = {0, 0, 0, 0};
	//uint8_t result[8];
	AJD_idCasilla id = origin->id;
	uint8_t pos[2] = {origin->id % 8, origin->id / 8};
	while(pos[0] + vector[0] < 8 || pos[1] + vector[1] < 8 || pos[0] - vector[0] >= 0 || pos[1] - vector[1] >= 0){
		vectorProcess(pos, vector, pred, block);
		predProcess(pred, predict);
		for(int i = 0; i < 4; i++){
			if(pred[i] != 255){
				block[i] = tablero->casilla[pred[i]].pieza != NONE ? 1 : block[i];
			}
		}
		vector[0]++;
		vector[1]++;
	}
}

void predVertHorz(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero){
	uint8_t vector[2] = {1, 0};
	uint8_t pred[4];
	uint8_t block[4] = {0, 0, 0, 0};
	uint8_t i;
	AJD_idCasilla id = origin->id;
	uint8_t pos[2] = {origin->id % 8, origin->id / 8};
	while(((pos[0] + vector[0]) < 8) || ((pos[0] - vector[0]) >= 0)){
		vectorProcess(pos, vector, pred, block);
		predProcess(pred, predict);
		for(i = 0; i < 4; i++){
			if(pred[i] != 255){
				block[i] = tablero->casilla[pred[i]].pieza != NONE ? 1 : block[i];
			}
		}
		vector[0]++;
	}
	vector[0] = 0;
	vector[1] = 1;
	memset(block, 0, 4*sizeof(uint8_t));
	while(((pos[1] + vector[1]) < 8) || ((pos[1] - vector[1]) >= 0)){
		vectorProcess(pos, vector, pred, block);
		predProcess(pred, predict);
		for(i = 0; i < 4; i++){
			if(pred[i] != 255){
				block[i] = tablero->casilla[pred[i]].pieza != NONE ? 1 : block[i];
			}
		}
		vector[1]++;
	}
}
void predRey(uint8_t predict[64], AJD_CasillaPtr origin, AJD_TableroPtr tablero){
	uint8_t vector[2] = {1, 0};
	uint8_t pred[4];
	uint8_t block[4] = {0, 0, 0, 0};
	uint8_t i;
	AJD_idCasilla id = origin->id;
	uint8_t pos[2] = {origin->id % 8, origin->id / 8};
	vectorProcess(pos, vector, pred, block);
	predProcess(pred, predict);
	vector[1]++;
	memset(block, 0, 4*sizeof(uint8_t));
	vectorProcess(pos, vector, pred, block);
	predProcess(pred, predict);
	vector[0]--;
	memset(block, 0, 4*sizeof(uint8_t));
	vectorProcess(pos, vector, pred, block);
	predProcess(pred, predict);
	
}
void vectorProcess(uint8_t pos[2], uint8_t vector[2], uint8_t pred[4], uint8_t block[4]){
	uint8_t i;
	int8_t resposx;
	int8_t resposy;
	int8_t cz[8] = {+1, +1, +1, -1, -1, +1, -1, -1};
	for(i = 0; i < 4; i++){
		resposx = (int8_t)pos[0]+vector[0]*cz[i*2];
		resposy = (int8_t)pos[1]+vector[1]*cz[i*2+1];
		if(resposx<8 && resposy<8 && resposx>=0 && resposy>=0 && block[i] == 0){
			pred[i] = (uint8_t)(resposy*8 + resposx);
		}else{
			pred[i] = 255;
		}
	}
}

void predProcess(uint8_t pred[4], uint8_t predict[64]){
	uint8_t i;
	for(i = 0; i < 4; i++){
		predict[pred[i]] = 1 ? pred[i] != 255 : 0;
	}
}