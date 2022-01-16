
#define NUMAXPROCS 50
#define MAXPOSICIONES 50

#include <stdio.h>
#include <Windows.h>
#include "cruce.h"

//prototipos
DWORD WINAPI HiloGestorSemaforico(LPVOID parAmetro);
DWORD WINAPI HiloPeaton(LPVOID parAmetro);
DWORD WINAPI HiloCoche(LPVOID parAmetro);
void funcionSalir();

//tipo proceso
typedef struct tipoProceso {
	int ocupado;
	HANDLE manejador;
}tipoProceso;

//tipos funciones dll
typedef int (*TipoCruceInicio)(int, int);
typedef int(*TipoCruceFin)(void);
typedef int(*TipoCruceGestorInicio)(void);
typedef int (*TipoCrucePonSemaforo)(int, int);
typedef int(*TipoCruceNuevoProceso)(void);
typedef struct posiciOn(*TipoCruceInicioCoche)(void);
typedef struct posiciOn(*TipoCruceAvanzarCoche)(struct posiciOn);
typedef int (*TipoCruceFinCoche)(void);
typedef struct posiciOn(*TipoCruceNuevoInicioPeaton)(void);
typedef struct posiciOn(*TipoCruceAvanzarPeaton)(struct posiciOn);
typedef int (*TipoCruceFinPeaton)(void);
typedef int (*TipoPausa)(void);
typedef int(*TipoPausaCoche)(void);
typedef int(*TipoRefrescar)(void);
typedef void(*TipoPonError)(const char*);


//variables globales
HANDLE manejadorGestor;

//semaforos, mutexes, eventos
HANDLE semProcs;
HANDLE semC1;
HANDLE semC2;
HANDLE semP1;
HANDLE semP2;

HANDLE semC1A;
HANDLE semC2A;

HANDLE mutexeCruc;
HANDLE posiciones[MAXPOSICIONES][MAXPOSICIONES];
HANDLE posicionesEvent[MAXPOSICIONES][MAXPOSICIONES];
HANDLE mutexeVector;

//vector de procesos
tipoProceso vectorProcesos[NUMAXPROCS];

//punteros a funcion
TipoCruceInicio funcionCruceInicio;
TipoCruceFin CruceFin;
TipoCruceGestorInicio CruceGestorInicio;
TipoCrucePonSemaforo CrucePonSemaforo;
TipoCruceNuevoProceso CruceNuevoProceso;
TipoCruceInicioCoche CruceInicioCoche;
TipoCruceAvanzarCoche CruceAvanzarCoche;
TipoCruceFinCoche CruceFinCoche;
TipoCruceNuevoInicioPeaton CruceNuevoInicioPeaton;
TipoCruceAvanzarPeaton CruceAvanzarPeaton;
TipoCruceFinPeaton CruceFinPeaton;
TipoPausa pausa;
TipoPausaCoche PausaCoche;
TipoRefrescar Refrescar;
TipoPonError ponError;
HMODULE manejadorLibreria;

int main(int argc, char** argv)
{
	int i = 0;
	int j = 0;
	//punteros a funcion


	//Handles

	//Id hilos
	DWORD gestorId;

	int velocidad;
	int numProcs;

	/*******************************
	Verificacion parametros
	*******************************/
	if (argc != 3) {
		printf("numero de parametros incorrecto");
		return -1;
	}
	numProcs = atoi(argv[1]);
	velocidad = atoi(argv[2]);
	if (numProcs > NUMAXPROCS) {
		printf("numero excesivo de procesos");
		exit(2);
	}
	if (numProcs <= 2) {
		printf("numero insuficiente de processos");
		exit(2);
	}


	if (NULL == (manejadorLibreria = LoadLibrary(TEXT("cruce2.dll")))) {
		PERROR("Error: LoadLibrary");
		exit(2);
	}

	/********************************
	Asignacion de punteros a funcion
	************************************/

	if ((funcionCruceInicio = (TipoCruceInicio)GetProcAddress(manejadorLibreria, "CRUCE_inicio")) == NULL) {
		PERROR("Error: CRUCE_inicio");
		funcionSalir();
		exit(2);
	}


	if ((CruceFin = (TipoCruceFin)GetProcAddress(manejadorLibreria, "CRUCE_fin")) == NULL) {
		PERROR("Error: CRUCE_fin");
		funcionSalir();
		exit(2);
	}

	if ((CruceGestorInicio = (TipoCruceGestorInicio)GetProcAddress(manejadorLibreria, "CRUCE_gestor_inicio")) == NULL) {
		PERROR("Error: CRUCE_GestorInicio");
		funcionSalir();
		exit(2);
	}

	if ((CrucePonSemaforo = (TipoCrucePonSemaforo)GetProcAddress(manejadorLibreria, "CRUCE_pon_semAforo")) == NULL) {
		PERROR("Error: Cruce_pon_semAforo");
		funcionSalir();
		exit(2);
	}

	if ((CruceNuevoProceso = (TipoCruceNuevoProceso)GetProcAddress(manejadorLibreria, "CRUCE_nuevo_proceso")) == NULL) {
		PERROR("Error: CRUCE_nuevo_proceso");
		funcionSalir();
		exit(2);
	}

	if ((CruceInicioCoche = (TipoCruceInicioCoche)GetProcAddress(manejadorLibreria, "CRUCE_inicio_coche")) == NULL) {
		PERROR("ERROR: Cruce_inicio_coche");
		funcionSalir();
		exit(2);
	}

	if ((CruceAvanzarCoche = (TipoCruceAvanzarCoche)GetProcAddress(manejadorLibreria, "CRUCE_avanzar_coche")) == NULL) {
		PERROR("ERROR: Cruce_avanzar_coche");
		funcionSalir();
		exit(2);
	}

	if ((CruceFinCoche = (TipoCruceFinCoche)GetProcAddress(manejadorLibreria, "CRUCE_fin_coche")) == NULL) {
		PERROR("ERROR: CRUCE_fin_coche");
		funcionSalir();
		exit(2);
	}

	if ((CruceNuevoInicioPeaton = (TipoCruceNuevoInicioPeaton)GetProcAddress(manejadorLibreria, "CRUCE_nuevo_inicio_peatOn")) == NULL) {
		PERROR("ERRROR: CRUCE_inicio_peatOn");
		funcionSalir();
		exit(2);
	}

	if ((CruceAvanzarPeaton = (TipoCruceAvanzarPeaton)GetProcAddress(manejadorLibreria, "CRUCE_avanzar_peatOn")) == NULL) {
		PERROR("ERROR: CRUCE_avanzar_peatOn");
		funcionSalir();
		exit(2);
	}

	if ((CruceFinPeaton = (TipoCruceFinPeaton)GetProcAddress(manejadorLibreria, "CRUCE_fin_peatOn")) == NULL) {
		PERROR("ERROR: CRUCE_fin_peatOn");
		funcionSalir();
		exit(2);
	}

	if ((pausa = (TipoPausa)GetProcAddress(manejadorLibreria, "pausa")) == NULL) {
		PERROR("ERROR: CRUCE_pausa");
		funcionSalir();
		exit(2);
	}

	if ((PausaCoche = (TipoPausaCoche)GetProcAddress(manejadorLibreria, "pausa_coche")) == NULL) {
		PERROR("ERROR: pausa_coche");
		funcionSalir();
		exit(2);
	}

	if ((Refrescar = (TipoRefrescar)GetProcAddress(manejadorLibreria, "refrescar")) == NULL) {
		PERROR("ERROR: refrescar");
		funcionSalir();
		exit(2);
	}

	if ((ponError = (TipoPonError)GetProcAddress(manejadorLibreria, "pon_error")) == NULL) {
		PERROR("Error: pon_error");
		funcionSalir();
		exit(2);
	}

	/********************************
	Creacion de semaforos
	************************************/

	//semaforo que controla el numero maximo de hilos concurrentes
	if (NULL == (semProcs = CreateSemaphore(NULL, numProcs - 2, numProcs - 2, NULL))) {
		PERROR("Error: Creacion semprocs");
		funcionSalir();
		exit(2);
	}



	/********************************
	Creacion de eventos
	************************************/

	if (NULL == (semC1 = CreateEvent(NULL, true, false, NULL))) {
		PERROR("Error: evento semC1");
		funcionSalir();
		exit(2);
	}

	if (NULL == (semC2 = CreateEvent(NULL, true, false, NULL))) {
		PERROR("Error: evento semC2");
		funcionSalir();
		exit(2);
	}

	if (NULL == (semP1 = CreateEvent(NULL, true, true, NULL))) {
		PERROR("Error: evento semP1");
		funcionSalir();
		exit(2);
	}

	if (NULL == (semP2 = CreateEvent(NULL, true, false, NULL))) {
		PERROR("Error: evento semP2");
		funcionSalir();
		exit(2);
	}


	if (NULL == (semC1A = CreateEvent(NULL, true, true, NULL))) {
		PERROR("Error: evento semP2");
		funcionSalir();
		exit(2);
	}


	if (NULL == (semC2A = CreateEvent(NULL, true, true, NULL))) {
		PERROR("Error: evento semP2");
		funcionSalir();
		exit(2);
	}


	/********************************
	Creacion de mutex
	************************************/

	if (NULL == (mutexeCruc = CreateMutex(NULL, false, NULL))) {
		PERROR("Error: creacion mutexCruc");
		funcionSalir();
		exit(2);
	}

	if (NULL == (mutexeVector = CreateMutex(NULL, false, NULL))) {
		PERROR("Error: creacion mutexVector");
		funcionSalir();
		exit(2);
	}

	//creacion del array de mutexes

	for (i = 0; i < MAXPOSICIONES; i++) {
		for (j = 0; j < MAXPOSICIONES; j++) {

			if (NULL == (posiciones[i][j] = CreateMutex(NULL, false, NULL))) {
				PERROR("Error array mutexes");
				funcionSalir();
			}
		}
	}

//inicializo el vecto de procesos a 0
	for (i = 0; i < NUMAXPROCS; i++) {
		vectorProcesos[i].manejador = NULL;
		vectorProcesos[i].ocupado = 0;
	}


	/********************************
	INICIO DE LA BIBLIOTECA
	************************************/
	funcionCruceInicio(velocidad, numProcs);

	//creacion del proceso gestor semaforico
	manejadorGestor = CreateThread(NULL, 0, &HiloGestorSemaforico, NULL, 0, &gestorId);

	for (;;) {
		WaitForSingleObject(semProcs, INFINITE);
		if (CruceNuevoProceso() == COCHE) {
			if (NULL == (CreateThread(NULL, 0, &HiloCoche, NULL, 0, NULL))) {
				PERROR("error creando coche");
				funcionSalir();
				exit(2);
			}
		}
		else {
			if (NULL == (CreateThread(NULL, 0, &HiloPeaton, NULL, 0, NULL))) {
				PERROR("Error creando peaton");
				funcionSalir();
				exit(2);
			}
		}
	}
}


/*
FUNCIONES
*/


DWORD WINAPI HiloGestorSemaforico(LPVOID parAmetro) {
	int cont;
	CruceGestorInicio();


	for (;;) {
		CrucePonSemaforo(SEM_C1, VERDE);
		if (!SetEvent(semC1)) {
			PERROR("SetEvent C1");
			funcionSalir();
		}


		CrucePonSemaforo(SEM_C2, ROJO);
		if (!ResetEvent(semC2)) {
			PERROR("ResetEvent C2");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_P1, ROJO);
		if (!ResetEvent(semP1)) {
			PERROR("SetEvent P1");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_P2, VERDE);
		if (!SetEvent(semP2)) {
			PERROR("SetEvent semP2");
			funcionSalir();
		}

		for (cont = 0; cont < 6; cont++) {
			pausa();
		}

		//cambio SemC1 y SemP2

		CrucePonSemaforo(SEM_C1, AMARILLO);
		if (!ResetEvent(semC1)) {
			PERROR("ResetEvent semC1");
			funcionSalir();
		}
		WaitForSingleObject(semC1A, INFINITE);
		for (cont = 0; cont < 2; cont++) {
			pausa();
		}

		CrucePonSemaforo(SEM_C1, ROJO);
		if (!ResetEvent(semC1)) {
			PERROR("ResetEvent semC1");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_C2, VERDE);
		if (!SetEvent(semC2)) {
			PERROR("SetEvent semC2");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_P1, ROJO);
		if (!ResetEvent(semP1)) {
			PERROR("ResetEvent semP1");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_P2, ROJO);
		if (!ResetEvent(semP2)) {
			PERROR("ResetEvent semP2");
			funcionSalir();
		}

		//cambio SEMC2 a amarillo

		for (cont = 0; cont < 9; cont++) {
			pausa();
		}

		CrucePonSemaforo(SEM_C2, AMARILLO);
		if (!ResetEvent(semC2)) {
			PERROR("ResetEvent semc1");
			funcionSalir();
		}
		WaitForSingleObject(semC2A, INFINITE);
		for (cont = 0; cont < 2; cont++) {
			pausa();
		}

		CrucePonSemaforo(SEM_C1, ROJO);
		if (!ResetEvent(semC1)) {
			PERROR("ResetEvent semC1");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_C2, ROJO);


		CrucePonSemaforo(SEM_P1, VERDE);
		if (!SetEvent(semP1)) {
			PERROR("SetEvent semP1");
			funcionSalir();
		}

		CrucePonSemaforo(SEM_P2, ROJO);
		if (!ResetEvent(semP2)) {
			PERROR("ResetEvent semP2");
			funcionSalir();
		}

		for (cont = 0; cont < 12; cont++) {
			pausa();
		}



	}
	return 0;
}

DWORD WINAPI HiloPeaton(LPVOID parAmetro) {
	struct posiciOn posicion;
	struct posiciOn posTemp;
	struct posiciOn posAnterior;
	int contador = 0;
	int indice;

	WaitForSingleObject(mutexeVector, INFINITE);
	for (contador = 0; contador < NUMAXPROCS; contador++) {
		if (vectorProcesos[contador].ocupado == 0) {
			vectorProcesos[contador].ocupado = 1;
			vectorProcesos[contador].manejador = GetCurrentThread();
			break;
		}
	}
	ReleaseMutex(mutexeVector);

	posicion = CruceNuevoInicioPeaton();
	posTemp.x = posTemp.y = 0;

	for (;;) {

		//si me salgo muero
		if (posicion.y < 0) {
			CruceFinPeaton();
			ReleaseSemaphore(semProcs, 1, NULL);
			return 0;
		}

		if (posicion.x == 30) {
			//ponError("estoy en x = 30");
			WaitForSingleObject(semP1, INFINITE);
		}

		if (posicion.y == 11 && posicion.x < 40) {
			WaitForSingleObject(semP2, INFINITE);
		}

		//si estoy cruzando el cruce bloqueo el evento
		if (posicion.y < 11 && posicion.x < 40 && posicion.y > 7) {
			//ponError("Reset event");
			if (!ResetEvent(semC2A)) {
				PERROR("reset event semC2A");
				funcionSalir();
				exit(2);
			}
		}

		//si estoy cruzando el cruce bloqueo el evento
		if (posicion.x > 30 && posicion.x < 40) {
			//ponError("ResetEvent");
			if (!ResetEvent(semC1A)) {
				PERROR("reset event semc1A");
				funcionSalir();
				exit(2);
			}
		}

		//si he terminado el cruce desbloqueo el evento
		if (posicion.x == 41 && posicion.y > 11) {
			//ponError("SetEvent");
			if (!SetEvent(semC1A)) {
				PERROR("set event semC1A");
				funcionSalir();
				exit(2);
			}

		}


		//si he terminado el cruce desbloqueo el evento
		if (posicion.y == 6 && posicion.x < 40) {
			//ponError("set Event");
			if (!SetEvent(semC2A)) {
				PERROR("set event semC2A");
				funcionSalir();
				exit(2);
			}
		}


		WaitForSingleObject(posiciones[posicion.x][posicion.y], INFINITE);
		posAnterior = posTemp;
		posTemp = posicion;
		posicion = CruceAvanzarPeaton(posicion);
		ReleaseMutex(posiciones[posAnterior.x][posAnterior.y]);
		pausa();
	}
	return 0;
}

DWORD WINAPI HiloCoche(LPVOID parAmetro) {
	struct posiciOn posicion;
	struct posiciOn posTemp;
	posTemp.x = posTemp.y = 0;
	struct posiciOn posAnterior = posTemp;
	HANDLE reservas[3];
	HANDLE esperaMultipleC2[2];
	int i;
	int indice, contador = 0;

	esperaMultipleC2[0] = semC2;
	esperaMultipleC2[1] = semC2A;

	WaitForSingleObject(mutexeVector, INFINITE);
	for (contador = 0; contador < NUMAXPROCS; contador++) {
		if (vectorProcesos[contador].ocupado == 0) {
			vectorProcesos[contador].ocupado = 1;
			vectorProcesos[contador].manejador = GetCurrentThread();
			break;
		}
	}
	ReleaseMutex(mutexeVector);

	posicion = CruceInicioCoche();
	for (;;) {

		for (i = 0; i < 3; i++) {
			reservas[i] = posiciones[posicion.x + i][posicion.y];
		}
		WaitForMultipleObjects(3, reservas, true, INFINITE);

		//espero a que el cruce este libre, si lo está lo reservo y espero al semaforo
		if (posicion.x == 13) {
			WaitForSingleObject(mutexeCruc, INFINITE);
			//WaitForSingleObject(semC2,INFINITE);
			//ponError("espero al peaton");
			WaitForMultipleObjects(2, esperaMultipleC2, true, INFINITE);
		}

		//espero a que el cruce este libre, si lo está lo reservo y espero al semaforo
		if (posicion.y == 6) {
			WaitForSingleObject(mutexeCruc, INFINITE);
			WaitForSingleObject(semC1, INFINITE);
		}

		if (posicion.y == 10 && posicion.x > 31) {
			//ponError("estoy en la posicion 10");
			WaitForSingleObject(semC1A, INFINITE);
		}
		//salgo de la zona del cruce
		if (posicion.y == 16) {
			ReleaseMutex(mutexeCruc);
		}

		//el coche muere
		if (posicion.y < 0) {
			CruceFinCoche();
			ReleaseSemaphore(semProcs, 1, NULL);
			return 0;
		}

		//WaitForSingleObject(posiciones[posicion.x][posicion.y], INFINITE);

		posAnterior = posTemp;
		posTemp = posicion;
		posicion = CruceAvanzarCoche(posicion);


		ReleaseMutex(posiciones[posAnterior.x][posAnterior.y]);

		PausaCoche();
	}
	return 0;
}

void funcionSalir() {
	DWORD valorSalida = NULL;
	int cont;

	WaitForSingleObject(mutexeVector,INFINITE);
	for (cont = 0; cont < NUMAXPROCS; cont++) {
		if (vectorProcesos[cont].ocupado == 1) {
			if (vectorProcesos[cont].manejador != NULL) {
				TerminateThread(vectorProcesos[cont].manejador, valorSalida);
			}
		}
		cont++;
	}
	ReleaseMutex(mutexeVector);
	FreeLibrary(manejadorLibreria);
}
