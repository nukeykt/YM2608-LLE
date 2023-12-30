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

            chip->fsm_connect = chip->tm_w1;

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
}
