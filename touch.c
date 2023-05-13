#define _CRT_SECURE_NO_WARNINGS 1
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <sys/mman.h>
//#include <linux/input.h>
//
//int main()
//{
//
//    // 1.打开event0文件
//    int ts_fd;
//    ts_fd = open("/dev/input/event0", O_RDONLY);
//    if (ts_fd == -1)
//    {
//        printf("open ts failed!\n");
//        return -1;
//    }
//    int x, y;
//    while (1)
//    {
//        // 2.读取event0文件的数据
//        struct input_event ts_buf;
//        read(ts_fd, &ts_buf, sizeof(ts_buf));
//        // 3.判断事件类型，打印出数据
//        if (ts_buf.tybe == EV_ABS && ts_buf.code == ABS_X)
//        {
//            x = ts_b
//                if (ts_buf.tybe == EV_ABS && ts_buf.code == ABS_Y)
//                {
//                    y = ts_buf.value;
//                }
//            printf("(x,y)=(%d,%d)\n", x, y); 
//        }
//    }
//
//
//
//  /*  printf("type = %d\n", ts_buf.type);
//    printf("code = %d\n", ts_buf.code);
//    printf("value = %d\n", ts_buf.value);*/
//
//    // 4.关闭文件
//    //close(ts_fd);
//
//
//    return 0;
//}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <linux/input.h>

int main()
{

    // 1.打开event0文件
    int ts_fd;
    ts_fd = open("/dev/input/event0", O_RDONLY);
    if (ts_fd == -1)
    {
        printf("open ts failed!\n");
        return -1;
    }

    // 2.读取event0文件的数据
    struct input_event ts_buf;

    // 3.判断事件类型，打印出数据
    while (1)
    {
        read(ts_fd, &ts_buf, sizeof(ts_buf));
        int ts_x, ts_y;
        if (ts_buf.type == EV_ABS && ts_buf.code == ABS_X)
        {
            ts_x = ts_buf.value;
            ts_x = ts_x * 800 / 1024;
        }
        if (ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)
        {
            ts_y = ts_buf.value;
            ts_y = ts_y * 480 / 600;
        }
        if (ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH && ts_buf.value == 0)
        {
            printf("(x,y) = (%d,%d)\n", ts_x, ts_y);

        }
    }

    // 4.关闭文件
    close(ts_fd);


    return 0;
}