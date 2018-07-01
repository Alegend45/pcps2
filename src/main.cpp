#ifdef __APPLE__
#error "Apple is vehemently anti-dev, and refuses to support modern OpenGL, OpenCL, or any Vulkan natively. Therefore, this software does not support MacOS or Apple."
#endif

#include "common.h"
#include "attotime.h"
#include "ee.h"
#include "iop.h"
#include "scph10000.h"
#include "scph15000.h"

enum class ps2_model
{
    scph10000, scph15000
};

int main(int ac, char** av)
{
#ifdef __APPLE__
    printf("Really? You were smart enough to disable the first check, but still stupid enough to use an Apple operating system? Fucking shame on you, I'm out.\n");
    return -1;
#endif
    if(ac < 2)
    {
        printf("usage: %s [model]\n", av[0]);
        printf("model can be \"scph10000\" or \"scph15000\". No other models are supported at this time.\n");
        return 1;
    }

    std::string model = av[1];
    ps2_model ps2_type;
    double ee_clock = 294912000;
    double iop_clock = 67737600 / 2;

    attotime ee_atto = attotime::from_hz(ee_clock);
    attotime iop_atto = attotime::from_hz(iop_clock);
    attotime current_time = attotime::zero;

    if(model == "scph10000")
    {
        ps2_type = ps2_model::scph10000;

        scph10000 dev;
        ee_cpu ee;
        iop_cpu iop;

        ee.init();
        iop.init();

        dev.ee = &ee;
        dev.iop = &iop;

        dev.init();

        ee.device = &dev;
        iop.device = &dev;

        ee.rb_real = scph10000_ee_rb;
        ee.rh_real = scph10000_ee_rh;
        ee.rw_real = scph10000_ee_rw;
        ee.rd_real = scph10000_ee_rd;
        ee.rq_real = scph10000_ee_rq;
        ee.wb_real = scph10000_ee_wb;
        ee.wh_real = scph10000_ee_wh;
        ee.ww_real = scph10000_ee_ww;
        ee.wd_real = scph10000_ee_wd;
        ee.wq_real = scph10000_ee_wq;

        iop.rb_real = scph10000_iop_rb;
        iop.rh_real = scph10000_iop_rh;
        iop.rw_real = scph10000_iop_rw;
        iop.wb_real = scph10000_iop_wb;
        iop.wh_real = scph10000_iop_wh;
        iop.ww_real = scph10000_iop_ww;
    

        FILE* fp = fopen("roms/ps2-0100j-20000117.bin","rb");
        fread(dev.bios, 1, 0x400000, fp);
        fclose(fp);

        for(int i = 0; i < 1000000; i++)
        {
            ee.tick();
            current_time += ee_atto;
            if(current_time > iop_atto)
            {
                iop.tick();
                current_time -= iop_atto;
            }
        }
    }

    if(model == "scph15000")
    {
        ps2_type = ps2_model::scph15000;

        scph15000 dev;
        ee_cpu ee;
        iop_cpu iop;

        ee.init();
        iop.init();

        dev.ee = &ee;
        dev.iop = &iop;

        dev.init();

        ee.device = &dev;
        iop.device = &dev;

        ee.rb_real = scph15000_ee_rb;
        ee.rh_real = scph15000_ee_rh;
        ee.rw_real = scph15000_ee_rw;
        ee.rd_real = scph15000_ee_rd;
        ee.rq_real = scph15000_ee_rq;
        ee.wb_real = scph15000_ee_wb;
        ee.wh_real = scph15000_ee_wh;
        ee.ww_real = scph15000_ee_ww;
        ee.wd_real = scph15000_ee_wd;
        ee.wq_real = scph15000_ee_wq;

        iop.rb_real = scph15000_iop_rb;
        iop.rh_real = scph15000_iop_rh;
        iop.rw_real = scph15000_iop_rw;
        iop.wb_real = scph15000_iop_wb;
        iop.wh_real = scph15000_iop_wh;
        iop.ww_real = scph15000_iop_ww;

        FILE* fp = fopen("roms/ps2-0101j-20000217.bin","rb");
        fread(dev.bios, 1, 0x400000, fp);
        fclose(fp);

        for(int i = 0; i < 100000; i++)
        {
            ee.tick();
            current_time += ee_atto;
            if(current_time > iop_atto)
            {
                iop.tick();
                current_time -= iop_atto;
            }
        }
    }

    return 0;
}
