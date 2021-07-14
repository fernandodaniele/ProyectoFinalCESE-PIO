#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void iniciarFlash (void);
int leerFlash (const char *valorNombre, int64_t *valorLeido);
int guardarFlash(const char *valorNombre, int64_t valorGuardar);

#endif