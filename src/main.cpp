#ifdef __APPLE__
#error "Apple is vehemently anti-dev, and refuses to support modern OpenGL, OpenCL, or any Vulkan natively. Therefore, this software does not support MacOS or Apple."
#endif

#include "common.h"
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
        ee.rw_real = scph10000_ee_rw;
        ee.wb_real = scph10000_ee_wb;
        ee.ww_real = scph10000_ee_ww;
        ee.wd_real = scph10000_ee_wd;

        iop.rb_real = scph10000_iop_rb;
        iop.rw_real = scph10000_iop_rw;
        iop.wb_real = scph10000_iop_wb;
        iop.ww_real = scph10000_iop_ww;
    

        FILE* fp = fopen("roms/ps2-0100j-20000117.bin","rb");
        fread(dev.bios, 1, 0x400000, fp);
        fclose(fp);

        for(int i = 0; i < 500; i++)
        {
            ee.tick();
            iop.tick();
            dev.tick();
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
        ee.rw_real = scph15000_ee_rw;
        ee.wb_real = scph15000_ee_wb;
        ee.ww_real = scph15000_ee_ww;
        ee.wd_real = scph15000_ee_wd;

        iop.rb_real = scph15000_iop_rb;
        iop.rw_real = scph15000_iop_rw;
        iop.wb_real = scph15000_iop_wb;
        iop.ww_real = scph15000_iop_ww;

        FILE* fp = fopen("roms/ps2-0101j-20000217.bin","rb");
        fread(dev.bios, 1, 0x400000, fp);
        fclose(fp);

        for(int i = 0; i < 100; i++)
        {
            ee.tick();
            iop.tick();
            dev.tick();
        }
    }

    return 0;
}
