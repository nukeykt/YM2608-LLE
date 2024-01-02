#include <string.h>
#include "fmopna.h"
#include "fmopna_rom.h"

enum {
    eg_state_attack = 0,
    eg_state_decay,
    eg_state_sustain,
    eg_state_release
};

static const int fm_algorithm[4][6][8] = {
    {
        { 1, 1, 1, 1, 1, 1, 1, 1 }, /* OP1_0         */
        { 1, 1, 1, 1, 1, 1, 1, 1 }, /* OP1_1         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP2           */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 1 }  /* Out           */
    },
    {
        { 0, 1, 0, 0, 0, 1, 0, 0 }, /* OP1_0         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_1         */
        { 1, 1, 1, 0, 0, 0, 0, 0 }, /* OP2           */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 1, 1, 1 }  /* Out           */
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_0         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_1         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP2           */
        { 1, 0, 0, 1, 1, 1, 1, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 1, 1, 1, 1 }  /* Out           */
    },
    {
        { 0, 0, 1, 0, 0, 1, 0, 0 }, /* OP1_0         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_1         */
        { 0, 0, 0, 1, 0, 0, 0, 0 }, /* OP2           */
        { 1, 1, 0, 1, 1, 0, 0, 0 }, /* Last operator */
        { 0, 0, 1, 0, 0, 0, 0, 0 }, /* Last operator */
        { 1, 1, 1, 1, 1, 1, 1, 1 }  /* Out           */
    }
};


void FMOPNA_Clock(fmopna_t *chip, int clk)
{
    int i;

    chip->input.clk = clk;

    chip->mclk1 = !chip->input.clk;
    chip->mclk2 = chip->input.clk;

    chip->ic = !chip->input.ic;


    if (chip->mclk1)
    {
        int prescaler_of = 0;
        int dclk = 1;
        switch (chip->prescaler_sel[1])
        {
            case 2:
                prescaler_of = (chip->prescaler_latch[1] & 0x7ff) == 0;
                dclk = (chip->prescaler_latch[1] & 0xffc) == 0;
                break;
            case 0:
                prescaler_of = (chip->prescaler_latch[1] & 7) == 0;
                dclk = (chip->prescaler_latch[1] & 7) != 0;
                break;
            case 3:
                prescaler_of = (chip->prescaler_latch[1] & 0x1f) == 0;
                dclk = (chip->prescaler_latch[1] & 0x1f) == 0;
                break;

        }
        chip->ic_latch1[0] = chip->ic;

        chip->ic_latch2[0] = (chip->ic_latch2[1] << 1) | chip->ic_latch1[1];
        chip->ic_latch3[0] = (chip->ic_latch3[1] << 1) | chip->ic_check1;

        chip->prescaler_latch[0] = chip->prescaler_latch[1] << 1;
        chip->prescaler_latch[0] |= !chip->ic_check1 && prescaler_of;

        chip->pssel_l[0][0] = (chip->prescaler_latch[1] & 0x861) != 0;
        chip->pssel_l[1][0] = (chip->prescaler_latch[1] & 0x30c) != 0;
        chip->pssel_l[2][0] = (chip->prescaler_latch[1] & 0x12) != 0;
        chip->pssel_l[3][0] = (chip->prescaler_latch[1] & 0x24) != 0;
        chip->pssel_l[4][0] = chip->pssel_l[4][1] << 1;
        chip->pssel_l[4][0] |= (chip->prescaler_latch[1] & 0x5) != 0;
        chip->pssel_l[5][0] = (chip->prescaler_latch[1] & 0x3c) != 0;
        chip->pssel_l[6][0] = (chip->prescaler_latch[1] & 0xf00) != 0;
        chip->pssel_l[7][0] = (chip->prescaler_latch[1] & 0xc) != 0;
        chip->pssel_l[8][0] = (chip->prescaler_latch[1] & 0x21) != 0;
        chip->pssel_l[9][0] = (chip->prescaler_latch[1] & 0x2) != 0;
        chip->pssel_l[10][0] = (chip->prescaler_latch[1] & 0x1) != 0;
        chip->pssel_l[11][0] = (chip->prescaler_latch[1] & 0x924) != 0;
        chip->pssel_l[12][0] = (chip->prescaler_latch[1] & 0x249) != 0;
        chip->pssel_l[13][0] = chip->pssel_l[13][1] << 1;
        chip->pssel_l[13][0] |= (chip->prescaler_latch[1] & 9) == 0;
        chip->pssel_l[14][0] = (chip->prescaler_latch[1] & 0x555) != 0;

        chip->dclk = dclk;

    }
    if (chip->mclk2)
    {
        chip->ic_latch1[1] = chip->ic_latch1[0];
        chip->ic_latch2[1] = chip->ic_latch2[0];

        chip->ic_check1 = chip->ic_latch1[1] && (chip->ic_latch2[1] & 0x20000) == 0;

        chip->ic_latch3[1] = chip->ic_latch3[0];

        chip->ic_check2 = (chip->ic_latch3[1] & 4) != 0;
        chip->ic_check3 = (chip->ic_latch3[1] & 8) != 0;

        chip->prescaler_latch[1] = chip->prescaler_latch[0];

        for (i = 0; i < 15; i++)
            chip->pssel_l[i][1] = pssel_l[i][0];
    }

    if (chip->clk1)
    {
        chip->ic_latch_fm[0] = chip->ic;
    }
    if (chip->clk2)
    {
        chip->ic_latch_fm[1] = chip->ic_latch_fm[0];
    }

    {
        int clk1 = 1;
        int clk2 = 1;
        int aclk1 = 1;
        int aclk2 = 1;
        int bclk1 = 1;
        int bclk2 = 1;
        switch (chip->prescaler_sel_l)
        {
            case 2:
                clk1 = chip->pssel_l[0][1];
                clk2 = chip->pssel_l[1][1];
                aclk1 = chip->pssel_l[5][1];
                aclk2 = chip->pssel_l[6][1];
                bclk1 = chip->pssel_l[11][1];
                bclk2 = chip->pssel_l[12][1];
                break;
            case 0:
                clk1 = (chip->pssel_l[4][1] & 2) != 0 && (chip->pssel_l[4][0] & 4) != 0;
                clk2 = (chip->pssel_l[4][1] & 1) != 0 && (chip->pssel_l[4][0] & 2) != 0;
                aclk1 = chip->pssel_l[9][1];
                aclk2 = chip->pssel_l[10][1];
                bclk1 = !chip->mclk2;
                bclk2 = !chip->mclk1;
                break;
            case 3:
                clk1 = chip->pssel_l[2][1];
                clk2 = chip->pssel_l[3][1];
                aclk1 = chip->pssel_l[7][1];
                aclk2 = chip->pssel_l[8][1];
                bclk1 = ((chip->pssel_l[13][1] & 4) == 0 && (chip->pssel_l[13][0] & 8) != 0)
                    || ((chip->pssel_l[13][1] & 1) == 0 && (chip->pssel_l[13][0] & 2) == 0);
                bclk2 = ((chip->pssel_l[13][1] & 1) == 0 && (chip->pssel_l[13][0] & 2) != 0)
                    || ((chip->pssel_l[13][1] & 2) == 0 && (chip->pssel_l[13][0] & 4) == 0);
                break;

        }


        chip->clk1 = clk1;
        chip->clk2 = clk2;

        chip->bclk1 = bclk1;
        chip->bclk2 = bclk2;

        chip->cclk1 = !chip->pssel_l[14][1];
        chip->cclk2 = chip->pssel_l[14][1];
    }

    {
        int read = !chip->ic && !chip->input.rd && !chip->input.cs;
        int read0 = !chip->ic && !chip->input.rd && !chip->input.cs && !chip->input.a1 && !chip->input.a0;
        int read1 = !chip->ic && !chip->input.rd && !chip->input.cs && !chip->input.a1 && chip->input.a0;
        int read2 = !chip->ic && !chip->input.rd && !chip->input.cs && chip->input.a1 && !chip->input.a0;
        int read3 = !chip->ic && !chip->input.rd && !chip->input.cs && chip->input.a1 && chip->input.a0;
        int write = !chip->input.wr && !chip->input.cs;
        int write2 = !chip->ic && !chip->input.wr && !chip->input.cs && chip->input.a1 && !chip->input.a0;
        int write3 = !chip->ic && !chip->input.wr && !chip->input.cs && chip->input.a1 && chip->input.a0;
        int writeaddr = chip->ic || (!chip->input.wr && !chip->input.cs && !chip->input.a0);
        int writedata = !chip->ic && !chip->input.wr && !chip->input.cs && chip->input.a0;

        chip->ssg_write0 = writeaddr && !chip->input.a1;
        chip->ssg_write1 = (writedata && !chip->input.a1) || chip->ic;

        if (writedata)
            chip->write0_trig0 = 1;
        else if (chip->write0_l[0])
            chip->write0_trig0 = 0;
        if (chip->clk1)
        {
            chip->write0_trig1 = chip->write0_trig0;
            chip->write0_l[1] = chip->write0_l[0];
        }
        if (chip->clk2)
        {
            chip->write0_l[0] = chip->write0_trig1;
            chip->write0_l[2] = chip->write0_l[1];
        }
        chip->write0_en = chip->write0_l[0] && !chip->write0_l[2];

        if (writeaddr)
            chip->write1_trig0 = 1;
        else if (chip->write1_l[0])
            chip->write1_trig0 = 0;
        if (chip->clk1)
        {
            chip->write1_trig1 = chip->write1_trig0;
            chip->write1_l[1] = chip->write1_l[0];
        }
        if (chip->clk2)
        {
            chip->write1_l[0] = chip->write1_trig1;
            chip->write1_l[2] = chip->write1_l[1];
        }
        chip->write1_en = chip->write1_l[0] && !chip->write1_l[2];

        if (writeaddr)
            chip->write2_trig0 = 1;
        else if (chip->write2_l[0])
            chip->write2_trig0 = 0;
        if (chip->mclk1)
        {
            chip->write2_trig1 = chip->write2_trig0;
            chip->write2_l[1] = chip->write2_l[0];
        }
        if (chip->mclk2)
        {
            chip->write2_l[0] = chip->write2_trig1;
            chip->write2_l[2] = chip->write2_l[1];
        }
        chip->write2_en = chip->write2_l[0] && !chip->write2_l[2];

        if (writeaddr)
            chip->write3_trig0 = 1;
        else if (chip->write3_l[0])
            chip->write3_trig0 = 0;
        if (chip->tm_w1)
        {
            chip->write3_trig1 = chip->write3_trig0;
            chip->write3_l[1] = chip->write3_l[0];
        }
        if (chip->tm_w2)
        {
            chip->write3_l[0] = chip->write3_trig1;
            chip->write3_l[2] = chip->write3_l[1];
        }
        chip->write3_en = chip->write3_l[0] && !chip->write3_l[2];

        if (write)
            chip->data_l = (chip->input.data & 255) | (chip->input.a1 << 8);

        if (chip->ic)
            chip->data_bus2 |= 0x2f;
        else if (!read && !chip->ic)
            chip->data_bus2 = chip->data_l ^ 0x1ff;

        if (chip->ic_latch_fm[1])
        {
            chip->data_bus1 &= ~255;
        }
        else if (!read && !chip->ic)
        {
            chip->data_bus1 = chip->data_l;
        }
        else if (read1 && chip->addr_ff[1])
        {
            chip->data_bus1 &= ~255;
            chip->data_bus1 |= 1;
        }
        else if (read1 && !chip->tm_w1)
        {
            chip->data_bus1 &= ~255;
            chip->data_bus1 |= chip->input.gpio_a & 255;
        }
        else if (read1 && !chip->tm_w1)
        {
            chip->data_bus1 &= ~255;
            chip->data_bus1 |= chip->input.gpio_a & 255;
        }
        // TODO: more data bus

        if (read1 || read3)
            chip->read_bus = chip->data_bus1;
        // TODO: more read bus


        if (chip->ssg_write1 && !chip->tm_w2)
            chip->o_gpio_a = chip->data_bus1 & 255;
        //else if (!chip->ssg_write1)
        //    chip->o_gpio_a = chip->o_gpio_a;
        chip->o_gpio_a_d = chip->tm_w1;

        if (chip->ssg_write1 && !chip->tm_w2)
            chip->o_gpio_b = chip->data_bus1 & 255;
        //else if (!chip->ssg_write1)
        //    chip->o_gpio_b = chip->o_gpio_b;
        chip->o_gpio_b_d = chip->tm_w1;

#define ADDRESS_MATCH(x) ((chip->data_bus2 & x) == 0 && (chip->data_bus1 & (x^511)) == 0)
        if (chip->mclk1)
        {
            int addr2d = chip->write2_en && ADDRESS_MATCH(0x2d);
            int addr2e = chip->write2_en && ADDRESS_MATCH(0x2e);
            int addr2f = chip->write2_en && ADDRESS_MATCH(0x2f);
            chip->prescaler_sel[0] = chip->prescaler_sel[1];
            if (addr2f)
                chip->prescaler_latch[0] = 0;
            if (chip->ic)
                chip->prescaler_latch[0] = 2;
            if (addr2d)
                chip->prescaler_latch[0] |= 2;
            if (addr2e)
                chip->prescaler_latch[0] |= 1;
        }
        if (chip->mclk2)
        {
            chip->prescaler_sel[1] = chip->prescaler_sel[0];
        }

        if (chip->clk1)
        {
            int is_fm = (chip->data_bus1 & 0xf0) != 0;

            chip->write_fm_address[0] = chip->write0_en ? is_fm : chip->write_fm_address[1];

            if (chip->ic)
                chip->fm_address[0] = 0;
            else if (is_fm && chip->write0_en)
                chip->fm_address[0] = chip->data_bus1;
            else
                chip->fm_address[0] = chip->fm_address[1];

            if (chip->ic)
                chip->fm_data[0] = 0;
            else if (chip->write_fm_address[1] && chip->write1_en)
                chip->fm_data[0] = chip->data_bus1 & 255;
            else
                chip->fm_data[0] = chip->fm_data[1];

            chip->write_fm_data[0] = (chip->write_fm_address[1] && chip->write1_en) || (chip->write_fm_data[1] && !chip->write0_en);

            chip->addr_10[0] = chip->write0_en ? ADDRESS_MATCH(0x110) : chip->addr_10[1];
            chip->addr_12[0] = chip->write0_en ? ADDRESS_MATCH(0x12) : chip->addr_12[1];
            chip->addr_21[0] = chip->write0_en ? ADDRESS_MATCH(0x21) : chip->addr_21[1];
            chip->addr_22[0] = chip->write0_en ? ADDRESS_MATCH(0x22) : chip->addr_22[1];
            chip->addr_24[0] = chip->write0_en ? ADDRESS_MATCH(0x24) : chip->addr_24[1];
            chip->addr_25[0] = chip->write0_en ? ADDRESS_MATCH(0x25) : chip->addr_25[1];
            chip->addr_26[0] = chip->write0_en ? ADDRESS_MATCH(0x26) : chip->addr_26[1];
            chip->addr_27[0] = chip->write0_en ? ADDRESS_MATCH(0x27) : chip->addr_27[1];
            chip->addr_28[0] = chip->write0_en ? ADDRESS_MATCH(0x28) : chip->addr_28[1];
            chip->addr_29[0] = chip->write0_en ? ADDRESS_MATCH(0x29) : chip->addr_29[1];
            chip->addr_ff[0] = chip->write0_en ? ADDRESS_MATCH(0xff) : chip->addr_ff[1];


            int write10 = chip->addr_10[1] && (chip->data_bus1 & 0x100) != 0 && chip->write1_en;
            int irq_rst = write10 && (chip->data_bus2 & 0x80) == 0;

            if (chip->ic)
            {
                chip->reg_test_12[0] = 0;
                chip->reg_test_21[0] = 0;
                chip->reg_lfo[0] = 0;
                chip->reg_timer_a[0] = 0;
                chip->reg_timer_b[0] = 0;
                chip->reg_ch3[0] = 0;
                chip->reg_timer_a_load[0] = 0;
                chip->reg_timer_b_load[0] = 0;
                chip->reg_timer_a_enable[0] = 0;
                chip->reg_timer_b_enable[0] = 0;
                chip->reg_timer_a_reset[0] = 0;
                chip->reg_timer_b_reset[0] = 0;
                chip->reg_kon_operator[0] = 0;
                chip->reg_kon_channel[0] = 0;
                chip->reg_sch[0] = 0;
                chip->reg_irq[0] = 31;
                chip->reg_mask[0] = 28;
            }
            else
            {
                int write10r = write10 && (chip->data_bus2 & 0x80) != 0;
                if (write10r)
                {
                    chip->reg_mask[0] = chip->data_bus1 & 3;
                    chip->reg_mask[0] |= (chip->data_bus2 ^ 28) & 28;
                }
                else
                {
                    chip->reg_mask[0] = chip->reg_mask[0];
                }
                if (chip->addr_12[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_test_12[0] = chip->data_bus1 & 252;
                }
                else
                {
                    chip->reg_test_12[0] = chip->reg_test_12[1];
                }
                if (chip->addr_21[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_test_21[0] = chip->data_bus1 & 255;
                }
                else
                {
                    chip->reg_test_21[0] = chip->reg_test_21[1];
                }
                if (chip->addr_22[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_lfo[0] = chip->data_bus1 & 15;
                }
                else
                {
                    chip->reg_lfo[0] = chip->reg_lfo[1];
                }
                chip->reg_timer_a[0] = 0;
                if (chip->addr_24[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_timer_a[0] |= (chip->data_bus1 & 255) << 2;
                }
                else
                {
                    chip->reg_timer_a[0] |= chip->reg_timer_a[1] & 0x3fc;
                }
                if (chip->addr_25[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_timer_a[0] |= chip->data_bus1 & 3;
                }
                else
                {
                    chip->reg_timer_a[0] |= chip->reg_timer_a[1] & 3;
                }
                if (chip->addr_26[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_timer_b[0] = chip->data_bus1 & 255;
                }
                else
                {
                    chip->reg_timer_b[0] = chip->reg_timer_b[1];
                }
                if (chip->addr_27[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_ch3[0] = (chip->data_bus1 >> 6) & 3;
                    chip->reg_timer_a_load[0] = (chip->data_bus1 >> 0) & 1;
                    chip->reg_timer_b_load[0] = (chip->data_bus1 >> 1) & 1;
                    chip->reg_timer_a_enable[0] = (chip->data_bus1 >> 2) & 1;
                    chip->reg_timer_b_enable[0] = (chip->data_bus1 >> 3) & 1;
                    chip->reg_timer_a_reset[0] = (chip->data_bus1 >> 4) & 1;
                    chip->reg_timer_b_reset[0] = (chip->data_bus1 >> 5) & 1;
                }
                else
                {
                    chip->reg_ch3[0] = chip->reg_ch3[1];
                    chip->reg_timer_a_load[0] = chip->reg_timer_a_load[1];
                    chip->reg_timer_b_load[0] = chip->reg_timer_b_load[1];
                    chip->reg_timer_a_enable[0] = chip->reg_timer_a_enable[1];
                    chip->reg_timer_b_enable[0] = chip->reg_timer_b_enable[1];
                    chip->reg_timer_a_reset[0] = 0;
                    chip->reg_timer_b_reset[0] = 0;
                }
                if (chip->addr_28[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_kon_operator[0] = (chip->data_bus1 >> 4) & 15;
                    chip->reg_kon_channel[0] = (chip->data_bus1 >> 0) & 7;
                }
                else
                {
                    chip->reg_kon_operator[0] = chip->reg_kon_operator[1];
                    chip->reg_kon_channel[0] = chip->reg_kon_channel[1];
                }
                if (chip->addr_29[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_sch[0] = (chip->data_bus1 >> 7) & 1;
                    chip->reg_irq[0] = (chip->data_bus2 & 31) ^ 31;
                }
                else
                {
                    chip->reg_sch[0] = chip->reg_sch[1];
                    chip->reg_irq[0] = chip->reg_irq[1];
                }
            }

            int rst1 = chip->reg_cnt_sync || chip->ic;
            int of = (chip->reg_cnt1[1] & 2) != 0;

            chip->reg_cnt1[0] = (rst1 || of) ? 0 : ((chip->reg_cnt1[1] + 1) & 3);
            chip->reg_cnt2[0] = rst1 ? 0 : ((chip->reg_cnt2[1] + of) & 3);

            int rst2 = chip->reg_cnt_sync || chip->reg_cnt_rss_of;
            chip->reg_cnt_rss[0] = rst2 ? 0 : ((chip->reg_cnt_rss[1] + 1) & 15);

            int of2 = (chip->reg_key_cnt2[1] & 2) != 0;
            chip->reg_key_cnt1[0] = (rst1 || of2) ? 0 : ((chip->reg_key_cnt1[1] + 1) & 3);
            chip->reg_key_cnt2[0] = rst1 ? 0 : ((chip->reg_key_cnt2[1] + of2) & 7);

            chip->reg_kon[0][0] = chip->reg_kon[0][1] << 1;
            chip->reg_kon[1][0] = chip->reg_kon[1][1] << 1;
            chip->reg_kon[2][0] = chip->reg_kon[2][1] << 1;
            chip->reg_kon[3][0] = chip->reg_kon[3][1] << 1;

            if (chip->reg_kon_match)
            {
                chip->reg_kon[0][0] |= (chip->reg_kon_operator[1] >> 0) & 1;
                chip->reg_kon[1][0] |= (chip->reg_kon_operator[1] >> 3) & 1;
                chip->reg_kon[2][0] |= (chip->reg_kon_operator[1] >> 1) & 1;
                chip->reg_kon[3][0] |= (chip->reg_kon_operator[1] >> 2) & 1;
            }
            else
            {
                if (!chip->ic)
                    chip->reg_kon[0][0] |= (chip->reg_kon[3][1] >> 5) & 1;
                chip->reg_kon[1][0] |= (chip->reg_kon[0][1] >> 5) & 1;
                chip->reg_kon[2][0] |= (chip->reg_kon[1][1] >> 5) & 1;
                chip->reg_kon[3][0] |= (chip->reg_kon[2][1] >> 5) & 1;
            }

            chip->reg_sync_timer_l[0] = chip->reg_sync_timer;

            int time = chip->timer_a_cnt[1];
            time += (chip->reg_test_21[1] & 4) != 0 || (chip->timer_a_reg_load && chip->reg_sync_timer);

            chip->timer_a_cnt[0] = chip->timer_a_load ? chip->reg_timer_a[1] : (!chip->timer_a_reg_load ? 0 : (time & 1023));
            chip->timer_a_of[0] = (time & 1024) != 0;

            chip->timer_a_reg_load_l[0] = chip->timer_a_reg_load;

            int rst_a = chip->reg_timer_a_reset[1] || chip->ic;
            int rst_a2 = rst_a || irq_rst || (chip->reg_mask[1] & 1) != 0;

            if (rst_a2)
                chip->timer_a_status[0] = 0;
            else
                chip->timer_a_status[0] = chip->timer_a_status[1];

            chip->timer_a_status[0] |= !rst_a && (chip->reg_mask[1] & 1) == 0
                && chip->reg_timer_a_enable[1] && chip->timer_a_of[1];

            int subcnt = chip->timer_b_subcnt[1] + chip->reg_sync_timer;
            chip->timer_b_subcnt[0] = chip->ic ? 0 : subcnt & 15;
            chip->timer_b_subcnt_of[0] = (subcnt & 16) != 0;

            time = chip->timer_b_cnt[1];
            time += (chip->reg_test_21[1] & 4) != 0 || (chip->timer_b_reg_load && chip->timer_b_subcnt_of[1]);

            chip->timer_b_cnt[0] = chip->timer_b_load ? chip->reg_timer_b[1] : (!chip->timer_b_reg_load ? 0 : (time & 255));
            chip->timer_b_of[0] = (time & 256) != 0;

            chip->timer_b_reg_load_l[0] = chip->timer_b_reg_load;

            int rst_b = chip->reg_timer_b_reset[1] || chip->ic;
            int rst_b2 = rst_b || irq_rst || (chip->reg_mask[1] & 2) != 0;

            if (rst_b2)
                chip->timer_b_status[0] = 0;
            else
                chip->timer_b_status[0] = chip->timer_b_status[1];

            chip->timer_b_status[0] |= !rst_b && (chip->reg_mask[1] & 2) == 0
                && chip->reg_timer_b_enable[1] && chip->timer_b_of[1];

            memcpy(&chip->reg_freq[0][1], &chip->reg_freq[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->reg_freq_3ch[0][1], &chip->reg_freq_3ch[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->reg_connect_fb[0][1], &chip->reg_connect_fb[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->reg_b4[0][1], &chip->reg_b4[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->reg_rss[0][1], &chip->reg_rss[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->op_multi_dt[0][1][0], &chip->op_multi_dt[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_tl[0][1][0], &chip->op_tl[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ar_ks[0][1][0], &chip->op_ar_ks[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_dr_a[0][1][0], &chip->op_dr_a[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_sr[0][1][0], &chip->op_sr[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_rr_sl[0][1][0], &chip->op_rr_sl[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ssg[0][1][0], &chip->op_ssg[1][0][0], 11 * 2 * sizeof(unsigned char));

            if (chip->ic)
            {
                chip->reg_a4[0] = 0;
                chip->reg_freq[0][0] = 0;
                chip->reg_ac[0] = 0;
                chip->reg_freq_3ch[0][0] = 0;
                chip->reg_connect_fb[0][0] = 0;
                chip->reg_b4[0][0] = 0xc0;
                chip->reg_rss[0][0] = 0;

                chip->op_multi_dt[0][0][0] = 0;
                chip->op_multi_dt[0][0][1] = 0;
                chip->op_tl[0][0][0] = 0;
                chip->op_tl[0][0][1] = 0;
                chip->op_ar_ks[0][0][0] = 0;
                chip->op_ar_ks[0][0][1] = 0;
                chip->op_dr_a[0][0][0] = 0;
                chip->op_dr_a[0][0][1] = 0;
                chip->op_sr[0][0][0] = 0;
                chip->op_sr[0][0][1] = 0;
                chip->op_rr_sl[0][0][0] = 0;
                chip->op_rr_sl[0][0][1] = 0;
                chip->op_ssg[0][0][0] = 0;
                chip->op_ssg[0][0][1] = 0;
            }
            else
            {
                chip->reg_a4[0] = chip->fm_isa4 ? (chip->fm_data[1] & 0x3f) : chip->reg_a4[1];
                chip->reg_freq[0][0] = chip->fm_isa0 ? (chip->fm_data[1] & 0xff) | (chip->reg_a4[1] << 8) : chip->reg_freq[1][5];
                chip->reg_ac[0] = chip->fm_isac ? (chip->fm_data[1] & 0x3f) : chip->reg_ac[1];
                chip->reg_freq_3ch[0][0] = chip->fm_isa8 ? (chip->fm_data[1] & 0xff) | (chip->reg_ac[1] << 8) : chip->reg_freq_3ch[1][5];
                chip->reg_connect_fb[0][0] = chip->fm_isb0 ? (chip->fm_data[1] & 0x3f) : chip->reg_connect_fb[1][5];
                chip->reg_b4[0][0] = chip->fm_isb4 ? (chip->fm_data[1] & 0xf7) : chip->reg_b4[1][5];
                chip->reg_rss[0][0] = chip->rss_18 ? (chip->fm_data[1] & 0xdf) : chip->reg_rss[1][5];

                int bank = (chip->fm_address[1] & 8) != 0;

                chip->op_multi_dt[0][0][0] = (chip->fm_is30 && !bank) ? (chip->fm_data[1] & 0x7f) : chip->op_multi_dt[1][5][0];
                chip->op_multi_dt[0][0][1] = (chip->fm_is30 && bank) ? (chip->fm_data[1] & 0x7f) : chip->op_multi_dt[1][5][1];
                chip->op_tl[0][0][0] = (chip->fm_is40 && !bank) ? (chip->fm_data[1] & 0x7f) : chip->op_tl[1][5][0];
                chip->op_tl[0][0][1] = (chip->fm_is40 && bank) ? (chip->fm_data[1] & 0x7f) : chip->op_tl[1][5][1];
                chip->op_ar_ks[0][0][0] = (chip->fm_is50 && !bank) ? (chip->fm_data[1] & 0xdf) : chip->op_ar_ks[1][5][0];
                chip->op_ar_ks[0][0][1] = (chip->fm_is50 && bank) ? (chip->fm_data[1] & 0xdf) : chip->op_ar_ks[1][5][1];
                chip->op_dr_a[0][0][0] = (chip->fm_is60 && !bank) ? (chip->fm_data[1] & 0x9f) : chip->op_dr_a[1][5][0];
                chip->op_dr_a[0][0][1] = (chip->fm_is60 && bank) ? (chip->fm_data[1] & 0x9f) : chip->op_dr_a[1][5][1];
                chip->op_sr[0][0][0] = (chip->fm_is70 && !bank) ? (chip->fm_data[1] & 0x1f) : chip->op_sr[1][5][0];
                chip->op_sr[0][0][1] = (chip->fm_is70 && bank) ? (chip->fm_data[1] & 0x1f) : chip->op_sr[1][5][1];
                chip->op_rr_sl[0][0][0] = (chip->fm_is80 && !bank) ? (chip->fm_data[1] & 0xff) : chip->op_rr_sl[1][5][0];
                chip->op_rr_sl[0][0][1] = (chip->fm_is80 && bank) ? (chip->fm_data[1] & 0xff) : chip->op_rr_sl[1][5][1];
                chip->op_ssg[0][0][0] = (chip->fm_is90 && !bank) ? (chip->fm_data[1] & 0xf) : chip->op_ssg[1][5][0];
                chip->op_ssg[0][0][1] = (chip->fm_is90 && bank) ? (chip->fm_data[1] & 0xf) : chip->op_ssg[1][5][1];
            }

            int rst_cc = chip->ic || chip->ch_cnt_sync;
            int cc_of = (chip->ch_cnt1[1] & 2) != 0;

            chip->ch_cnt1[0] = (rst_cc || cc_of) ? 0 : (chip->ch_cnt1[1] + 1) & 3;
            chip->ch_cnt2[0] = rst_cc ? 0 : (chip->ch_cnt2[1] + cc_of) & 7;

            int cc_cnt = (chip->ch_cnt2[1] << 2) | chip->ch_cnt1[1];


            int freq;
            if (chip->ch3_en && cc_cnt == 1)
                freq = chip->reg_freq_3ch[1][5];
            else if (chip->ch3_en && cc_cnt == 9)
                freq = chip->reg_freq_3ch[1][0];
            else if (chip->ch3_en && cc_cnt == 17)
                freq = chip->reg_freq_3ch[1][4];
            else
                freq = chip->reg_freq[1][4];

            chip->fnum[0] = freq & 0x7ff;
            chip->kcode[0] = ((freq >> 11) & 7) << 2;
            if (freq & 0x400)
            {
                chip->kcode[0] |= 2;
                if ((freq & 0x380) != 0)
                    chip->kcode[0] |= 1;
            }
            else
            {
                if ((freq & 0x380) == 0x380)
                    chip->kcode[0] |= 1;
            }

            chip->fnum[2] = chip->fnum[1];
            chip->kcode[2] = chip->kcode[1];
        }
        if (chip->clk2)
        {
            chip->fm_address[1] = chip->fm_address[0];
            chip->write_fm_address[1] = chip->write_fm_address[0];
            chip->fm_data[1] = chip->fm_data[0];
            chip->write_fm_data[1] = chip->write_fm_data[0];

            chip->addr_10[1] = chip->addr_10[0];
            chip->addr_12[1] = chip->addr_12[0];
            chip->addr_21[1] = chip->addr_21[0];
            chip->addr_22[1] = chip->addr_22[0];
            chip->addr_24[1] = chip->addr_24[0];
            chip->addr_25[1] = chip->addr_25[0];
            chip->addr_26[1] = chip->addr_26[0];
            chip->addr_27[1] = chip->addr_27[0];
            chip->addr_28[1] = chip->addr_28[0];
            chip->addr_29[1] = chip->addr_29[0];
            chip->addr_ff[1] = chip->addr_ff[0];

            chip->reg_mask[1] = chip->reg_mask[0];
            chip->reg_test_21[1] = chip->reg_test_21[0];
            chip->reg_lfo[1] = chip->reg_lfo[0];
            chip->reg_timer_a[1] = chip->reg_timer_a[0];
            chip->reg_timer_b[1] = chip->reg_timer_b[0];
            chip->reg_ch3[1] = chip->reg_ch3[0];
            chip->reg_timer_a_load[1] = chip->reg_timer_a_load[0];
            chip->reg_timer_b_load[1] = chip->reg_timer_b_load[0];
            chip->reg_timer_a_enable[1] = chip->reg_timer_a_enable[0];
            chip->reg_timer_b_enable[1] = chip->reg_timer_b_enable[0];
            chip->reg_timer_a_reset[1] = chip->reg_timer_a_reset[0];
            chip->reg_timer_b_reset[1] = chip->reg_timer_b_reset[0];
            chip->reg_kon_operator[1] = chip->reg_kon_operator[0];
            chip->reg_kon_channel[1] = chip->reg_kon_channel[0];
            chip->reg_sch[1] = chip->reg_sch[0];
            chip->reg_irq[1] = chip->reg_irq[0];

            int op_match = chip->write_fm_data[0] && (chip->reg_cnt1[0] == (chip->fm_address[0] & 3))
                && (chip->reg_cnt2[0] & 1) == ((chip->fm_address[0] >> 8) & 1)
                && ((chip->reg_cnt2[0] >> 1) & 1) == ((chip->fm_address[0] >> 2) & 1);
            int ch_match = chip->write_fm_data[0] && (chip->reg_cnt1[0] == (chip->fm_address[0] & 3))
                && (chip->reg_cnt2[0] & 1) == ((chip->fm_address[0] >> 8) & 1);

            chip->fm_is30 = op_match && (chip->fm_address[0] & 0xf0) == 0x30;
            chip->fm_is40 = op_match && (chip->fm_address[0] & 0xf0) == 0x40;
            chip->fm_is50 = op_match && (chip->fm_address[0] & 0xf0) == 0x50;
            chip->fm_is60 = op_match && (chip->fm_address[0] & 0xf0) == 0x60;
            chip->fm_is70 = op_match && (chip->fm_address[0] & 0xf0) == 0x70;
            chip->fm_is80 = op_match && (chip->fm_address[0] & 0xf0) == 0x80;
            chip->fm_is90 = op_match && (chip->fm_address[0] & 0xf0) == 0x90;
            chip->fm_isa0 = ch_match && (chip->fm_address[0] & 0xfc) == 0xa0;
            chip->fm_isa4 = ch_match && (chip->fm_address[0] & 0xfc) == 0xa4;
            chip->fm_isa8 = ch_match && (chip->fm_address[0] & 0xfc) == 0xa8;
            chip->fm_isac = ch_match && (chip->fm_address[0] & 0xfc) == 0xac;
            chip->fm_isb0 = ch_match && (chip->fm_address[0] & 0xfc) == 0xb0;
            chip->fm_isb4 = ch_match && (chip->fm_address[0] & 0xfc) == 0xb4;

            chip->reg_cnt1[1] = chip->reg_cnt1[0];
            chip->reg_cnt2[1] = chip->reg_cnt2[0];
            chip->reg_cnt_sync = chip->fsm_sel23[1];

            chip->reg_cnt_rss[1] = chip->reg_cnt_rss[0];

            chip->reg_cnt_rss_of = (chip->reg_cnt_rss[0] & 11) == 11;

            chip->rss_18 = chip->write_fm_data[0]
                && chip->reg_cnt_rss[0] == (chip->fm_address[0] & 7) && (chip->fm_address[0] & 6) != 6
                && (chip->fm_address[0] & 0x1f8) == 0x18;

            chip->reg_kon_match = chip->reg_key_cnt1[0] == (chip->reg_kon_channel[0] & 3)
                && chip->reg_key_cnt2[0] == (((chip->reg_kon_channel[0] >> 2) & 1) && chip->reg_sch[0]);

            chip->reg_kon[0][1] = chip->reg_kon[0][0];
            chip->reg_kon[1][1] = chip->reg_kon[1][0];
            chip->reg_kon[2][1] = chip->reg_kon[2][0];
            chip->reg_kon[3][1] = chip->reg_kon[3][0];

            chip->reg_ch3_sel = chip->fsm_sel_ch3[1];

            chip->ch3_en = chip->reg_ch3[0] != 0;
            chip->ch3_csm = chip->reg_ch3[0] == 2;

            chip->timer_a_cnt[1] = chip->timer_a_cnt[0];
            chip->timer_a_of[1] = chip->timer_a_of[0];

            chip->reg_sync_timer = chip->fsm_sel1[1];
            chip->reg_sync_timer_l[1] = chip->reg_sync_timer_l[0];

            chip->timer_a_reg_load_l[1] = chip->timer_a_reg_load_l[0];

            chip->reg_sync_timer_load = chip->reg_sync_timer_l[0] && chip->reg_sync_timer_l[1];
            if (chip->reg_sync_timer_load)
            {
                chip->timer_a_reg_load = chip->reg_timer_a_load[1];
                chip->timer_b_reg_load = chip->reg_timer_b_load[1];
            }

            chip->timer_a_load = chip->timer_a_of[1] || (!chip->timer_a_reg_load_l[1] && chip->timer_a_reg_load);

            if (chip->reg_sync_timer_load)
            {
                chip->ch3_csm_load = chip->ch3_csm && chip->timer_a_load;
            }

            chip->kon_comb = (chip->reg_kon[3][1] >> 5) & 1;
            chip->kon_comb |= chip->reg_ch3_sel && chip->ch3_csm_load;

            chip->timer_a_status[1] = chip->timer_a_status[0];

            chip->timer_b_subcnt[1] = chip->timer_b_subcnt[0];
            chip->timer_b_subcnt_of[1] = chip->timer_b_subcnt_of[0];

            chip->timer_b_cnt[1] = chip->timer_b_cnt[0];
            chip->timer_b_of[1] = chip->timer_b_of[0];

            chip->timer_b_reg_load_l[1] = chip->timer_b_reg_load_l[0];

            chip->timer_b_load = chip->timer_b_of[1] || (!chip->timer_b_reg_load_l[1] && chip->timer_b_reg_load);

            chip->timer_b_status[1] = chip->timer_b_status[0];

            chip->irq_eos_l = chip->tm_w1;


            memcpy(&chip->reg_freq[1][0], &chip->reg_freq[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->reg_freq_3ch[1][0], &chip->reg_freq_3ch[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->reg_connect_fb[1][0], &chip->reg_connect_fb[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->reg_b4[1][0], &chip->reg_b4[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->reg_rss[1][0], &chip->reg_rss[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->op_multi_dt[1][0][0], &chip->op_multi_dt[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_tl[1][0][0], &chip->op_tl[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ar_ks[1][0][0], &chip->op_ar_ks[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_dr_a[1][0][0], &chip->op_dr_a[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_sr[1][0][0], &chip->op_sr[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_rr_sl[1][0][0], &chip->op_rr_sl[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ssg[1][0][0], &chip->op_ssg[0][0][0], 12 * 2 * sizeof(unsigned char));

            chip->reg_a4[1] = chip->reg_a4[0];
            chip->reg_ac[1] = chip->reg_ac[0];

            chip->ch_cnt_sync = chip->fsm_sel23[1];

            chip->ch_cnt1[1] = chip->ch_cnt1[0];
            chip->ch_cnt2[1] = chip->ch_cnt2[0];

            chip->fnum[1] = chip->fnum[0];
            chip->fnum[3] = chip->fnum[2];
            chip->kcode[1] = chip->kcode[0];
            chip->kcode[3] = chip->kcode[2];
        }

        {
            int write10 = chip->addr_10[1] && (chip->data_bus1 & 0x100) != 0 && chip->write1_en;
            int irq_rst = write10 && (chip->data_bus2 & 0x80) == 0;
            chip->irq_mask_eos = (chip->reg_mask[1] & 4) != 0 || chip->irq_eos_l || irq_rst;
            chip->irq_mask_brdy = (chip->reg_mask[1] & 8) != 0 || irq_rst;
            chip->irq_mask_zero = (chip->reg_mask[1] & 16) != 0 || irq_rst;
        }

#undef ADDRESS_MATCH
    }

    {
        if (chip->clk1)
        {
            int fsm_cnt;
            int of1 = (chip->fsm_cnt1[1] & 2) != 0;
            chip->fsm_cnt1[0] = (chip->ic_check3 || of1) ? 0 : (chip->fsm_cnt1[1] + 1) & 3;
            chip->fsm_cnt2[0] = chip->ic_check3 ? 0 : (chip->fsm_cnt2[1] + of1) & 7;

            fsm_cnt = (chip->fsm_cnt2[0] << 2) | chip->fsm_cnt1[0];

            chip->fsm_out[0] = (fsm_cnt & 30) == 30;
            chip->fsm_out[1] = (fsm_cnt & 28) == 0;
            chip->fsm_out[2] = (fsm_cnt & 30) == 4;
            chip->fsm_out[3] = (fsm_cnt & 30) == 22;
            chip->fsm_out[4] = (fsm_cnt & 28) == 24;
            chip->fsm_out[5] = (fsm_cnt & 30) == 28;
            chip->fsm_out[6] = (fsm_cnt & 30) == 14;
            chip->fsm_out[7] = (fsm_cnt & 28) == 16;
            chip->fsm_out[8] = (fsm_cnt & 30) == 20;
            chip->fsm_out[9] = (fsm_cnt & 30) == 6;
            chip->fsm_out[10] = (fsm_cnt & 28) == 8;
            chip->fsm_out[11] = (fsm_cnt & 30) == 12;
            chip->fsm_out[12] = (fsm_cnt & 30) == 30;
            chip->fsm_out[13] = fsm_cnt == 0;
            chip->fsm_out[14] = fsm_cnt == 1;
            chip->fsm_out[15] = fsm_cnt == 13;
            chip->fsm_out[16] = fsm_cnt == 29;
            chip->fsm_out[17] = (fsm_cnt & 7) == 1;
            chip->fsm_out[18] = (fsm_cnt & 28) == 4;
            chip->fsm_out[19] = fsm_cnt == 8;
            chip->fsm_out[20] = (fsm_cnt & 28) == 20;
            chip->fsm_out[21] = fsm_cnt == 24;

            chip->alg_mod_op1_0_l = 0;
            chip->alg_mod_op1_1_l = 0;
            chip->alg_mod_op2_l = 0;
            chip->alg_mod_prev_0_l = 0;
            chip->alg_mod_prev_1_l = 0;
            chip->alg_output_l = 0;

            if (chip->fsm_op2_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[0][0][connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[0][1][connect];
                chip->alg_mod_op2_l |= fm_algorithm[0][2][connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[0][3][connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[0][4][connect];
                chip->alg_output_l |= fm_algorithm[2][5][connect];
            }
            if (chip->fsm_op4_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[1][0][connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[1][1][connect];
                chip->alg_mod_op2_l |= fm_algorithm[1][2][connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[1][3][connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[1][4][connect];
                chip->alg_output_l |= fm_algorithm[3][5][connect];
            }
            if (chip->fsm_op1_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[2][0][connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[2][1][connect];
                chip->alg_mod_op2_l |= fm_algorithm[2][2][connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[2][3][connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[2][4][connect];
                chip->alg_output_l |= fm_algorithm[0][5][connect];
            }
            if (chip->fsm_op3_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[3][0][connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[3][1][connect];
                chip->alg_mod_op2_l |= fm_algorithm[3][2][connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[3][3][connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[3][4][connect];
                chip->alg_output_l |= fm_algorithm[1][5][connect];
            }

            chip->alg_do_fb[1] = chip->alg_do_fb[0];

            chip->alg_load_fb = chip->fsm_op1_sel_l;

            chip->fsm_sel0[1] = chip->fsm_sel0[0];
            chip->fsm_sel1[1] = chip->fsm_sel1[0];
            chip->fsm_sel2[1] = chip->fsm_sel2[0];
            chip->fsm_sel12[1] = chip->fsm_sel12[0];
            chip->fsm_sel23[1] = chip->fsm_sel23[0];
            chip->fsm_sel_ch3[1] = chip->fsm_sel_ch3[0];
            chip->fsm_sh1[1] = chip->fsm_sh1[0];
            chip->fsm_sh2[1] = chip->fsm_sh2[0];

            chip->fsm_rss2 = !chip->fsm_out[16] && !chip->fsm_sel23[1];
        }
        if (chip->clk2)
        {
            chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
            chip->fsm_cnt2[1] = chip->fsm_cnt2[0];

            chip->fsm_op4_sel_l = chip->fsm_out[0] || chip->fsm_out[1] || chip->fsm_out[2];
            chip->fsm_op2_sel_l = chip->fsm_out[3] || chip->fsm_out[4] || chip->fsm_out[5];
            chip->fsm_op3_sel_l = chip->fsm_out[6] || chip->fsm_out[7] || chip->fsm_out[8];
            chip->fsm_op1_sel_l = chip->fsm_out[9] || chip->fsm_out[10] || chip->fsm_out[11];

            chip->fsm_connect = chip->reg_connect_fb[0][4] & 7;

            chip->alg_do_fb[0] = chip->alg_mod_op1_1_l;

            chip->fsm_sel0[0] = chip->fsm_out[12];
            chip->fsm_sel1[0] = chip->fsm_out[13];
            chip->fsm_sel2[0] = chip->fsm_out[14];
            chip->fsm_sel12[0] = chip->fsm_out[15];
            chip->fsm_sel23[0] = chip->fsm_out[16];
            chip->fsm_sel_ch3[0] = chip->fsm_out[17];
            chip->fsm_sh1[0] = chip->fsm_out[18] || chip->fsm_out[19];
            chip->fsm_sh2[0] = chip->fsm_out[20] || chip->fsm_out[21];

            chip->fsm_rss = (chip->fsm_cnt2[0] & 2) != 0;
        }
    }

    {
        if (chip->clk1)
        {
            int inc = chip->lfo_sync[0] || (chip->reg_test_21[1] & 2) != 0;
            int subcnt_rst = chip->ic || chip->lfo_subcnt_of;

            chip->lfo_subcnt[0] = subcnt_rst ? 0 : (chip->lfo_subcnt[1] + inc) & 127;

            int cnt = chip->lfo_cnt[1] + chip->lfo_subcnt_of;

            chip->lfo_cnt_of = (cnt & 128) != 0 && chip->lfo_mode;
            chip->lfo_cnt[0] = chip->lfo_cnt_rst ? 0 : cnt & 127;

            chip->lfo_sync[1] = chip->lfo_sync[0];
            chip->lfo_sync[3] = chip->lfo_sync[2];

            // LFO shift
            static const int pg_lfo_sh1[8][8] = {
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 7, 7, 1, 1 },
                { 7, 7, 7, 7, 1, 1, 1, 1 },
                { 7, 7, 7, 1, 1, 1, 1, 0 },
                { 7, 7, 1, 1, 0, 0, 0, 0 },
                { 7, 7, 1, 1, 0, 0, 0, 0 },
                { 7, 7, 1, 1, 0, 0, 0, 0 }
            };

#if 0
            // YM2608/YM2610 decap, doesn't match YM2608 nor YM2610 hardware tests though
            static const int pg_lfo_sh2[8][8] = {
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 2, 2, 2, 2 },
                { 7, 7, 7, 2, 2, 2, 7, 7 },
                { 7, 7, 2, 2, 7, 7, 2, 2 },
                { 7, 2, 2, 7, 7, 2, 2, 7 },
                { 7, 2, 7, 2, 7, 2, 2, 1 },
                { 7, 2, 7, 2, 7, 2, 2, 1 },
                { 7, 2, 7, 2, 7, 2, 2, 1 }
            };
#endif
#if 1
            // YM2612/YM3438 decap, matches to YM2608 and YM2610 hardware tests O_O
            static const int pg_lfo_sh2[8][8] = {
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 2, 2, 2, 2 },
                { 7, 7, 7, 2, 2, 2, 7, 7 },
                { 7, 7, 2, 2, 7, 7, 2, 2 },
                { 7, 7, 2, 7, 7, 7, 2, 7 },
                { 7, 7, 7, 2, 7, 7, 2, 1 },
                { 7, 7, 7, 2, 7, 7, 2, 1 },
                { 7, 7, 7, 2, 7, 7, 2, 1 }
            };
#endif
            int pms = chip->reg_b4[1][5] & 7;
            int lfo = (chip->lfo_cnt_load >> 2) & 7;
            if (chip->lfo_cnt_load & 32)
                lfo ^= 7;
            int fnum_h = chip->fnum[1] >> 4;

            chip->lfo_fnum1 = fnum_h >> pg_lfo_sh1[pms][lfo];
            chip->lfo_fnum2 = fnum_h >> pg_lfo_sh2[pms][lfo];

            chip->lfo_shift = 2;
            if (pms > 5)
                chip->lfo_shift = 7 - pms;

            chip->lfo_sign = (chip->lfo_cnt_load >> 6) & 1;

            chip->lfo_fnum = ((chip->fnum[3] << 1) + chip->lfo_pm) & 0xfff;
        }
        if (chip->clk2)
        {
            static int lfo_cycles[8] = {
                108, 77, 71, 67, 62, 44, 8, 5
            };
            chip->lfo_sync[0] = chip->fsm_sel23[1];
            chip->lfo_sync[2] = chip->lfo_sync[1];

            chip->lfo_subcnt[1] = chip->lfo_subcnt[0];

            chip->lfo_mode = chip->tm_w1 || (chip->tm_w2 && chip->tm_w3);
            int of = (chip->lfo_subcnt[0] & lfo_cycles[chip->reg_lfo[0] & 7]) == lfo_cycles[chip->reg_lfo[0] & 7];

            chip->lfo_subcnt_of = chip->lfo_mode ? (chip->lfo_subcnt[0] & 127) == 127 : of;
            chip->lfo_cnt_rst = chip->lfo_mode ? chip->tm_w1 : (chip->reg_lfo[0] & 8) != 0;

            chip->lfo_cnt[1] = chip->lfo_cnt[0];

            if (!chip->lfo_sync[3] && chip->lfo_sync[2])
            {
                chip->lfo_cnt_load = chip->lfo_cnt[1];
            }

            if (chip->lfo_cnt_load & 64)
                chip->lfo_am = chip->lfo_cnt_load & 63;
            else
                chip->lfo_am = (chip->lfo_cnt_load & 63) ^ 63;

            chip->lfo_pm = (chip->lfo_fnum1 + chip->lfo_fnum2) >> chip->lfo_shift;
            if (chip->lfo_sign)
                chip->lfo_pm = -chip->lfo_pm;
        }
    }


    {
        if (chip->clk1)
        {
            chip->pg_block = chip->kcode[3] >> 2;

            chip->pg_dt_multi = (chip->reg_key_cnt2[1] & 4) == 0 ? chip->op_multi_dt[1][5][0]
                : chip->op_multi_dt[1][5][1];

            chip->dt_note[1] = chip->dt_note[0];
            chip->dt_blockmax[1] = chip->chip->dt_blockmax[0];

            chip->dt_enable[1] = chip->dt_enable[0];

            chip->dt_sign[1] = chip->dt_sign[0];

            chip->dt_sum = chip->dt_add1 + chip->dt_add2 + 1;

            chip->pg_freqdt[0] = (chip->pg_freq + chip->pg_dt_add) & 0x1ffff;

            chip->pg_multi[1] = chip->pg_multi[0];
            chip->pg_multi[3] = chip->pg_multi[2];

            chip->pg_add[0] = chip->pg_multi[4] ? chip->pg_freqdt[1] * chip->pg_multi[4] :
                (chip->pg_freqdt[1] >> 1);
            chip->pg_add[2] = chip->pg_add[1];
            chip->pg_add[4] = chip->pg_add[3];

            chip->pg_reset[1] = chip->pg_reset[0];
            chip->pg_reset[3] = chip->pg_reset[2];

            memcpy(&chip->pg_phase[0][1], &chip->pg_phase[1][0], 22 * 2 * sizeof(int));

            chip->pg_phase2[0] = chip->pg_phase[1][22];

            chip->pg_phase[0][0] = (chip->pg_phase2[1] + chip->pg_add[5]) & 0xfffff;

            chip->pg_dbg[0] = chip->pg_dbg[1] >> 1;
            if (chip->pg_dbgsync)
                chip->pg_dbg[0] |= chip->pg_phase[1][22] & 1023;

        }
        if (chip->clk2)
        {
            chip->pg_freq = (chip->lfo_fnum << chip->pg_block) >> 2;

            chip->dt_note[0] = chip->kcode[2] & 3;
            chip->dt_blockmax[0] = (chip->kcode[2] & 28) == 28;
            chip->dt_add1 = (chip->kcode[2] >> 2) & 7;
            if ((chip->pg_dt_multi & 0x30) != 0)
                chip->dt_add1 |= 8;
            chip->dt_add2 = 0;
            if ((chip->pg_dt_multi & 0x30) == 0x30)
                chip->dt_add2 |= 1;
            if (chip->pg_dt_multi & 0x20)
                chip->dt_add2 |= 2;

            chip->dt_enable[0] = (chip->pg_dt_multi & 0x30) != 0;

            chip->dt_sign[0] = (chip->pg_dt_multi & 0x40) != 0;

            int dt_l = (chip->dt_sum & 1) << 2;
            if (!chip->dt_blockmax[1])
                dt_l |= chip->dt_note[1];
            int dt_h = chip->dt_sum >> 1;

            static const int pg_detune[8] = { 16, 17, 19, 20, 22, 24, 27, 29 };

            int dt_freq = pg_detune[dt_l] >> (9 - dt_h);

            if (chip->dt_sign[1])
                dt_freq = -dt_freq;

            chip->pg_dt_add = dt_freq;

            chip->pg_multi[0] = chip->pg_dt_multi & 15;
            chip->pg_multi[2] = chip->pg_multi[1];
            chip->pg_multi[4] = chip->pg_multi[3];

            chip->pg_freqdt[1] = chip->pg_freqdt[0];

            chip->pg_add[1] = chip->pg_add[0];
            chip->pg_add[3] = chip->pg_reset[1] ? 0 : chip->pg_add[2];
            chip->pg_add[5] = chip->pg_add[4];

            chip->pg_reset[0] = (chip->eg_pg_reset[0] & 2) != 0;
            chip->pg_reset[2] = chip->pg_reset[1];
            memcpy(&chip->pg_phase[1][0], &chip->pg_phase[0][0], 23 * 2 * sizeof(int));

            chip->pg_out = chip->pg_phase[1][18] >> 10;

            chip->pg_phase2[1] = chip->pg_reset[3] ? 0 : chip->pg_phase2[0];

            chip->pg_dbgsync = chip->fsm_sel2[1];

            chip->pg_dbg[1] = chip->pg_dbg[0];
        }
    }

    {
        if (chip->clk1)
        {
            chip->eg_prescaler_clock_l[0] = chip->eg_sync;
            chip->eg_prescaler[0] = (chip->eg_prescaler[1] + chip->eg_sync) & 3;
            if (((chip->eg_prescaler[1] & 2) != 0 && chip->eg_sync) || chip->ic)
                chip->eg_prescaler[0] = 0;
            chip->eg_step[0] = chip->eg_prescaler[1] >> 1;
            chip->eg_step[2] = chip->eg_step[1];
            chip->eg_timer_step[1] = chip->eg_timer_step[0];

            chip->eg_ic[0] = chip->ic;

            chip->eg_clock_delay[0] = (chip->eg_clock_delay[1] << 1) || chip->eg_prescaler_clock_l[1];


            int sum = (chip->eg_timer[1] >> 10) & 1;
            int add = chip->eg_timer_carry[1];
            if ((chip->eg_prescaler[1] & 2) != 0 && chip->eg_prescaler_clock_l[1])
                add = 1;
            sum += add;

            chip->eg_timer[0] = (chip->eg_timer[1] << 1) | chip->eg_timer_sum[1];

            chip->eg_timer_carry[0] = sum >> 1;
            chip->eg_timer_sum[0] = sum & 1;
            chip->eg_timer_test = (chip->reg_test_21[1] & 32) != 0;
            chip->eg_timer_test_bit[0] = chip->input.test;

            int timer_bit = chip->eg_timer_sum[1] || !chip->eg_timer_test_bit[1];

            chip->eg_timer_mask[0] = timer_bit | chip->eg_timer_mask[1];
            if (chip->eg_prescaler_clock_l[1] || ((chip->eg_clock_delay[1] >> 11) & 1) != 0 || chip->eg_ic[1])
                chip->eg_timer_mask[0] = 0;

            int timer_bit_masked = timer_bit && !chip->eg_timer_mask[1];

            chip->eg_timer_masked[0] = (chip->eg_timer_masked[1] << 1) | timer_bit_masked;

            if (chip->eg_timer_step[0] && chip->eg_timer_step[1])
            {
                int b0, b1, b2, b3;
                b0 = (chip->eg_timer[0] >> 11) & 1;
                b1 = (chip->eg_timer[0] >> 10) & 1;
                chip->eg_timer_low_lock = b1 * 2 + b0;

                b0 = (chip->eg_timer_masked[0] & 0xaaa) != 0;
                b1 = (chip->eg_timer_masked[0] & 0x666) != 0;
                b2 = (chip->eg_timer_masked[0] & 0x1e1) != 0;
                b3 = (chip->eg_timer_masked[0] & 0x1f) != 0;
                chip->eg_shift_lock = b3 * 8 + b2 * 4 + b1 * 2 + b0;
            }

            int bank = (chip->reg_key_cnt2[1] & 4) != 0;

            chip->eg_rate_ar = chip->op_ar_ks[1][11][bank] & 0x1f;
            chip->eg_ks = (chip->op_ar_ks[1][11][bank] >> 6) & 3;
            chip->eg_rate_dr = chip->op_dr_a[1][11][bank] & 0x1f;
            chip->eg_rate_sr = chip->op_sr[1][11][bank] & 0x1f;
            chip->eg_rate_rr = chip->op_rr_sl[1][11][bank] & 0xf;

            chip->eg_rate_nonzero[1] = chip->eg_rate_nonzero[0];

            chip->eg_rate2 = (chip->eg_rate << 1) + chip->eg_ksv;

            chip->eg_maxrate[1] = chip->eg_maxrate[0];

            int inc1 = 0;
            if (chip->eg_rate_slow && chip->eg_rate_nonzero[2])
            {
                switch (sum & 15)
                {
                case 12:
                    inc1 = chip->eg_ratenz;
                    break;
                case 13:
                    inc1 = (chip->eg_rate_low >> 1) & 1;
                    break;
                case 14:
                    inc1 = chip->eg_rate_low & 1;
                    break;
                }
            }

            chip->eg_incsh0[0] = inc1;
            chip->eg_incsh3[0] = chip->eg_rate15;
            if (!chip->eg_inc2)
            {
                chip->eg_incsh0[0] |= chip->eg_rate12;
                chip->eg_incsh1[0] = chip->eg_rate13;
                chip->eg_incsh2[0] = chip->eg_rate14;
            }
            else
            {
                chip->eg_incsh1[0] = chip->eg_rate12;
                chip->eg_incsh2[0] = chip->eg_rate13;
                chip->eg_incsh3[0] |= chip->eg_rate14;
            }

            chip->eg_kon_latch[0] = (chip->eg_kon_latch[1] << 1) | chip->kon_comb;
            int csm_kon = chip->reg_ch3_sel && chip->ch3_csm_load;
            chip->eg_kon_csm[0] = (chip->eg_kon_csm[1] << 1) | csm_kon;

            int kon = (chip->eg_kon_latch[1] >> 1) & 1;
            int okon = (chip->eg_key[1] >> 23) & 1;
            int pg_reset = (kon && !okon) || (chip->eg_ssg_pgreset[1] & 2) != 0;
            chip->eg_pg_reset[0] = (chip->eg_pg_reset[1] << 1) | pg_reset;
            chip->eg_kon_event = (kon && !okon) || (okon && (chip->eg_ssg_egrepeat[1] & 2) != 0);

            chip->eg_key[0] = (chip->eg_key[1] << 1) | kon;

            int okon2 = (chip->eg_key[1] >> 21) & 1;


            chip->eg_ssg_sign[0] = (chip->eg_level[1][19] & 0x200) != 0;

            int ssg_eg = chip->op_ssg[1][11][bank] & 15;
            int ssg_enable = (ssg_eg & 8) != 0;
            chip->eg_ssg_enable[0] = (chip->eg_ssg_enable[1] << 1) | ssg_enable;
            int ssg_inv_e = ssg_enable && (ssg_eg & 4) != 0;
            int ssg_holdup = ssg_enable && ((ssg_eg & 7) == 3 || (ssg_eg & 7) == 5) && chip->kon_comb;
            chip->eg_ssg_holdup[0] = (chip->eg_ssg_holdup[1] << 1) | ssg_holdup;
            int ssg_pgreset = ssg_enable && chip->eg_ssg_sign[1] && (ssg_eg & 3) == 0;
            chip->eg_ssg_pgreset[0] = (chip->eg_ssg_pgreset[1] << 1) | ssg_pgreset;
            int ssg_egrepeat = ssg_enable && chip->eg_ssg_sign[1] && (ssg_eg & 1) == 0;
            chip->eg_ssg_egrepeat[0] = (chip->eg_ssg_egrepeat[1] << 1) | ssg_egrepeat;

            chip->eg_rate_sel = (okon2 ? ssg_egrepeat : chip->kon_comb) ? eg_state_attack : chip->eg_state[1][20];

            int ssg_odir = (chip->eg_ssg_dir[1] >> 23) & 1;
            int ssg_dir = ssg_enable && okon2 &&
                ((ssg_odir ^ ((ssg_eg & 3) == 2 && chip->eg_ssg_sign[1])) || ((ssg_eg & 3) == 3 && chip->eg_ssg_sign[1]));
            chip->eg_ssg_dir[0] = (chip->eg_ssg_dir[1] << 1) | ssg_dir;

            int ssg_inv = okon2 && (ssg_odir ^ ssg_inv_e);

            chip->eg_ssg_inv = ssg_inv;

            chip->eg_level_ssg[0] = chip->eg_output;
            int eg_output = chip->eg_output;

            if (chip->reg_test_21[1] & 32)
                eg_output = 0;

            int sl = (chip->op_rr_sl[1][11][bank] >> 4) & 15;

            if (sl == 15)
                sl |= 16;

            chip->eg_sl[0][0] = sl;
            chip->eg_sl[1][0] = chip->eg_sl[0][1];
            chip->eg_tl[0][0] = chip->op_tl[1][11][bank];
            chip->eg_tl[1][0] = chip->eg_tl[0][1];
            chip->eg_tl[2][0] = chip->eg_tl[1][1];

            int level = (okon && !kon) ? chip->eg_level_ssg[1] : chip->eg_level[1][21];

            chip->eg_off = (chip->eg_ssg_enable[1] & 2) != 0 ? (level & 512) != 0 : (level & 0x3f0) == 0x3f0;
            chip->eg_slreach = (level >> 4) == (chip->eg_sl[1][1] << 1);
            chip->eg_zeroreach = level == 0;
            
            chip->eg_level_l[0] = level;

            chip->eg_state_l = chip->eg_state[1][22];

            memcpy(&chip->eg_state[0][1], &chip->eg_state[1][0], 22 * sizeof(unsigned char));
            chip->eg_state[0][0] = chip->eg_nextstate;

            int inc_total = 0;
            if (chip->eg_exp)
            {
                if (chip->eg_incsh0[1])
                    inc_total |= ~chip->eg_level_l[1] >> 4;
                if (chip->eg_incsh1[1])
                    inc_total |= ~chip->eg_level_l[1] >> 3;
                if (chip->eg_incsh2[1])
                    inc_total |= ~chip->eg_level_l[1] >> 2;
                if (chip->eg_incsh3[1])
                    inc_total |= ~chip->eg_level_l[1] >> 1;
            }
            if (chip->eg_linear)
            {
                if (chip->eg_ssg_enable[1] & 4)
                {
                    if (chip->eg_incsh0[1])
                        inc_total |= 4;
                    if (chip->eg_incsh1[1])
                        inc_total |= 8;
                    if (chip->eg_incsh2[1])
                        inc_total |= 16;
                    if (chip->eg_incsh3[1])
                        inc_total |= 32;
                }
                else
                {
                    if (chip->eg_incsh0[1])
                        inc_total |= 1;
                    if (chip->eg_incsh1[1])
                        inc_total |= 2;
                    if (chip->eg_incsh2[1])
                        inc_total |= 4;
                    if (chip->eg_incsh3[1])
                        inc_total |= 8;
                }
            }

            chip->eg_inc_total = inc_total;

            int nextlevel = 0;

            if (!chip->eg_istantattack)
                nextlevel |= chip->eg_level_l[1];

            if (chip->eg_kon_csm[1] & 4)
                nextlevel |= chip->eg_tl[2][1] << 3;

            if (chip->eg_mute)
                nextlevel |= 0x3ff;

            chip->eg_nextlevel[0] = nextlevel;

            memcpy(&chip->eg_level[0][1], &chip->eg_level[1][0], 21 * sizeof(unsigned short));

            chip->eg_am_l[0] = chip->lfo_am;
            chip->eg_am_shift[0] = (chip->op_dr_a[1][11][bank] & 0x80) != 0 ? (chip->reg_b4[1][5] >> 4) & 3 : 0;
            static const int eg_am_shift[4] = {
                7, 3, 1, 0
            };

            int lfo_add = (chip->eg_am_l[1] << 1) >> eg_am_shift[chip->eg_am_shift[1]];

            eg_output += lfo_add;

            chip->eg_of1 = (eg_output & 1024) != 0;
            chip->eg_output_lfo = eg_output & 1023;

            chip->eg_ch3_l[1] = chip->eg_ch3_l[0];

            chip->eg_csm_tl = (chip->ch3_csm && (chip->eg_ch3_l[0] & 2) != 0) ? 0 : chip->eg_tl[1][0];

            chip->eg_debug[0] = chip->eg_debug[1] << 1;

            if (chip->eg_dbg_sync)
            {
                chip->eg_debug[0] |= chip->eg_out;
            }
        }
        if (chip->clk2)
        {
            chip->eg_sync = chip->fsm_sel0[1];
            chip->eg_prescaler_clock_l[1] = chip->eg_prescaler_clock_l[0];
            chip->eg_prescaler[1] = chip->eg_prescaler[0];
            chip->eg_step[1] = chip->eg_step[0];
            chip->eg_timer_step[0] = chip->eg_step[0] && chip->eg_prescaler_clock_l[0];

            chip->eg_ic[1] = chip->eg_ic[0];

            chip->eg_timer_test_bit[1] = chip->eg_timer_test_bit[0];

            chip->eg_timer_sum[1] = chip->eg_timer_sum[0] && !chip->eg_ic[0] && !chip->eg_timer_test;
            chip->eg_timer[1] = chip->eg_timer[0];
            chip->eg_clock_delay[1] = chip->eg_clock_delay[0];
            chip->eg_timer_carry[1] = chip->eg_timer_carry[0];
            chip->eg_timer_mask[1] = chip->eg_timer_mask[0];
            chip->eg_timer_masked[1] = chip->eg_timer_masked[0];

            int rate = 0;
            switch (chip->eg_rate_ar)
            {
                case eg_state_attack:
                    rate = chip->eg_rate_ar;
                    break;
                case eg_state_decay:
                    rate = chip->eg_rate_dr;
                    break;
                case eg_state_sustain:
                    rate = chip->eg_rate_sr;
                    break;
                case eg_state_release:
                    rate = (chip->eg_rate_rr * 2) | 1;
                    break;
            }

            chip->eg_rate_nonzero[0] = rate != 0;
            chip->eg_rate_nonzero[2] = chip->eg_rate_nonzero[1];
            chip->eg_rate = rate;
            chip->eg_ksv = chip->kcode[2] >> (chip->eg_ks ^ 3);

            int rate2 = chip->eg_rate2;
            if (rate2 & 64)
                rate2 = 63;

            rate2 &= 63;

            static const int eg_stephi[4][4] = {
                { 0, 0, 0, 0 },
                { 1, 0, 0, 0 },
                { 1, 0, 1, 0 },
                { 1, 1, 1, 0 }
            };

            chip->eg_inc2 = eg_stephi[rate2 & 3][chip->eg_timer_low_lock];
            chip->eg_ratenz = rate2 == 0;
            chip->eg_rate12 = (rate2 & 60) == 48;
            chip->eg_rate13 = (rate2 & 60) == 52;
            chip->eg_rate14 = (rate2 & 60) == 56;
            chip->eg_rate15 = (rate2 & 60) == 60;
            chip->eg_maxrate[0] = (rate2 & 62) == 62;
            chip->eg_rate_low = rate2 & 3;
            chip->eg_rate_slow = (rate2 & 48) != 48;

            chip->eg_incsh0[1] = chip->eg_step[2] && chip->eg_incsh0[0];
            chip->eg_incsh1[1] = chip->eg_step[2] && chip->eg_incsh1[0];
            chip->eg_incsh2[1] = chip->eg_step[2] && chip->eg_incsh2[0];
            chip->eg_incsh3[1] = chip->eg_step[2] && chip->eg_incsh3[0];

            chip->eg_kon_latch[1] = chip->eg_kon_latch[0];
            chip->eg_key[1] = chip->eg_key[0];

            chip->eg_level_ssg[1] = chip->eg_level_ssg[0];

            chip->eg_pg_reset[1] = chip->eg_pg_reset[0];

            chip->eg_ssg_enable[1] = chip->eg_ssg_enable[0];
            chip->eg_ssg_dir[1] = chip->eg_ssg_dir[0];
            chip->eg_ssg_holdup[1] = chip->eg_ssg_holdup[0];
            chip->eg_ssg_pgreset[1] = chip->eg_ssg_pgreset[0];
            chip->eg_ssg_egrepeat[1] = chip->eg_ssg_egrepeat[0];

            chip->eg_sl[0][1] = chip->eg_sl[0][0];
            chip->eg_sl[1][1] = chip->eg_sl[1][0];
            chip->eg_tl[0][1] = chip->eg_tl[0][0];
            chip->eg_tl[1][1] = chip->eg_tl[1][0];
            chip->eg_tl[2][1] = chip->eg_tl[2][0];

            chip->eg_nextlevel[1] = chip->eg_nextlevel[0] + chip->eg_inc_total;

            chip->eg_kon_csm[1] = chip->eg_kon_csm[0];

            int inv = ((chip->eg_level[0][20] ^ 1023) + 513) & 1023;

            chip->eg_output = chip->eg_ssg_inv ? inv : chip->eg_level[0][20];

            chip->eg_level_l[1] = chip->eg_level_l[0];


            int nextstate = eg_state_attack;

            int eg_mute = !chip->eg_kon_event && chip->eg_off && (chip->eg_ssg_holdup[0] & 4) == 0 && chip->eg_state_l != eg_state_attack;
            chip->eg_mute = eg_mute;

            if (eg_mute)
            {
                nextstate |= eg_state_release;
            }

            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_sustain)
            {
                nextstate |= eg_state_sustain;
            }

            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_decay && !chip->eg_slreach)
            {
                nextstate |= eg_state_decay;
            }
            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_decay && chip->eg_slreach)
            {
                nextstate |= eg_state_sustain;
            }

            if ((chip->eg_kon_latch[0] & 4) == 0 && !chip->eg_kon_event)
            {
                nextstate |= eg_state_release;
            }
            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_release)
            {
                nextstate |= eg_state_release;
            }

            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_attack && chip->eg_zeroreach)
            {
                nextstate |= eg_state_decay;
            }
            if (chip->eg_ic[0])
            {
                nextstate |= eg_state_release;
            }

            chip->eg_nextstate = nextstate;
            memcpy(&chip->eg_state[1][0], &chip->eg_state[0][0], 23 * sizeof(unsigned char));

            chip->eg_exp = (chip->eg_kon_latch[0] & 4) != 0 && (chip->eg_state_l == eg_state_attack) && !chip->eg_maxrate[1] && !chip->eg_zeroreach;
            chip->eg_linear = !chip->eg_kon_event && !chip->eg_off && (chip->eg_state_l == eg_state_sustain || chip->eg_state_l == eg_state_release);
            chip->eg_linear |= !chip->eg_kon_event && !chip->eg_off && !chip->eg_slreach && chip->eg_state_l == eg_state_decay;

            chip->eg_istantattack = chip->eg_maxrate[1] && (!chip->eg_maxrate[1] || kon_event);

            memcpy(&chip->eg_level[1][0], &chip->eg_level[0][0], 22 * sizeof(unsigned short));


            chip->eg_am_l[1] = chip->eg_am_l[0];
            chip->eg_am_shift[1] = chip->eg_am_shift[0];

            chip->eg_ch3_l[0] = (chip->eg_ch3_l[1] << 1) | chip->fsm_sel_ch3[1];

            int levelsum = chip->eg_output_lfo + (chip->eg_csm_tl << 3);

            int eg_of = (levelsum & 1024) != 0;

            if (eg_of || chip->eg_of1)
                levelsum = 1023;

            chip->eg_out = levelsum;

            chip->eg_dbg_sync = chip->fsm_sel2[1];
        }
    }

    {
        if (chip->clk1)
        {
            chip->op_phase1 = chip->pg_out;
            chip->op_phase2 = chip->op_mod[1][5];

            chip->op_sign[1] = chip->op_sign[0];
            static const int logsin[128] = {
                0x6c3, 0x58b, 0x4e4, 0x471, 0x41a, 0x3d3, 0x398, 0x365, 0x339, 0x311, 0x2ed, 0x2cd, 0x2af, 0x293, 0x279, 0x261,
                0x24b, 0x236, 0x222, 0x20f, 0x1fd, 0x1ec, 0x1dc, 0x1cd, 0x1be, 0x1b0, 0x1a2, 0x195, 0x188, 0x17c, 0x171, 0x166,
                0x15b, 0x150, 0x146, 0x13c, 0x133, 0x129, 0x121, 0x118, 0x10f, 0x107, 0x0ff, 0x0f8, 0x0f0, 0x0e9, 0x0e2, 0x0db,
                0x0d4, 0x0cd, 0x0c7, 0x0c1, 0x0bb, 0x0b5, 0x0af, 0x0a9, 0x0a4, 0x09f, 0x099, 0x094, 0x08f, 0x08a, 0x086, 0x081,
                0x07d, 0x078, 0x074, 0x070, 0x06c, 0x068, 0x064, 0x060, 0x05c, 0x059, 0x055, 0x052, 0x04e, 0x04b, 0x048, 0x045,
                0x042, 0x03f, 0x03c, 0x039, 0x037, 0x034, 0x031, 0x02f, 0x02d, 0x02a, 0x028, 0x026, 0x024, 0x022, 0x020, 0x01e,
                0x01c, 0x01a, 0x018, 0x017, 0x015, 0x014, 0x012, 0x011, 0x00f, 0x00e, 0x00d, 0x00c, 0x00a, 0x009, 0x008, 0x007,
                0x007, 0x006, 0x005, 0x004, 0x004, 0x003, 0x002, 0x002, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000
            };
            static const int logsin_d[128] = {
                0x196, 0x07c, 0x04a, 0x035, 0x029, 0x022, 0x01d, 0x019, 0x015, 0x013, 0x012, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c,
                0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x009, 0x008, 0x007, 0x007, 0x007, 0x007, 0x006, 0x007, 0x006, 0x006, 0x005,
                0x005, 0x005, 0x005, 0x005, 0x004, 0x005, 0x004, 0x004, 0x005, 0x004, 0x004, 0x003, 0x004, 0x003, 0x003, 0x003,
                0x003, 0x004, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x002, 0x003, 0x003, 0x003, 0x003, 0x002, 0x002,
                0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x001, 0x002, 0x002, 0x002, 0x001,
                0x001, 0x001, 0x002, 0x002, 0x001, 0x001, 0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
                0x001, 0x001, 0x001, 0x000, 0x001, 0x000, 0x001, 0x000, 0x001, 0x001, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001,
                0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x001, 0x000, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
            };
            static const int pow[128] = {
                0x3f5, 0x3ea, 0x3df, 0x3d4, 0x3c9, 0x3bf, 0x3b4, 0x3a9, 0x39f, 0x394, 0x38a, 0x37f, 0x375, 0x36a, 0x360, 0x356,
                0x34c, 0x342, 0x338, 0x32e, 0x324, 0x31a, 0x310, 0x306, 0x2fd, 0x2f3, 0x2e9, 0x2e0, 0x2d6, 0x2cd, 0x2c4, 0x2ba,
                0x2b1, 0x2a8, 0x29e, 0x295, 0x28c, 0x283, 0x27a, 0x271, 0x268, 0x25f, 0x257, 0x24e, 0x245, 0x23c, 0x234, 0x22b,
                0x223, 0x21a, 0x212, 0x209, 0x201, 0x1f9, 0x1f0, 0x1e8, 0x1e0, 0x1d8, 0x1d0, 0x1c8, 0x1c0, 0x1b8, 0x1b0, 0x1a8,
                0x1a0, 0x199, 0x191, 0x189, 0x181, 0x17a, 0x172, 0x16b, 0x163, 0x15c, 0x154, 0x14d, 0x146, 0x13e, 0x137, 0x130,
                0x129, 0x122, 0x11b, 0x114, 0x10c, 0x106, 0x0ff, 0x0f8, 0x0f1, 0x0ea, 0x0e3, 0x0dc, 0x0d6, 0x0cf, 0x0c8, 0x0c2,
                0x0bb, 0x0b5, 0x0ae, 0x0a8, 0x0a1, 0x09b, 0x094, 0x08e, 0x088, 0x082, 0x07b, 0x075, 0x06f, 0x069, 0x063, 0x05d,
                0x057, 0x051, 0x04b, 0x045, 0x03f, 0x039, 0x033, 0x02d, 0x028, 0x022, 0x01c, 0x016, 0x011, 0x00b, 0x006, 0x000,
            };
            static const int pow_d[128] = {
                0x005, 0x005, 0x005, 0x006, 0x006, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x006, 0x005, 0x005,
                0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x004, 0x005,
                0x004, 0x004, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x004, 0x004, 0x004, 0x005, 0x004, 0x005,
                0x004, 0x004, 0x004, 0x005, 0x004, 0x004, 0x005, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
                0x004, 0x003, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x003, 0x004, 0x004, 0x004,
                0x003, 0x003, 0x003, 0x003, 0x004, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 0x004, 0x003, 0x003, 0x004, 0x003,
                0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003,
                0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x002, 0x003,
            };

            chip->op_logsin_base = logsin[chip->op_phase_index >> 1];
            chip->op_logsin_delta = (chip->op_phase_index & 1) != 0 ? 0 : logsin_d[chip->op_phase_index >> 1];

            chip->op_eglevel = chip->eg_out;

            chip->op_shift[0] = chip->op_att >> 8;

            chip->op_pow_base = pow[(chip->op_att >> 1) & 127];
            chip->op_pow_delta = (chip->op_att & 1) != 0 ? 0 : pow_d[(chip->op_att >> 1) & 127];

            int output = chip->op_pow;
            output = (output << 2) >> chip->op_shift[1];

            if (chip->mode_test_21[1] & 16)
                output ^= 1 << 13;

            if (chip->op_sign[0] & 4)
                output ^= 0x3fff;

            chip->op_output[0] = output;
            chip->op_output[2] = chip->op_output[1];

            memcpy(&chip->op_op1_0[0][1], &chip->op_op1_0[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->op_op1_1[0][1], &chip->op_op1_1[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->op_op2[0][1], &chip->op_op2[1][0], 5 * sizeof(unsigned short));

            chip->op_op1_0[0][0] = chip->op_loadfb ? chip->op_output[3] : chip->op_op1_0[1][5];
            chip->op_op1_1[0][0] = chip->op_loadfb ? chip->op_op1_0[1][5] : chip->op_op1_1[1][5];
            chip->op_op2[0][0] = chip->op_loadop2 ? chip->op_output[3] : chip->op_op2[1][5];

            int mod1 = 0;
            int mod2 = 0;

            if (chip->op_mod_op1_0)
            {
                mod2 |= chip->op_op1_0[1][5]
            }
            if (chip->op_mod_op1_1)
            {
                mod1 |= chip->op_op1_1[1][5];
            }
            if (chip->op_mod_op2)
            {
                mod1 |= chip->op_op2[1][5];
            }
            if (chip->op_mod_prev_0)
            {
                mod2 |= chip->op_output[3];
            }
            if (chip->op_mod_prev_1)
            {
                mod1 |= chip->op_output[3];
            }
            if (mod1 & (1 << 13))
                mod1 |= 1 << 14;
            if (mod2 & (1 << 13))
                mod2 |= 1 << 14;
            chip->op_mod1 = mod1;
            chip->op_mod2 = mod2;

            int mod;

            if (chip->op_do_fb)
            {
                if (!chip->op_fb)
                    mod = 0;
                else
                {
                    mod = chip->op_mod_sum;
                    if (mod & (1 << 13))
                        mod |= ~0x3fff;

                    mod = mod >> (9 - chip->op_fb);
                }
            }
            else
                mod = chip->op_mod_sum;

            memcpy(&chip->op_mod[0][1], &chip->op_mod[1][0], 5 * sizeof(unsigned short));
            chip->op_mod[0][0] = mod;
        }
        if (chip->clk2)
        {

            int phase = chip->op_phase1 + chip->op_phase2;

            int phase2 = phase & 255;
            if (phase & 256)
                phase2 ^= 255;

            chip->op_phase_index = phase2;

            chip->op_sign[0] = chip->op_sign[1] << 1;
            if (phase & 512)
                chip->op_sign[0] |= 1;

            int level = chip->op_logsin_base + chip->op_logsin_delta + (chip->op_eglevel << 2);
            if (level & 4096)
                level = 4095;
            chip->op_att = level;

            chip->op_shift[1] = chip->op_shift[0];

            chip->op_pow = (chip->op_pow_base + chip->op_pow_delta) | 0x400;

            int output = chip->eg_output[0];

            if (chip->op_sign[1] & 4)
                output++;

            chip->op_output[1] = output;
            chip->op_output[3] = chip->op_output[2];

            chip->op_loadfb = chip->alg_load_fb;
            chip->op_loadop2 = chip->alg_mod_op1_1_l;
            chip->op_mod_op1_0 = chip->alg_mod_op1_0_l;
            chip->op_mod_op1_1 = chip->alg_mod_op1_1_l;
            chip->op_mod_op2 = chip->alg_mod_op2_l;
            chip->op_mod_prev_0 = chip->alg_mod_prev_0_l;
            chip->op_mod_prev_1 = chip->alg_mod_prev_1_l;
            chip->op_do_fb = chip->alg_do_fb[1];

            chip->op_fb = (chip->reg_connect_fb[0][0] >> 3) & 7;

            memcpy(&chip->op_op1_0[1][0], &chip->op_op1_0[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->op_op1_1[1][0], &chip->op_op1_1[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->op_op2[1][0], &chip->op_op2[0][0], 6 * sizeof(unsigned short));
            
            int mod = (chip->op_mod1 + chip->op_mod2) >> 1;
            mod &= 0x3fff;
            chip->op_mod_sum = mod;
            memcpy(&chip->op_mod[1][0], &chip->op_mod[0][0], 6 * sizeof(unsigned short));
        }
    }
}
