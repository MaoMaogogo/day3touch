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
int game_flag = 0;//��Ϊ�ӵ����Ǹñ�����Ϊ1


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
        for (x = 0; x < 800; x++)
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
            for (x = 0; x < 800; x++)
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
                point++;
                mode_y0 = 0;
                color_flag++;
                if (color_flag > 2)
                {
                    color_flag = 0;
                }
            }
            else//��û�ӵ���
            {
                game_flag = 1;
                printf("δ�ӵ���\n");
            }
            printf("point=%d\n", point);
        }
        //���������
        if (x0 == r)
        {
            mode_x0 = 1;
        }
        //�������ұ�
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