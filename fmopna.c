#include "fmopna.h"
#include "fmopna_rom.h"


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

            chip->addr_10[0] = chip->write0_en ? ADDRESS_MATCH(0x10) : chip->addr_10[1];
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
            }
            else
            {
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

            chip->reg_test_21[1] = chip->reg_test_21[0];
            chip->reg_lfo[1] = chip->reg_lfo[0];
            chip->reg_timer_a[1] = chip->reg_timer_a[0];
            chip->reg_timer_b[1] = chip->reg_timer_b[0];
            chip->reg_sch[1] = chip->reg_sch[0];
            chip->reg_irq[1] = chip->reg_irq[0];

            chip->fm_is30 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x30;
            chip->fm_is40 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x40;
            chip->fm_is50 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x50;
            chip->fm_is60 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x60;
            chip->fm_is70 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x70;
            chip->fm_is80 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x80;
            chip->fm_is90 = chip->tm_w1 && (chip->fm_address[0] & 0xf0) == 0x90;
            chip->fm_isa0 = chip->tm_w1 && (chip->fm_address[0] & 0xfc) == 0xa0;
            chip->fm_isa4 = chip->tm_w1 && (chip->fm_address[0] & 0xfc) == 0xa4;
            chip->fm_isa8 = chip->tm_w1 && (chip->fm_address[0] & 0xfc) == 0xa8;
            chip->fm_isac = chip->tm_w1 && (chip->fm_address[0] & 0xfc) == 0xac;
            chip->fm_isb0 = chip->tm_w1 && (chip->fm_address[0] & 0xfc) == 0xb0;
            chip->fm_isb4 = chip->tm_w1 && (chip->fm_address[0] & 0xfc) == 0xb4;
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
        }
        if (chip->clk2)
        {
            chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
            chip->fsm_cnt2[1] = chip->fsm_cnt2[0];
        }
    }
}
