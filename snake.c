#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 512
#define HEIGHT 512
#define SQUARE_SIZE 16
#define FRAME_DELAY 100
#define SNAKE_MAX_LENGTH (WIDTH * HEIGHT) / (SQUARE_SIZE * SQUARE_SIZE)

struct snake {
    SDL_Rect segments[SNAKE_MAX_LENGTH];
    int length;
    int dx, dy;
};

void init_snake(struct snake *snake) {
    snake->length = 3;
    snake->dx = SQUARE_SIZE;
    snake->dy = 0;

    for (int i = 0; i < snake->length; i++) {
        snake->segments[i].x = WIDTH / 2 - i * SQUARE_SIZE;
        snake->segments[i].y = HEIGHT / 2;
        snake->segments[i].w = SQUARE_SIZE;
        snake->segments[i].h = SQUARE_SIZE;
    }
}

void update_snake(struct snake *snake) {
    for (int i = snake->length - 1; i > 0; i--) {
        snake->segments[i] = snake->segments[i - 1];
    }
    snake->segments[0].x += snake->dx;
    snake->segments[0].y += snake->dy;
}

void grow_snake(struct snake *snake) {
    if (snake->length < SNAKE_MAX_LENGTH) {
        snake->segments[snake->length] = snake->segments[snake->length - 1];
        snake->length++;
    }
}

void draw_snake(SDL_Surface *surface, struct snake *snake) {
    Uint32 green = SDL_MapRGB(surface->format, 0, 255, 0);
    for (int i = 0; i < snake->length; i++) {
        SDL_FillRect(surface, &snake->segments[i], green);
    }
}

void check_for_collision(struct snake *snake, int *game_running) {
    if (snake->segments[0].x < 0 || snake->segments[0].x >= WIDTH || 
        snake->segments[0].y < 0 || snake->segments[0].y >= HEIGHT) {
        init_snake(snake);
        *game_running = 0;
    }
    for (int i = 1; i < snake->length; i++) {
        if (snake->segments[0].x == snake->segments[i].x && 
            snake->segments[0].y == snake->segments[i].y) {
            init_snake(snake);
            *game_running = 0;
        }
    }
}

void generate_food(SDL_Rect *food, struct snake *snake) {
    int valid;
    do {
        valid = 1;
        food->x = (rand() % (WIDTH / SQUARE_SIZE)) * SQUARE_SIZE;
        food->y = (rand() % (HEIGHT / SQUARE_SIZE)) * SQUARE_SIZE;
        food->w = SQUARE_SIZE;
        food->h = SQUARE_SIZE;
        for (int i = 0; i < snake->length; i++) {
            if (food->x == snake->segments[i].x && food->y == snake->segments[i].y) {
                valid = 0;
                break;
            }
        }
    } while (!valid);
}

void draw_food(SDL_Surface *surface, SDL_Rect *food) {
    Uint32 red = SDL_MapRGB(surface->format, 255, 0, 0);
    SDL_FillRect(surface, food, red);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    srand(time(NULL));

    struct snake snake;
    init_snake(&snake);

    SDL_Rect food;
    generate_food(&food, &snake);
    
    SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};
    int up_pressed = 0, down_pressed = 0, left_pressed = 0, right_pressed = 1;
    int quit = 0, game_running = 0;
    Uint32 last_update = SDL_GetTicks();

    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        game_running = !game_running;
                        if (game_running) {
                            init_snake(&snake);
                            generate_food(&food, &snake);
                            right_pressed = 1;
                            up_pressed = down_pressed = left_pressed = 0;
                        }
                        break;
                    case SDLK_UP:
                        if (snake.dy == 0) { up_pressed = 1; snake.dx = 0; snake.dy = -SQUARE_SIZE; }
                        break;
                    case SDLK_DOWN:
                        if (snake.dy == 0) { down_pressed = 1; snake.dx = 0; snake.dy = SQUARE_SIZE; }
                        break;
                    case SDLK_LEFT:
                        if (snake.dx == 0) { left_pressed = 1; snake.dx = -SQUARE_SIZE; snake.dy = 0; }
                        break;
                    case SDLK_RIGHT:
                        if (snake.dx == 0) { right_pressed = 1; snake.dx = SQUARE_SIZE; snake.dy = 0; }
                        break;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        if (game_running && now - last_update >= FRAME_DELAY) {
            last_update = now;
            check_for_collision(&snake, &game_running);
            update_snake(&snake);

            if (snake.segments[0].x == food.x && snake.segments[0].y == food.y) {
                grow_snake(&snake);
                generate_food(&food, &snake);
            }
        }

        SDL_FillRect(surface, &erase_rect, SDL_MapRGB(surface->format, 0, 0, 0));
        draw_food(surface, &food);
        draw_snake(surface, &snake);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

