#if !defined(INPUTS_H_)
#define INPUTS_H_

typedef enum
{
    SW_NONE,
    SW_ENTER,
    SW_ENTER_LONG,
    SW_BACK,
    SW_DOWN,
    SW_UP
} Switch_t;

void inputs_init(void);
Switch_t read_switch(void);

#endif // INPUTS_H_
