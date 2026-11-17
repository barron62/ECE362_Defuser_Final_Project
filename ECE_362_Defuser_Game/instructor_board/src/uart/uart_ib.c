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
static int password[PASS_LENGTH];
static int rec[REC_BUFFER];
static int sequence[SEQ_LENGTH];
static volatile bool correct = false;
static int path[PATH_LEN];

static volatile bool answer_ready = false;

// read only getter for password
const int *get_ib_pass(void) 
{
    return password;
}

// read only getter for sequence
const int *get_ib_seq(void) 
{
    return sequence;
}

/*
    scans the uart tx for the output from the solver board

    checks the game state and compares the received data to the expected data

    return: void
    input: void
*/
static void scan_uart()
{

    //need to awknowledge

    if(game_state == IDLE || game_state == GAME_FINISH || game_state == GAME_OVER)
    {
        return;
    }

    if(correct = false && uart_is_readable(uart0))
    {
        
        correct = true;
        uart_read_blocking(uart0, rec, ceil(REC_BUFFER / 8.0));

        switch(game_state)
        {
            case: MAZE
                for(int i = 0; i < PATH_LEN; i++)
                {
                    if(rec[i] != path[i])
                    {
                        correct = false;
                        break;
                    }
                }
                sleep_ms(10);
                answer_ready = false;
                break;
            case: BUTTON
                for(int i = 0; i < PASS_LENGTH; i++)
                {
                    if(rec[i] != password[i])
                    {
                        correct = false;
                        break;
                    }
                }
                sleep_ms(10);
                answer_ready = false;
                break;
            case: KEYPAD
                for(int i = 0; i < SEQ_LENGTH; i++)
                {
                    if(rec[i] != sequence[i])
                    {
                        correct = false;
                        break;
                    }
                }
                sleep_ms(10);
                answer_ready = false;
                break;
            case: CHECK_CONN
                if(rec[0] != ACK_SEQUENCE)
                {
                    correct = false;
                }
                sleep_ms(10);
                answer_ready = false;
                break;
            default:
                break;
        }
    } 
}

/*
    generates a random PASS_LENGTH long password

    return: void
    input: void
*/
static void generate_password() 
{
    for(int i = 0; i < PASS_LENGTH; i++)
    {
        password = rand() % 10;
    }
}

static void generate_maze()
{

}

/*
    generates a random SEQ_LENGTH long button sequence
    buttons are bound to 1-4.

    return: void
    input: void
*/

static void generate_sequence()
{
    for(int i = 0; i < SEQ_LENGTH; i++)
    {
        password = (rand() % 4) + 1;
    }
}

static void send_data(uint8_t data)
{
    uart_putc_raw(uart0, data);
}

void uart_ib_init() 
{
    uart_init(uart0, BAUD_RATE)
    gpio_set_function(UART_IB_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_IB_RX_PIN, GPIO_FUNC_UART);
    generate_password();
    generate_sequence();
    irq_set_exclusive_handler(UART0_IRQ, scan_uart);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enabled(uart0, true, false);
}

void game_over()
{
    game_state = GAME_OVER;
    send_data(FAILURE_SEQUENCE);
    sleep_ms(10);
    return;
}

bool read_fail_status()
{
    bool ret_correct = correct;
    correct = false;
    return ret_correct;
}

bool read_answer_ready()
{
    return answer_ready;
}


/*
    update the state of the uart controller
    
    essentially acts like the clock controlling a FSM
*/
void uart_ib_update() 
{
    switch(game_state)
    {
        case IDLE:
            send_data(READY_SEQUENCE);
            game_state = CHECK_CONN;
            break;
        case MAZE:
            if(correct){
                send_data(SUCCESS_SEQUENCE);
                game_state = GAME_FINISH;
            }
            else{
                send_data(FAILURE_SEQUENCE);
            }
            answer_ready = false;
            break;
        case KEYPAD: // first state
            if(correct){
                send_data(SUCCESS_SEQUENCE);
                game_state = BUTTON;
            }
            else{
                send_data(FAILURE_SEQUENCE);
            }
            answer_ready = false;
            break;
        case BUTTON:
            if(correct){
                send_data(SUCCESS_SEQUENCE);
                game_state = MAZE;
            }
            else{
                send_data(FAILURE_SEQUENCE);
            }
            answer_ready = false;
            break;
        case CHECK_CONN:
            if(correct){
                game_state = KEYPAD;
            }
            break;
        case GAME_OVER:
            break;
        case GAME_FINISH:
            break;
    }    
}



