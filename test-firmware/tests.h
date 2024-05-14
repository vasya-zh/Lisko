#pragma once

#include "main.h"
#include "epd27.h"
#include "psram.h"
#include "utils.h"
#include <em_gpcrc.h>
#include "AT25.h"
#include "NfcDecoder.h"
#include "qrcode.h"

typedef struct {
	char **array;
	int width, row, col;
} Shape;

void DispTest();
bool RamTest();
bool RomTest();

void drawSierpinski(float x1, float y1, float x2, float y2, float x3, float y3); 
void subTriangle(int n, float x1, float y1, float x2, float y2, float x3, float y3); 
void Game();
void PrintTable();
void SetNewRandomShape();
void DeleteShape(Shape shape);
void ManipulateCurrent(int action);
void SensTest();
void NfcTest();
void FreqVoltTest();
void Demo();





