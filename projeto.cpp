#include <SDL2/SDL.h>  // sdl2
#include <pthread.h>   // threads
#include <semaphore.h> // semaphore
#include <unistd.h>    // sleep,
#include <cstdlib>     // rand
#include <ctime>       // seed,
#include <queue>       // buffer

#define ROBOTS_AMOUNT 3
#define SOURCES_AMOUNT ROBOTS_AMOUNT
#define nullptr NULL
#define error_on_initialize()                                  \
  {                                                            \
    printf("Error on initialize. Code: %s\n", SDL_GetError()); \
  }

const int BOARD_Y = 600;
const int BOARD_X = 800;
const int ROBOT_HEIGHT = 20, ROBOT_WIDTH = 20;
const int SCREEN_WIDTH = BOARD_X + ROBOT_HEIGHT,
          SCREEN_HEIGHT = BOARD_Y + ROBOT_WIDTH;
const int TIME_MIN = 0;
const int TIME_MAX = 4;
const int TIME_EQUIPAMENTS[ROBOTS_AMOUNT] = {0, 50, 0};
const int BUFFER_SIZE_MAX = 100;
int aux_iterator_amount[3] = {0, 1, 2};

struct object
{
  /**
   * @brief Struct used in buffer
   **/
  int x, y, robot, source;
  object(int x, int y, int robot, int source)
  {
    this->x = x;
    this->y = y;
    this->robot = robot;
    this->source = source;
  }
};
struct
{
  /**
   * @brief Structure used in robots processing
   **/
  bool mark_equipament[SOURCES_AMOUNT];
  int position[SOURCES_AMOUNT][2];
} robots[ROBOTS_AMOUNT];
struct
{
  Uint8 r, g, b, a;
} colors[ROBOTS_AMOUNT + 1] = {
    {37, 85, 179, 1}, {148, 71, 194, 1}, {49, 106, 79, 1}, {30, 30, 30, 1}};

std::queue<object> buffer;
pthread_t sr_threads[SOURCES_AMOUNT], sd_threads[SOURCES_AMOUNT], d_thread,
    mr_thread;
pthread_mutex_t lock_aux_iterator = PTHREAD_MUTEX_INITIALIZER,
                lock_buffer = PTHREAD_MUTEX_INITIALIZER,
                lock_draw = PTHREAD_MUTEX_INITIALIZER,
                lock_stop = PTHREAD_MUTEX_INITIALIZER;
int STOP_THREADS = false;
SDL_Rect robots_to_draw[3] = {{0, 0, ROBOT_WIDTH, ROBOT_HEIGHT},
                              {0, 0, ROBOT_WIDTH, ROBOT_HEIGHT},
                              {0, 0, ROBOT_WIDTH, ROBOT_HEIGHT}};
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

void set_stop_value(int flag_value);
object pos_rand(int robot, int source);
void *source_rand(void *source);
void *make_robots(void *);
void *set_to_draw(void *robot);
void *draw(void *);
void init_all();
void create_threads();
void join_threads();
void end_threads();
bool sdl_init_start();
bool sdl_window_start();
bool sdl_render_start();
void sdl_close();
void sdl_render_clear();
void sdl_render_update();

int main()
{
  if (!sdl_init_start())
    return EXIT_FAILURE;
  if (!sdl_window_start())
    return EXIT_FAILURE;
  if (!sdl_render_start())
    return EXIT_FAILURE;

  SDL_Event window_event;

  sdl_render_clear();
  sdl_render_update();

  init_all();
  create_threads();

  while (1)
  {
    if (SDL_PollEvent(&window_event))
      if (SDL_QUIT == window_event.type)
      {
        end_threads();
        break;
      }
  }
  sdl_close();
  join_threads();
  return EXIT_SUCCESS;
}

void set_stop_value(int flag_value)
{
  /**
   * @brief Set value to flag STOP_THREADS
   * @param {int} flag_value. True for stop all threads
   **/
  pthread_mutex_lock(&lock_stop);
  STOP_THREADS = flag_value;
  pthread_mutex_unlock(&lock_stop);
}
object pos_rand(int robot, int source)
{
  /**
   * @brief Rand X,Y position varing into interval [-10,10] from last position
   * in draw
   * @param {int} robot. Rand to this robot
   * @param {int} source. Rand to this source
   * @return {object} An object with (X,Y,ROBOT,SOURCE)
   **/
  int x = (rand() % 21) - 10;
  int y = (rand() % 21) - 10;
  return object(x, y, robot, source);
}

void *source_rand(void *source)
{
  /**
   * @brief Rand X,Y, and Robot to {source}
   * @param {int} source. Source font to rand
   **/
  int isource = *((int *)source);
  int robot;

  while (1)
  {
    pthread_mutex_lock(&lock_stop);
    if (STOP_THREADS)
    {
      pthread_mutex_unlock(&lock_stop);
      break;
    }
    pthread_mutex_unlock(&lock_stop);

    robot = rand() % ROBOTS_AMOUNT;

    for (int i = 0; i < ROBOTS_AMOUNT; robot++, i++)
    {
      usleep(1000 * TIME_EQUIPAMENTS[isource]);

      pthread_mutex_lock(&lock_buffer);
      if (buffer.size() <= BUFFER_SIZE_MAX)
        buffer.push(pos_rand(robot % ROBOTS_AMOUNT, isource));
      //   sem_post(&buffer_count);
      pthread_mutex_unlock(&lock_buffer);
    }
  }
  return NULL;
}
void *make_robots(void *)
{
  /**
   * @brief Push from buffer and insert into robots,
   * mark source position, and also pop from buffer
   */
  while (1)
  {
    pthread_mutex_lock(&lock_stop);
    if (STOP_THREADS)
    {
      pthread_mutex_unlock(&lock_stop);
      break;
    }
    pthread_mutex_unlock(&lock_stop);

    // sem_wait(&buffer_count);

    pthread_mutex_lock(&lock_buffer);
    if (!buffer.empty())
    {
      robots[buffer.front().robot].position[buffer.front().source][0] =
          buffer.front().x;
      robots[buffer.front().robot].position[buffer.front().source][1] =
          buffer.front().y;
      robots[buffer.front().robot].mark_equipament[buffer.front().source] = 1;
      buffer.pop();
    }
    pthread_mutex_unlock(&lock_buffer);
  }
  return NULL;
}
void *set_to_draw(void *robot)
{
  /**
   * @brief Make robot ant put it on draw buffer
   * Also, mark off source
   * @param {int} robot. Robot id
   **/
  int irobot = *((int *)robot);
  int x, y;

  while (1)
  {
    pthread_mutex_lock(&lock_stop);
    if (STOP_THREADS)
    {
      pthread_mutex_unlock(&lock_stop);
      break;
    }
    pthread_mutex_unlock(&lock_stop);

    if (robots[irobot].mark_equipament[0] & robots[irobot].mark_equipament[1] &
        robots[irobot].mark_equipament[2])
    {
      x = (int)((robots[irobot].position[0][0] + robots[irobot].position[1][0] +
                 robots[irobot].position[2][0]) /
                ROBOTS_AMOUNT);
      y = (int)((robots[irobot].position[0][1] + robots[irobot].position[1][1] +
                 robots[irobot].position[2][1]) /
                ROBOTS_AMOUNT);

      pthread_mutex_lock(&lock_draw);
      if (((robots_to_draw[irobot].x + x) >= 0) and
          ((robots_to_draw[irobot].x + x) <= BOARD_X))
        robots_to_draw[irobot].x += x;

      if (((robots_to_draw[irobot].y + y) >= 0) and
          ((robots_to_draw[irobot].y + y) <= BOARD_Y))
        robots_to_draw[irobot].y += y;

      robots[irobot].mark_equipament[0] = robots[irobot].mark_equipament[1] =
          robots[irobot].mark_equipament[2] = 0;
      pthread_mutex_unlock(&lock_draw);
    }
  }
  return NULL;
}
void *draw(void *)
{
  /**
   * @brief Draw function. Function responsible
   * for draw the robots
   **/

  while (1)
  {
    pthread_mutex_lock(&lock_stop);
    if (STOP_THREADS)
    {
      pthread_mutex_unlock(&lock_stop);
      break;
    }
    pthread_mutex_unlock(&lock_stop);

    pthread_mutex_lock(&lock_draw);
    sdl_render_clear();
    for (int i = 0; i < ROBOTS_AMOUNT; i++)
    {
      SDL_SetRenderDrawColor(renderer, colors[i].r, colors[i].g, colors[i].b,
                             colors[i].a);
      SDL_RenderFillRect(renderer, &robots_to_draw[i]);
    }
    sdl_render_update();
    // SDL_Delay(6);  // Add a 16msec delay to make our game run at ~60 fps
    pthread_mutex_unlock(&lock_draw);
  }
  return NULL; // Even if it returns nothing, it is necessary because of the arguments of the pthread
}

void init_all()
{
  /**
   * @brief Start vectors and srand
   **/
  srand(time(NULL));
  // Randomize starting
  for (int i = 0, aux; i < ROBOTS_AMOUNT; i++)
  {
    robots_to_draw[i].x = (rand() % BOARD_X - 1) + 1;
    robots_to_draw[i].y = (rand() % BOARD_Y - 1) + 1;

    robots[i].mark_equipament[0] = robots[i].mark_equipament[1] =
        robots[i].mark_equipament[2] = 0;
    robots[i].position[0][0] = robots[i].position[0][1] =
        robots[i].position[2][0] = robots[i].position[2][1] =
            robots[i].position[3][0] = robots[i].position[3][1] = 0;
  }
}

void create_threads()
{
  /**
   * @brief It create the threads
   **/
  for (int i = 0; i < SOURCES_AMOUNT; i++)
  {
    pthread_mutex_lock(&lock_aux_iterator);

    if (pthread_create(&sr_threads[i], NULL, &source_rand,
                       &aux_iterator_amount[i]))
      printf("Error in create source thread[%d]\n", i);
    if (pthread_create(&sd_threads[i], NULL, &set_to_draw,
                       &aux_iterator_amount[i]))
      printf("Error in create set to draw thread[%d]\n", i);

    pthread_mutex_unlock(&lock_aux_iterator);
  }
  if (pthread_create(&mr_thread, NULL, &make_robots, NULL))
    printf("Error in create make_robots thread\n");
  if (pthread_create(&d_thread, NULL, &draw, NULL))
    printf("Error in create draw thread\n");
}
void join_threads()
{
  /**
   * @brief Join the threads
   **/
  // printf("\n\n\t Joining threads.\n");
  for (int i = 0; i < SOURCES_AMOUNT; i++)
  {
    pthread_mutex_lock(&lock_aux_iterator);

    pthread_join(sr_threads[i], NULL);
    pthread_join(sd_threads[i], NULL);

    pthread_mutex_unlock(&lock_aux_iterator);
  }
  pthread_join(mr_thread, NULL);
  pthread_join(d_thread, NULL);
}
void end_threads()
{
  /**
   * @brief Force threads exit
   **/
  sleep(3);
  pthread_mutex_lock(&lock_stop);
  STOP_THREADS = true;
  pthread_mutex_unlock(&lock_stop);
}

bool sdl_init_start()
{
  /**
   * @brief Tenta iniciar o SDL video.
   * Isto e, a biblioteca de exibicao
   * tenta encontrar um driver de video
   * @return {bool} True para sucesso
   *
   */
  if (!SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    error_on_initialize();
    return 0;
  }
  return 1;
}
bool sdl_window_start()
{
  /**
   * @brief Tenta criar uma janela
   * usada na exibicao
   * @return {bool} True, caso consiga
   */
  window = SDL_CreateWindow(
      "Projeto 2 - Sistemas Operacionais", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
      SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_ALWAYS_ON_TOP);

  if (window == nullptr)
  {
    error_on_initialize();
    return 0;
  }
  return 1;
}
bool sdl_render_start()
{
  /**
   * @brief Configura o render
   * @return {bool} True, para
   * sucesso
   */
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr)
  {
    error_on_initialize();
    return 0;
  }
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  return 1;
}
void sdl_close()
{
  /**
   * @brief Deleta o ambiente SDL
   * e limpa os ponteiros
   */
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  window = nullptr;
  renderer = nullptr;
  SDL_Quit();
}

void sdl_render_clear()
{
  /**
   * @brief Limpa o quadro de exibicao
   */
  SDL_SetRenderDrawColor(renderer, 30, 30, 30, 1.0);
  SDL_RenderClear(renderer);
}
void sdl_render_update()
{
  /**
   * @brief Atualiza o frame de exibicao
   */
  SDL_RenderPresent(renderer);
}
