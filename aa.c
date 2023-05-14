#define _CRT_SECURE_NO_WARNINGS 1
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <pthread.h>

int show_bmp(char* path, int zs_x, int zs_y, int wide, int high);

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
int score = 0;//游戏分数

char* score_pic[] = { "./0.bmp","./1.bmp","./2.bmp","./3.bmp","./4.bmp","./5.bmp","./6.bmp",
                    "./7.bmp","./8.bmp","./9.bmp" };
//显示分数
int show_score()
{
    while (1)
    {
        if (score / 10 > 0)//两位数
        {
            show_bmp(score_pic[score / 10], 715, 10, 32, 32);//显示十位
            score %= 10;
        }
        else if(score/10 == 0)
        {
            show_bmp(score_pic[score % 10], 753, 10, 32, 32);
            break;
        }
    }
}

/*
 *函数名：show_bmp
 *函数功能：显示bmp图片
 *参数：
 *  path： 图片的路径
 *  zs_x:  图片显示的原点x轴坐标
 *  zs_y:  图片显示的原点y轴坐标
 *  whide: 图片的宽度（单位像素）
 *  high： 图片的高度（单位像素）
*/
int show_bmp(char* path, int zs_x, int zs_y, int wide, int high)
{
    //1.打开bmp文件 LCD屏设备文件
    int bmp_fd;
    bmp_fd = open(path, O_RDWR);
    if (bmp_fd == -1)
    {
        printf("open bmp error\n");
        return -1;
    }

    //2.在bmp文件从头开始偏移54个字节
    lseek(bmp_fd, 54, SEEK_SET);

    //3.读取bmp文件中的像素数据
    char bmp_buf[wide * high * 3];//定义一个字符型数组用来存放读取到的wide*high个像素点
    read(bmp_fd, bmp_buf, wide * high * 3);//定义一个新的数组用来存放wide*high像素点

    //4.进行数据处理
    int n;
    int lcd_buf[wide * high];
    for (n = 0; n < wide * high; n++)
    {
        lcd_buf[n] = bmp_buf[3 * n] | bmp_buf[3 * n + 1] << 8 | bmp_buf[3 * n + 2] << 16;
    }



    int* new_p = p + 800 * zs_y + zs_x;

    //6.将数据写入映射的地址
    int x, y;
    for (y = 0; y < high; y++)
    {
        for (x = 0; x < wide; x++)
        {
            *(new_p + 800 * ((high - 1) - y) + x) = lcd_buf[wide * y + x];
        }
    }

    //7.关闭bmp文件 LCD屏设备文件
    close(bmp_fd);

    return 0;
}

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
        for (x = 0; x < 700; x++)
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
            for (x = 0; x < 700; x++)
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
                mode_y0 = 0;
                color_flag++;
                if (color_flag > 2)
                {
                    color_flag = 0;
                }
                score++;
                show_score();
            }
            else//板没接到球
            {
                game_flag = 1;
                printf("未接到球\n");
            }
        }
        //球碰到左边
        if (x0 == r)
        {
            mode_x0 = 1;
        }
        //球碰到右边
        if (x0 == 699 - r)
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
    show_bmp("./cc.bmp", 0, 0, 800, 480);
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