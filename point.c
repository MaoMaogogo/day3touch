#define _CRT_SECURE_NO_WARNINGS 1
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <pthread.h>

int point;

int ts_fd;
int lcd_fd;
int ts_x, ts_y;
int* p;
struct input_event ts_buf;
int color[] = { 0x00ff0000, 0x0000ff00, 0x000000ff };
int white = 0x00ffffff;
int blue = 0x000000ff;
int color_flag = 0;
int game_flag = 0;//当为接到球是该变量置为1


//设备初始化函数
int devive_init()
{
    // 1.打开fb0、event0文件
    lcd_fd = open("/dev/fb0", O_RDWR);
    if (lcd_fd == -1)
    {
        printf("open fb0 failed!\n");
        return -1;
    }

    ts_fd = open("/dev/input/event0", O_RDONLY);
    if (ts_fd == -1)
    {
        printf("open ts failed!\n");
        return -1;
    }
    int blue = 0x000000ff, white = 0x00ffffff;
    //2.内存映射
    p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
}

//设备关闭函数
int device_close()
{
    munmap(p, 800 * 480 * 4);
    close(ts_fd);
    close(lcd_fd);
}

//获取触摸坐标函数
void get_xy()
{
    read(ts_fd, &ts_buf, sizeof(ts_buf));
    // 3.判断事件类型，打印出数据
    if (ts_buf.type == EV_ABS && ts_buf.code == ABS_X)//判断X轴是否发生绝对位移事件
    {
        ts_x = ts_buf.value;
        ts_x = ts_x * 800 / 1024;
    }
    if (ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)//判断Y轴是否发生绝对位移事件
    {
        ts_y = ts_buf.value;
        ts_y = ts_y * 480 / 600;
    }

    if (ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH && ts_buf.value == 0)//松手检测
    {
        printf("(x,y) = (%d, %d)\n", ts_x, ts_y);
    }
}

//画板函数
int drawplate()
{
    //画板
    int x, y;
    for (y = 430; y < 480; y++)
    {
        for (x = 0; x < 800; x++)
        {
            if (x<ts_x + 50 && x>ts_x - 50)//判断画板的位置
            {
                *(p + 800 * y + x) = blue;//画蓝色的板
            }
            else
            {

                *(p + 800 * y + x) = white;//板外填充白色
            }
        }
    }
}


//画球函数
void* drawcircle(void* arg)
{
    //4.画圆球
    int x0 = 400, y0 = 240, r = 60;
    int x, y;
    int mode_x0 = 0, mode_y0 = 0;//值为0时，--；值为1时，++。
    int count = 1;
    while (1)
    {
        for (y = 0; y < 430; y++)
        {
            for (x = 0; x < 800; x++)
            {
                if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= r * r)//判断是否在圆内
                {
                    //write(lcd_fd, &green, 4);//圆内填充绿色
                    *(p + 800 * y + x) = color[color_flag];
                }
                else
                {
                    //write(lcd_fd, &white, 4);//圆外填充白色
                    *(p + 800 * y + x) = white;
                }
            }
        }
        
        //球碰到上边
        if (y0 == r)
        {
            mode_y0 = 1;
        }
        //球碰到下边
        if (y0 == 430 - r)
        {
            if (x0 >= ts_x - 50 && x0 <= ts_x + 50)//板接到球
            {
                point++;
                mode_y0 = 0;
                color_flag++;
                if (color_flag > 2)
                {
                    color_flag = 0;
                }
            }
            else//板没接到球
            {
                game_flag = 1;
                printf("未接到球\n");
            }
            printf("point=%d\n", point);
        }
        //球碰到左边
        if (x0 == r)
        {
            mode_x0 = 1;
        }
        //球碰到右边
        if (x0 == 799 - r)
        {
            mode_x0 = 0;
        }

        if (mode_x0 == 0)  x0--;
        if (mode_x0 == 1)  x0++;
        if (mode_y0 == 0)  y0--;
        if (mode_y0 == 1)  y0++;

    }
}

int main()
{
    devive_init();
    pthread_t tid;
    pthread_create(&tid, NULL, drawcircle, NULL);

    while (1)
    {
        get_xy();
        drawplate();
        if (game_flag == 1)
        {
            return -1;
        }
    }

    device_close();
    return 0;
}