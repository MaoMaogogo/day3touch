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
int game_flag = 0;//��Ϊ�ӵ����Ǹñ�����Ϊ1
int score = 0;//��Ϸ����

char* score_pic[] = { "./0.bmp","./1.bmp","./2.bmp","./3.bmp","./4.bmp","./5.bmp","./6.bmp",
                    "./7.bmp","./8.bmp","./9.bmp" };
//��ʾ����
int show_score()
{
    while (1)
    {
        if (score / 10 > 0)//��λ��
        {
            show_bmp(score_pic[score / 10], 715, 10, 32, 32);//��ʾʮλ
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
 *��������show_bmp
 *�������ܣ���ʾbmpͼƬ
 *������
 *  path�� ͼƬ��·��
 *  zs_x:  ͼƬ��ʾ��ԭ��x������
 *  zs_y:  ͼƬ��ʾ��ԭ��y������
 *  whide: ͼƬ�Ŀ�ȣ���λ���أ�
 *  high�� ͼƬ�ĸ߶ȣ���λ���أ�
*/
int show_bmp(char* path, int zs_x, int zs_y, int wide, int high)
{
    //1.��bmp�ļ� LCD���豸�ļ�
    int bmp_fd;
    bmp_fd = open(path, O_RDWR);
    if (bmp_fd == -1)
    {
        printf("open bmp error\n");
        return -1;
    }

    //2.��bmp�ļ���ͷ��ʼƫ��54���ֽ�
    lseek(bmp_fd, 54, SEEK_SET);

    //3.��ȡbmp�ļ��е���������
    char bmp_buf[wide * high * 3];//����һ���ַ�������������Ŷ�ȡ����wide*high�����ص�
    read(bmp_fd, bmp_buf, wide * high * 3);//����һ���µ������������wide*high���ص�

    //4.�������ݴ���
    int n;
    int lcd_buf[wide * high];
    for (n = 0; n < wide * high; n++)
    {
        lcd_buf[n] = bmp_buf[3 * n] | bmp_buf[3 * n + 1] << 8 | bmp_buf[3 * n + 2] << 16;
    }



    int* new_p = p + 800 * zs_y + zs_x;

    //6.������д��ӳ��ĵ�ַ
    int x, y;
    for (y = 0; y < high; y++)
    {
        for (x = 0; x < wide; x++)
        {
            *(new_p + 800 * ((high - 1) - y) + x) = lcd_buf[wide * y + x];
        }
    }

    //7.�ر�bmp�ļ� LCD���豸�ļ�
    close(bmp_fd);

    return 0;
}

//�豸��ʼ������
int devive_init()
{
    // 1.��fb0��event0�ļ�
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
    //2.�ڴ�ӳ��
    p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
}

//�豸�رպ���
int device_close()
{
    munmap(p, 800 * 480 * 4);
    close(ts_fd);
    close(lcd_fd);
}

//��ȡ�������꺯��
void get_xy()
{
    read(ts_fd, &ts_buf, sizeof(ts_buf));
    // 3.�ж��¼����ͣ���ӡ������
    if (ts_buf.type == EV_ABS && ts_buf.code == ABS_X)//�ж�X���Ƿ�������λ���¼�
    {
        ts_x = ts_buf.value;
        ts_x = ts_x * 800 / 1024;
    }
    if (ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)//�ж�Y���Ƿ�������λ���¼�
    {
        ts_y = ts_buf.value;
        ts_y = ts_y * 480 / 600;
    }

    if (ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH && ts_buf.value == 0)//���ּ��
    {
        printf("(x,y) = (%d, %d)\n", ts_x, ts_y);
    }
}

//���庯��
int drawplate()
{
    //����
    int x, y;
    for (y = 430; y < 480; y++)
    {
        for (x = 0; x < 700; x++)
        {
            if (x<ts_x + 50 && x>ts_x - 50)//�жϻ����λ��
            {
                *(p + 800 * y + x) = blue;//����ɫ�İ�
            }
            else
            {
                *(p + 800 * y + x) = white;//��������ɫ
            }
        }
    }
}


//������
void* drawcircle(void* arg)
{
    //4.��Բ��
    int x0 = 400, y0 = 240, r = 60;
    int x, y;
    int mode_x0 = 0, mode_y0 = 0;//ֵΪ0ʱ��--��ֵΪ1ʱ��++��
    int count = 1;
    while (1)
    {
        for (y = 0; y < 430; y++)
        {
            for (x = 0; x < 700; x++)
            {
                if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= r * r)//�ж��Ƿ���Բ��
                {
                    //write(lcd_fd, &green, 4);//Բ�������ɫ
                    *(p + 800 * y + x) = color[color_flag];
                }
                else
                {
                    //write(lcd_fd, &white, 4);//Բ������ɫ
                    *(p + 800 * y + x) = white;
                }
            }
        }

        //�������ϱ�
        if (y0 == r)
        {
            mode_y0 = 1;
        }
        //�������±�
        if (y0 == 430 - r)
        {
            if (x0 >= ts_x - 50 && x0 <= ts_x + 50)//��ӵ���
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
            else//��û�ӵ���
            {
                game_flag = 1;
                printf("δ�ӵ���\n");
            }
        }
        //���������
        if (x0 == r)
        {
            mode_x0 = 1;
        }
        //�������ұ�
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