row = 16
col = 1
count = 1
num = 0
with open("nxfonts_bitmaps_cn.c","a") as fw:
    while num < 6768:
        h = 0xa0 + row
        l = 0xa0 + col
        data = (h<<8) + l
        fw.write("    { {2,15,16,1,10,0}, g_bitmap_%x },\n" % data);
        count += 1
        if(count > 94):
            count = 1
            row += 1 
            col = 1
        else:
            col += 1
        num += 1
