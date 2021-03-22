#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void iniciarFlash (void);
int leerFlash (const char *valorNombre, int16_t *valorLeido);
int guardarFlash(const char *valorNombre, int16_t valorGuardar);

#endif