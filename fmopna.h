#pragma once


typedef struct {

    int tm_w1;
    int tm_w2;
    int tm_w3;
    int tm_w4;
    int tm_w5;
    int tm_w6;
    int tm_w7;
} fmopna_t;

void FMOPNA_Clock(fmopna_t *chip, int clk);
