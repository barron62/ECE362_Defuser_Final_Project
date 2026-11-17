/*

have 3 games: keypad password, button puzzle, and maze puzzle

keypad password puzzle:
need a basic FSM to control sending and recieving from the solver
use uart for this
instruction board commands the entire game, has a master FSM 
that controls the game state and sends commands to the solver board

need the state to determine the what the uart outputs

need substate/stages for the keypad puzzle, button puzzle, and maze puzzle

KEYPAD PUZZLE STATES:
- GENERATE PASSWORD AND STORE IN UART
- SEND SOLVER A READY SEQUENCE OVER UART
- DISPLAY PASSWORD ON INSTRUCTION BOARD
- WAIT FOR SOLVER TO SEND BACK CORRECT PASSWORD OVER UART
- IF CORRECT PASSWORD RECEIVED, SEND SOLVER A SUCCESS SEQUENCE OVER UART
- IF INCORRECT PASSWORD RECEIVED, SEND SOLVER A FAILURE SEQUENCE OVER UART
- IF ATTEMPT LIMIT OR TIME LIMIT REACHED, SEND SOLVER A FAILURE SEQUENCE OVER UART
- GAME OVER STATE

BUTTON PUZZLE STATES:
- GENERATE BUTTON SEQUENCE AND STORE IN UART
- SEND SOLVER A READY SEQUENCE OVER UART
- DISPLAY BUTTON SEQUENCE ON INSTRUCTION BOARD
- WAIT FOR SOLVER TO SEND BACK CORRECT BUTTON SEQUENCE OVER UART
- IF CORRECT BUTTON SEQUENCE RECEIVED, SEND SOLVER A SUCCESS SEQUENCE OVER UART
- IF ATTEMPT LIMIT OR TIME LIMIT REACHED, SEND SOLVER A FAILURE SEQUENCE OVER UART
- GAME OVER STATE

MAZE PUZZLE STATES:
- GENERATE MAZE AND STORE IN UART
- SEND SOLVER A READY SEQUENCE OVER UART
- DISPLAY MAZE ON INSTRUCTION BOARD
- WAIT FOR SOLVER TO SEND BACK CORRECT MAZE SOLUTION OVER UART
- IF CORRECT MAZE SOLUTION RECEIVED, SEND SOLVER A SUCCESS SEQUENCE OVER UART
- IF ATTEMPT LIMIT OR TIME LIMIT REACHED, SEND SOLVER A FAILURE SEQUENCE OVER UART
- GAME OVER STATE


*/


/*
    main handles the wait states

    uart has an array that is global and accessible by the main, main checks that and sets up the interrupt for it

    main should probably also handle timing....

    uart checks corrected, scans for inputs, sends messages to the solver, etc.
*/

#include "uart.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

typedef enum {
    IDLE,
    MAZE,
    KEYPAD,
    BUTTON,
    GAME_FINISH,
    GAME_OVER,
    CHECK_CONN
} uart_state_t;

static uart_state_t game_state = IDLE;
static int output_buffer[REC_BUFFER];
static uint8_t input_buffer;
static bool answer_ready = false;
static bool correct_ack = false;
static bool answer_sent = false;
static bool fail = false;


static void scan_uart()
{

    //need to awknowledge

    if(game_state == GAME_FINISH || game_state == GAME_OVER)
    {
        return;
    }

    if(uart_is_readable(uart0))
    {
        input_buffer = (uint8_t)uart_getc(uart0);

        if(game_state == CHECK_CONN)
        {
            if(rec[0] == READY_SEQUENCE)
            {
                correct_ack = true;
            }
        }
        sleep_ms(10);
        answer_ready = true;
    } 
}


static void send_data(int * data)
{
    uart_write_blocking(uart0, output_buffer, sizeof(output_buffer));
}

void uart_sb_init() 
{
    uart_init(uart0, BAUD_RATE)
    gpio_set_function(UART_IB_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_IB_RX_PIN, GPIO_FUNC_UART);
    irq_set_exclusive_handler(UART0_IRQ, scan_uart);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enabled(uart0, true, false);
}

bool read_answer_ready_sb()
{
    return answer_ready;
}

// cannot send data if the answer_sent wasnt cleared
void send_answer(int * data)
{
    sleep_ms(10);
    if(!answer_sent)
    {
        send_data(data);
        answer_sent = true;
    }
}

bool read_fail(void)
{
    return fail;
}


/*
    update the state of the uart controller
    
    essentially acts like the clock controlling a FSM
*/
void uart_sb_update() 
{
    if(input_buffer == GO_SEQUENCE && answer_ready)
    {
        game_state = GAME_OVER;
    }
    switch(game_state)
    {
        case IDLE:
            game_state = CHECK_CONN;
            break;
        case MAZE:
            if(answer_ready && output_buffer == SUCCESS_SEQUENCE)
            {
                fail = false;
                game_state = GAME_FINISH;
            }
            else if(answer_ready && output_buffer == FAILURE_SEQUENCE)
            {
                fail = true;
            }
            answer_ready = false;
            break;
        case KEYPAD: // first state
            if(answer_ready && output_buffer == SUCCESS_SEQUENCE)
            {
                fail = false;
                game_state = BUTTON;
            }
            else if(answer_ready && output_buffer == FAILURE_SEQUENCE)
            {
                fail = true;
            }
            answer_ready = false;
            break;
        case BUTTON:
            if(answer_ready && output_buffer == SUCCESS_SEQUENCE)
            {
                fail = false;
                game_state = MAZE;
            }
            else if(answer_ready && output_buffer == FAILURE_SEQUENCE)
            {
                fail = true;
            }
            answer_ready = false;
            break;
        case CHECK_CONN:
            if(correct_ack){
                send_data(ACK_SEQUENCE);
                game_state = KEYPAD;
                correct_ack = false;
            }
            answer_ready = false;
            break;
        case GAME_OVER:
            break;
        case GAME_FINISH:
            break;
    }    
}



