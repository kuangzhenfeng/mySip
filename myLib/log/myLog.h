#ifndef MY_LOG_H
#define MY_LOG_H

#define CHECK_RETURN(func)                                                                   \
    do                                                                                       \
    {                                                                                        \
        ret = func;                                                                          \
        if(ret < 0)                                                                          \
            printf("%s(%d) error: function return %d!\n", __FILE__, __LINE__, ret);          \
    } while(0)


#endif
