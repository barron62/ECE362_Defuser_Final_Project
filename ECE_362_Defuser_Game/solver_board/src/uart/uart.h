#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define FAILURE_SEQUENCE 0x55
#define BAUD_RATE 115200
#define UART_IB_TX_PIN 0
#define UART_IB_RX_PIN 1
#define UART_IB_IRQ UART0_IRQ
#define SUCCESS_SEQUENCE 0xAA
#define READY_SEQUENCE 0x67
#define PASS_LENGTH 16
#define SEQ_LENGTH 8
#define REC_BUFFER 16
#define PATH_LEN 16
#define ACK_SEQUENCE 0x28
#define GO_SEQUENCE 0x96

void uart_ib_init(void);

void uart_ib_update(void);

void game_over();

bool read_fail_status();

bool read_answer_ready();

void uart_sb_update();

bool read_fail(void);

void send_answer(int * data);

bool read_answer_ready_sb();

void uart_sb_init();
