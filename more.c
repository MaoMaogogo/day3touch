#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>

// ���߳�������
void* DoSomeThing(void* arg)
{
    while (1)
    {
        get_xy();
                for (y = 430; y < 480; y++)
                {
                    for (x = 0; x < 800; x++)
                    {
                        if (x<ts_x + 50 && x>ts_x - 50)//�ж��Ƿ���Բ��
                        {
                            //write(lcd_fd, &green, 4);//Բ�������ɫ
                            *(p + 800 * y + x) = blue;
                        }
                        else
                        {
                            //write(lcd_fd, &white, 4);//Բ������ɫ
                            *(p + 800 * y + x) = wirte;
                        }
                    }
                }
 
        sleep(1);
    }
}

int main(int argc, char const* argv[])
{
    // pthread_t ��Linuxƽ̨��unsigned long
    pthread_t tid;

    // �����߳�  
    pthread_create(&tid, NULL, DoSomeThing, NULL);

    while (1)
    {
		//#define _CRT_SECURE_NO_WARNINGS 1
//#include<sys/stat.h>
//#include<fcntl.h>
//#include<stdio.h>
//#include<unistd.h>
//#include<string.h>
//#include<sys/mman.h>


////��Բ��
//int main()
//{
//	//1.��fb0�ļ�
//	int lcd_fd = 0;
//	lcd_fd = open("/dev/fb0", O_RDWR);
//	if (lcd_fd == -1)
//	{
//		printf("open fb0 failed!\n");
//	}
//
//	//2.׼����ɫ����
//	int black = 0x00000000;//ARGB ��rgb͸��
//	int red = 0x00ff0000;
//	int yellow = 0x00ffff00;
//	int white = 0x00000000;
//
//	//3.��Բ��
//	int x = 0;
//	int y = 0;
//	int x0 = 400;
//	int y0 = 480;
//	int r = 120;
//
//	for (y = 0; y < 480; y++)
//	{
//		for (x = 0; x < 800; x++)
//		{
//			if ((x-x0)*(x-x0)+(y-y0)*(y-y0)<=r*r)//Բ��Ⱦɫ
//			{
//				write(lcd_fd, &yellow, 4);
//			}
//			else
//			{
//				write(lcd_fd, &white, 4);
//			}
//		}
//	}
//
//	//4.�ر��ļ�
//	close(lcd_fd);
//	return 0;
//}



		int color[] = { 0x00ff0000,0x0000ff00,0x000000ff };
		int i = 0;

		int main()
		{
			int lcd_fd;
			lcd_fd = open("/dev/fb0", O_RDWR);
			if (lcd_fd == -1)
			{
				printf("open fb0 failed!\n");
				return -1;
			}

			int* p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);//�ڴ�ӳ�� 

			int red = 0x00ff0000;
			int green = 0x0000ff00;
			int blue = 0x000000ff;
			int white = 0x00ffffff;//������ɫ���� 

			int x0 = 400, y0 = 240, r = 60;//Բ�ġ��뾶 
			int x, y;
			int mode_x0 = 0, mode_y0 = 0;
			while (1)
			{


				for (y = 0; y < 480; y++)
				{
					for (x = 0; x < 800; x++)
					{
						if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= r * r)//�ж��Ƿ���Բ�� 
						{
							*(p + 800 * y + x) = color[i];
						}
						else
						{
							*(p + 800 * y + x) = white;
						}
					}
				}


				if (y0 == r)//�ϱ�
				{
					mode_y0 = 1;
					i++;
					if (i > 2)
					{
						i = 0;
					}
				}

				if (y0 == 479 - r)//�±� 
				{
					mode_y0 = 0;
					i++;
					if (i > 2)
					{
						i = 0;
					}

				}
				if (x0 == r)//��� 
				{
					mode_x0 = 1;
					i++;
					if (i > 2)
					{
						i = 0;
					}
				}
				if (x0 == 799 - r)
				{
					mode_x0 = 0;
					i++;
					if (i > 2)
					{
						i = 0;
					}ci
				}
				if (mode_x0 == 0) x0--;
				if (mode_x0 == 1) x0++;
				if (mode_y0 == 0)  y0--;
				if (mode_y0 == 1) y0++;
			}

			munmap(p, 800 * 480 * 4);//ȡ��ӳ�� 

			close(lcd_fd);//�ر��ļ� 
			return 0;


		}

        printf("���߳�\n");
        sleep(1);
    }

    return 0;
}

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <sys/mman.h>
//#include <linux/input.h>
//
//int ts_fd;
//int lcd_fd;
//int ts_x, ts_y;
//struct input_event ts_buf;
//
//void get_xy()
//{
//    read(ts_fd, &ts_buf, sizeof(ts_buf));
//    if (ts_buf.type == EV_ABS && ts_buf.code == ABS_X)//X���ӡ
//    {
//        ts_x = ts_buf.value;
//        ts_x = ts_x * 800 / 1024;
//    }
//    if (ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)//Y���ӡ
//    {
//        ts_y = ts_buf.value;
//        ts_y = ts_y * 480 / 600;
//    }
//    if (ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH && ts_buf.value == 0)//���ּ��
//    {
//        printf("(x,y) = (%d, %d)\n", ts_x, ts_y);
//    }
//}
//
//int main()
//{
//    // 1.��event0�ļ�
//    lcd_fd = open("/dev/fb0", O_RDWR);
//    if (lcd_fd == -1)
//    {
//        printf("open fb0 failed\n");
//        return -1;
//    }
//
//    ts_fd = open("/dev/input/event0", O_RDONLY);
//    if (ts_fd == -1)
//    {
//        printf("open ts failed!\n");
//        return -1;
//    }
//
//    int blue = 0x000000ff;
//    int wirte = 0x00ffffff;
//    int x, y;
//    int* p = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
//
//    while (1)
//    {
//        get_xy();
//        for (y = 430; y < 480; y++)
//        {
//            for (x = 0; x < 800; x++)
//            {
//                if (x<ts_x + 50 && x>ts_x - 50)//�ж��Ƿ���Բ��
//                {
//                    //write(lcd_fd, &green, 4);//Բ�������ɫ
//                    *(p + 800 * y + x) = blue;
//                }
//                else
//                {
//                    //write(lcd_fd, &white, 4);//Բ������ɫ
//                    *(p + 800 * y + x) = wirte;
//                }
//            }
//        }
//    }
//    // 4.�ر��ļ�
//    close(ts_fd);
//    close(lcd_fd);
//    return 0;
//}