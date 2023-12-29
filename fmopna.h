#pragma once

typedef struct {
    int clk;
    int ic;
    int cs;
    int wr;
    int rd;
    int a0;
    int a1;
    int data;
    int gpio_a;
} fmopna_input_t;

typedef struct {

    fmopna_input_t input;

    int ic;

    int ic_latch1[2];
    int ic_latch2[2];
    int ic_latch3[2];
    int ic_check1;
    int ic_check2;
    int ic_check3;
    int prescaler_latch[2];
    int prescaler_sel[2];
    int pssel_l[15][2];
    int ic_latch_fm[2];

    int dclk;

    int mclk1;
    int mclk2;

    int clk1;
    int clk2;
    int aclk1;
    int aclk2;
    int bclk1;
    int bclk2;
    int cclk1;
    int cclk2;

    int ssg_write0;
    int ssg_write1;

    int write0_trig0;
    int write0_trig1;
    int write0_l[3];
    int write0_en;

    int write1_trig0;
    int write1_trig1;
    int write1_l[3];
    int write1_en;

    int write2_trig0;
    int write2_trig1;
    int write2_l[3];
    int write2_en;

    int write3_trig0;
    int write3_trig1;
    int write3_l[3];
    int write3_en;

    int data_l;

    int data_bus1;
    int data_bus2; // inverted

    int read_bus;

    int o_gpio_a;
    int o_gpio_a_d;
    int o_gpio_b;
    int o_gpio_b_d;

    int addr_10[2];
    int addr_12[2];
    int addr_21[2];
    int addr_22[2];
    int addr_24[2];
    int addr_25[2];
    int addr_26[2];
    int addr_27[2];
    int addr_28[2];
    int addr_29[2];
    int addr_ff[2];

    int reg_test_12[2];
    int reg_test_21[2];
    int reg_lfo[2];
    int reg_timer_a[2];
    int reg_timer_b[2];
    int reg_ch3[2];
    int reg_timer_a_load[2];
    int reg_timer_b_load[2];
    int reg_timer_a_enable[2];
    int reg_timer_b_enable[2];
    int reg_timer_a_reset[2];
    int reg_timer_b_reset[2];
    int reg_sch[2];
    int reg_irq[2];
    int reg_kon_operator[2];
    int reg_kon_channel[2];
    int fm_address[2];
    int fm_data[2];
    int write_fm_address[2];
    int write_fm_data[2]; 

    int fm_is30;
    int fm_is40;
    int fm_is50;
    int fm_is60;
    int fm_is70;
    int fm_is80;
    int fm_is90;
    int fm_isa0;
    int fm_isa4;
    int fm_isa8;
    int fm_isac;
    int fm_isb0;
    int fm_isb4;

    int fsm_cnt1[2];
    int fsm_cnt2[2];
    int fsm_out[22];

    int tm_w1;
    int tm_w2;
    int tm_w3;
    int tm_w4;
    int tm_w5;
    int tm_w6;
    int tm_w7;
} fmopna_t;

void FMOPNA_Clock(fmopna_t *chip, int clk);
