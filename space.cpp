#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define BAR_SIZE        6
#define MAP_WIDTH     116
#define MAP_HEIGHT     26
#define ENEMY_LENGHT   10
#define ENEMY_SPEED 0.025
#define SHOT_SPEED   0.05

int Lives = 50;

struct Enemy {
	float x;
	float y;
	float speed;
};

struct Shot {
	int alive;
	int x;
	float y;
	struct Shot *next;

	int Hit(int _x, float _y) {
		return (x == _x && (int)y == (int)_y) ? 1 : 0;
	}
};

struct Shot *head = NULL;
struct Shot *current = NULL;

int kbhit() {
	int ch = getch();
	if (ch != ERR) {
		ungetch(ch);
		return 1;
	}
	return 0;
}

void DrawMap() {
	for(int i = 0; i < MAP_WIDTH; i++) {
		move(BAR_SIZE, i + 2);
		printw("=");
		move(MAP_HEIGHT, i + 2);
		printw("=");
	}
	for(int i = BAR_SIZE + 1; i < MAP_HEIGHT; i++) {
		move(i, 2);
		printw("|");
		move(i, MAP_WIDTH + 1);
		printw("|");
	}
}

void RespawnEnemy(Enemy* enemy) {
	enemy->x = (float)(3 + rand() % (MAP_WIDTH - 3));
	enemy->y = BAR_SIZE;
	enemy->speed = (float)(rand());
}

void DrawEnemy(Enemy* enemies) {
	for(int i = 0; i < ENEMY_LENGHT; i++) {
		move(enemies[i].y, enemies[i].x);
		printw(" ");
		if(enemies[i].y <= MAP_HEIGHT) {
			enemies[i].y += ENEMY_SPEED;
			//enemies[i].y += enemies[i].speed;
		} else {
			RespawnEnemy(&enemies[i]);
		}
		move(enemies[i].y, enemies[i].x);
		printw(".");
	}
}

void DrawChar(int x, int y, int c) {
	move(y, x);
	printw("%c", c);
	refresh();
	usleep(25000);
}

void KillEnemy(Enemy* enemy, int cant) {
			for(int j = 0; j < cant; j++) {
				DrawChar(enemy->x, enemy->y, '/');
				DrawChar(enemy->x, enemy->y, '\\');
				DrawChar(enemy->x, enemy->y, '-');
				DrawChar(enemy->x, enemy->y, ' ');
			}
			RespawnEnemy(enemy);
}

void CheckCollitiions(Enemy* enemies, int x, int y) {
	for(int i = 0; i < ENEMY_LENGHT; i++) {
		if(enemies[i].x >= x && enemies[i].x <= x+5 && enemies[i].y >= y && enemies[i].y <= y+3) {
			Lives--;
			KillEnemy(&enemies[i], 3);
		}
	}
}

void DrawBar(int posX, int posY, int c) {
	move(1, 0);
	mvprintw(0,7,"MtSpace Invaders\n");
	printw("  position: [%d, %d]\n", posX, posY);
	printw("  KeyPress: %d\n", c);
	printw("  Lives: %d\n", Lives);
}

void Move(int x, int y, int xh, int yh) {
	move(yh, xh);   printw("     ");
	move(yh+1, xh); printw("     ");
	move(yh+2, xh); printw("     ");

	move(y, x);     printw("  *  ");
	move(y+1, x);   printw(" *** ");
	move(y+2, x);   printw("*****");
}

void DrawShots(Enemy* enemies) {
	struct Shot *p = head;
	struct Shot *prev = head;
	while(p != NULL) {
		if(p->next != NULL && p->next->alive == 0) {
			free(p->next);
			p->next = p->next->next;
		}
		mvprintw(p->y, p->x, " ");

		for(int i = 0; i < ENEMY_LENGHT; i++) {
			if(p->Hit(enemies[i].x, enemies[i].y) == 1) {
				p->alive = 0;
				KillEnemy(&enemies[i],2);
			}
		}

		if(p->y > BAR_SIZE) {
			p->y -= SHOT_SPEED;;
			mvprintw(p->y, p->x, "|");
		} else {
			p->alive = 0;
		}
		p = p->next;
	}
}

void MakeShoot(int x, int y) {
	struct Shot *shot = (struct Shot*) malloc(sizeof(Shot));
	shot->alive = 1;
	shot->x = x + 2;
	shot->y = (float)y - 1;
	shot->next = head;
	head = shot;
}

int main() {
	clear();
	initscr();
	curs_set(0);
	nodelay(stdscr, TRUE);
	noecho();
	clear();

	int x = (int) (MAP_WIDTH / 2);
	int y = BAR_SIZE + (int)(MAP_HEIGHT / 2);
	int xh = x;
	int yh = y;
	int c = 65;
	Enemy enemies[ENEMY_LENGHT];
	for(int i = 0; i < ENEMY_LENGHT; i++) {
		enemies[i] = { (float)(3 + rand() % (MAP_WIDTH - 3)), BAR_SIZE, ((double)rand()) / ((double)RAND_MAX) * 0.007 + 0.002);
	}
	Move(x,y,xh,yh);
	while (1) {
		DrawMap();
		DrawEnemy(enemies);
		DrawShots(enemies);
		CheckCollitiions(enemies, x, y);
		if (kbhit()) {
			c = getch();
			switch(c) {
				case 119: case 65:
					y -= (y > BAR_SIZE + 1) ? 1 : 0;
					break;
				case 115: case 66:
					y += (y < MAP_HEIGHT - 3) ? 1 : 0;
					break;
				case 100: case 67:
					x += (x < MAP_WIDTH - 4) ? 1 : 0;
					break;
				case 97: case 68:
					x -= (x > 3) ? 1 : 0;
					break;
				case 32:
					MakeShoot(x, y);
					break;
				//default:
				//	printf("%d", c);
			}
			DrawBar(x, y, c);
			Move(x, y, xh, yh);
			xh = x;
			yh = y;
		}
		refresh();
		fflush(stdout);
		usleep(1000);
	}
	return 0;
}
