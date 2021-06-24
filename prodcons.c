#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <ncurses.h>

sem_t semVazio;           
sem_t semCheio;
sem_t mutex;

pthread_mutex_t mutexBuffer;

int qproduz;
int qconsome;
int TAM_BUFFER;
int *produtos; 
int quantidade = 0; //quantidade de produtos no buffer

//NCURSES
int height, width;
WINDOW *win;


void *produzir(void *args)
{
    while (1)
    {
        int *arg = args;

        //Produção
        int i;
        int x = rand() % 100;

        //Adicionar ao buffer
        sem_wait(&semVazio);                            //retirar buffer do vazio
        pthread_mutex_lock(&mutexBuffer);
        produtos[quantidade] = x;                       //colocar no final do buffer
        quantidade++;
    
        //--------------------PRODUZIDOS--------------------- 
        
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win,12,*arg*3+20," ");
        mvwprintw(win,12,*arg*3+21," ");
        wattroff(win, COLOR_PAIR(1)); 
        wrefresh(win);

        wattron(win, COLOR_PAIR(3));
        mvwprintw(win,11,*arg*3+20,"%d",x);
        wattroff(win, COLOR_PAIR(3)); 
        wrefresh(win);      
        //----------------------------------------------------

        //--------------------PRODUZIR------------------------ 
        for(i=0; i<quantidade;i++){
            wattron(win, COLOR_PAIR(4));
            mvwprintw(win,5,i*3+20," ");
            mvwprintw(win,5,i*3+21," ");
            wattroff(win, COLOR_PAIR(4)); 
            wrefresh(win);

            wattron(win, COLOR_PAIR(3));
            mvwprintw(win,4,i*3+20,"%d",produtos[i]);
            wattroff(win, COLOR_PAIR(3)); 
            wrefresh(win);            
        }
        //----------------------------------------------------    

        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semCheio);                        //colocar buffer cheio
        
        sleep(2);                                   //tempo de produção
    }
}

void *consumir(void *args)
{
    while (1)
    {
        int *arg = args;

        //Consumo
        int j;
        int y;

        //Retirar do buffer
        sem_wait(&semCheio);                    //retirar buffer cheio
        pthread_mutex_lock(&mutexBuffer);
        y = produtos[0];
        for(j=0 ; j!='\n';j++){
            produtos[j]=produtos[j+1];          //buffer tratado como uma lista             
        }        
        quantidade--;

        //--------------------CONSUMIDOS--------------------- 
        wattron(win, COLOR_PAIR(2));
        mvwprintw(win,18,*arg*3+20," ");
        mvwprintw(win,18,*arg*3+21," ");
        wattroff(win, COLOR_PAIR(2)); 
        wrefresh(win);

        wattron(win, COLOR_PAIR(3));
        mvwprintw(win,17,*arg*3+20,"%d",y);
        wattroff(win, COLOR_PAIR(3)); 
        wrefresh(win);            
      
        //----------------------------------------------------

        //--------------------CONSUMIR---------------------
        for(j=0; j<quantidade;j++){
            
            wattron(win, COLOR_PAIR(4));
            mvwprintw(win,5,j*3+20," ");
            mvwprintw(win,5,j*3+21," ");
            wattroff(win, COLOR_PAIR(4)); 
            wrefresh(win);

            wattron(win, COLOR_PAIR(3));
            mvwprintw(win,4,j*3+20,"%d",produtos[j]);
            wattroff(win, COLOR_PAIR(3)); 
            wrefresh(win);            
         }
        //------------------------------------------------ 

        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semVazio);                    //colocar buffer vazio        
       
        sleep(2); //tempo de consumo
      
    }
}

int main(int argc, char *argv[])
{
    initscr(); // Start curses mode

    //Ncurses--------------------------------------
    height=1000;
    width =1000;

    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLUE);
    init_pair(2, COLOR_RED, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4,COLOR_WHITE, COLOR_WHITE);


    win = newwin(height, width, 0, 0);
    refresh();
    //---------------------------------------------

    //Alocação de memória e entradas---------------
    TAM_BUFFER = atoi(argv[1]);
    qproduz = atoi(argv[2]);
    qconsome = atoi(argv[3]);

    produtos = malloc(TAM_BUFFER * sizeof *produtos);
    //-----------------------------------------------

    srand(time(NULL));

    //Uma thread para cada produtor
    pthread_t threads_produtor[qproduz];
    //Uma thread para cada consumidor
    pthread_t threads_consumidor[qconsome];

    int i, arg[TAM_BUFFER]; 

    //---Iniciando a área crítica para exclusão mútua
    sem_init(&semVazio, 0, TAM_BUFFER);  //no inicio temos o tamanho do buffer para ocupar
    sem_init(&semCheio, 0, 0);           //no inicio temos 0 slots que estão ocupados 

    //Legenda
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win,3,4,"Buffer ");
    wattroff(win, COLOR_PAIR(3)); 
    wrefresh(win);

    wattron(win, COLOR_PAIR(4));
    mvwprintw(win,4,4," ");
    mvwprintw(win,5,4," ");
    mvwprintw(win,6,4," ");
    wattroff(win, COLOR_PAIR(4)); 
    wrefresh(win);

    wattron(win, COLOR_PAIR(3));
    mvwprintw(win,10,4,"Produtor ");
    wattroff(win, COLOR_PAIR(3)); 
    wrefresh(win);

    wattron(win, COLOR_PAIR(1));
    mvwprintw(win,11,4," ");
    mvwprintw(win,12,4," ");
    mvwprintw(win,13,4," ");
    wattroff(win, COLOR_PAIR(1)); 
    wrefresh(win);

    wattron(win, COLOR_PAIR(3));
    mvwprintw(win,16,4,"Consumidor ");
    wattroff(win, COLOR_PAIR(3)); 
    wrefresh(win);

    wattron(win, COLOR_PAIR(2));
    mvwprintw(win,17,4," ");
    mvwprintw(win,18,4," ");
    mvwprintw(win,19,4," ");
    wattroff(win, COLOR_PAIR(2)); 
    wrefresh(win);

    sleep(3);

    for (i = 0; i < qproduz; i++)
    {
        arg[i] = i;
        pthread_create(&threads_produtor[i], NULL, produzir, (void *)&arg[i]); //criar as threads para os produtores         
    }

    for (i = 0; i < qconsome; i++)
    {
        arg[i] = i;
        pthread_create(&threads_consumidor[i], NULL, consumir, (void *)&arg[i]); //criar as threads para os consumidores
    }

    for (i = 0; i < qproduz; i++)
    {
        pthread_join(threads_produtor[i], NULL);
    }

    for (i = 0; i < qconsome; i++)
    {
        pthread_join(threads_consumidor[i], NULL);
    }

    sem_destroy(&semVazio);
    sem_destroy(&semCheio);
    pthread_mutex_destroy(&mutexBuffer);

    endwin(); //End cueses mode

    return 0;
}
