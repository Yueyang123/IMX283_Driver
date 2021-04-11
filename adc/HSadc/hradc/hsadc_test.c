/*
 *  yanli@zlg.cn
 *
 *  2015/03/03
 *
 *  hsadc_test.c
 *     Part of DTU-M28x signal board test program for hsadc test.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
 #include <time.h>
#define tag_log(x, args...) fprintf(stdout, "dtu_hsadc_test: "x, ##args)
#define FAILED(x) ((x)<0)
#define SAMPLE_COUNT (0x100)
#define SAMPLE_WORD_SIZE (2) // sample word size in byte, 8-bit mode is 1 byte, 10,12-bit mode are 2 bytes
#define BUF_SIZE (SAMPLE_COUNT * SAMPLE_WORD_SIZE)

struct hsadc_context{
    int fd;
    int testcount;
    pthread_t thread_id;
    char * buf;
};

/*
 * How can we make sure hsadc is ok?
 *
 *        Get value 10 times, if 6 times success continuely, we though it ok.
 *
 */
void *hsadc_thread(void *pArgs)
{
    float iVret = 0;
    int j = 0;
    int ret = 0;
    struct hsadc_context *pContext = (struct hsadc_context*)pArgs;
    unsigned short *pbuf = (unsigned short *)pContext->buf;

    while(pContext->testcount){
        memset(pContext->buf, 0x00, BUF_SIZE);
        ret = read(pContext->fd, pContext->buf, BUF_SIZE);

        if(ret <= 0)
            continue;

        // hsadc work on 12 bits deepth , so Vin = value / 4096 * Vref
        for(j=0; j<(ret>>1); j++){
            iVret += pbuf[j]&0xfff;
        }

        iVret /= (ret>>1);
        iVret = (iVret * 1.8) / 4096.0;
        pContext->testcount --;

        iVret = 0;
        //usleep(300*1000);
    }
    return 0;
}

int hsadc_test(void)
{
    int ret = -1;
    struct hsadc_context context;

    context.fd = open("/dev/mxs-hsadc0", O_RDWR, S_IRUSR|S_IWUSR);
    if(FAILED(context.fd)){
        perror("open hsadc");
        goto fail;
    }

    context.buf = (char*)malloc(BUF_SIZE);
    if(!context.buf){
        goto fail;
    }

    context.testcount = 1000; /* test 10 times */
    ret = pthread_create(&(context.thread_id), NULL, &hsadc_thread, &context);
    if(FAILED(ret)){
        goto fail;
    }

fail:
    if(context.thread_id)
        pthread_join(context.thread_id, 0);

    if(context.fd > 0)
        close(context.fd);

    if(context.buf)
        free(context.buf);

    return 0;
}

int main(int argc, char *argv[])
{
    clock_t start, end;  
    start = clock();           /*记录起始时间*/
	hsadc_test();
    end = clock();
    double seconds  =(double)(end - start)/CLOCKS_PER_SEC;
    printf("Use time is: %.10f\n", seconds);
	return 0; 
}
